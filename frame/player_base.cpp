#include "player_base.h"

#include <ctime>

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

#include "data_layer.h"
#include "table.h"
#include "poker_cmd.h"
#include "room_interface.h"
#include "runfast_tracer.h"
#include "handle_obj.h"

bool LoadFriends( uid_type mid, std::set<uid_type > * friends )
{
	DataLayer::getInstance()->GetFriends(mid, *friends);
	DataLayer::getInstance()->GetPokerFriends(mid, *friends);

	return true;
}


void PlayerBase::SitDown( boost::int32_t seat )
{
    DCHECK(seat != Table::INVALID_SEAT);
    DCHECK(seat_ == Table::INVALID_SEAT);
	seat_ = seat;
}

PlayerBase::PlayerBase( uid_type mid, std::int32_t type ) :
    uid_(mid), type_(type), sitdowntime_(0), login_time_(0), seat_(Table::INVALID_SEAT),
    closed_(false), roomobject_(nullptr), win_rank_(0), riches_rank_(0),
    friends_(boost::bind(LoadFriends, mid, _1) )
{

}

int32_t PlayerBase::OnMessage(assistx2::Stream& packet)
{
    const std::int32_t cmd = packet.GetCmd();
    DLOG(INFO) << "RunFastGameMgr::OnMessage()->cmd:" << cmd;

    switch(cmd)
    {
    case Texas::CLIENT_REQUEST_CHANGE_NAME:
    {
        const std::string name = packet.Read<std::string>();
        if (name.empty() == false)
        {
            getRoleInfo().set_name(name);
        }
    }
    default:
        break;
    }

    if (GetRoomObject())
    {
        GetRoomObject()->OnMessage(this, &packet);
    }
    DLOG(INFO) << "RunFastGameMgr::OnMessage() GetRoomObject Failed room=nullptr mid:=" << GetUID();
    return 0;
}

void PlayerBase::SitUp()
{
    DCHECK(seat_ != Table::INVALID_SEAT);
	seat_ = Table::INVALID_SEAT;
}

std::int32_t PlayerBase::GetPoints() const
{
    return points_;
}

std::int32_t PlayerBase::GetLoginSource() /*const*/
{
	DLOG(INFO) << "PlayerBase::GetLoginSource, mid:=" << GetUID() << ", " << login_source_;
	if (login_source_ == 0)
	{
		return getRoleInfo().gp();
	}
	else
	{
		return login_source_;
	}
}

void PlayerBase::SetMatchProxy(std::shared_ptr<MatchProxy> obj)
{
    match_proxy_ = obj;
}

std::shared_ptr<MatchProxy> PlayerBase::GetMatchProxy()
{
    return match_proxy_;
}

int32_t PlayerBase::PlayCost(PlayerInterface *player, int32_t cost, bool isPayByOther, int32_t &proxy_mid)
{
    if (isPayByOther == false)
    {
        auto pay = player->GoldPay(cost);
        if (pay == true)
        {
            auto new_gold = player->GetGameBaseInfo().gold();

            RunFastTracer::getInstance()->WriteGoldLog(player->GetUID(), -cost, new_gold, 4);
        }
        else
        {
            return -2;//金币不足
        }
    }
    else
    {
        std::string data;
        auto res = DataLayer::getInstance()->GetRopen(player->GetUID(), data);
        if (res == false) return -5;//没有指定扣费代理

        proxy_mid = /*assistx2::atoi_s(data);*/atoi(data.c_str());
        if (proxy_mid == 0) return -5;

        chips_type amount = 0;
        chips_type real_pay = 0;
        const int err = DataLayer::getInstance()->Pay(proxy_mid, cost, amount, real_pay, false);
        if (err == 0)
        {
            assistx2::Stream stream(RunFast::SERVER__UPDATE_GOLD);
            stream.Write(proxy_mid);
            stream.Write(amount);
            stream.End();
            gatewayconnector_.SendTo(stream.GetNativeStream());

            RunFastTracer::getInstance()->WriteGoldLog(proxy_mid, -cost, amount, 4, player->GetUID());
            LOG(INFO) << "GoldPay " << ", proxy_mid:=" << proxy_mid << ",delta:" << cost << ",amount:" << amount;
        }
        else
        {
            LOG(ERROR) << "Player::GoldPay FALIED, err:=" << err << ", proxy_mid:=" << proxy_mid << ", gold:=" << cost;
            return -6;//代理金币不足
        }
    }

    return 0;
}
