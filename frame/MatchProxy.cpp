#include "MatchProxy.h"
#include "playerinterface.h"
#include "PokerCmd.h"
#include "game_obj.h"

const static std::int16_t GAMESERVER_ROUTE_PACKET = 10010;
const static std::int16_t LEAVE_FROM_MATCH_SERVER = 9998;



MatchProxy::MatchProxy(): guard_(GameObj::GetInstance()), connect_(guard_->gatewayconnector())
{

}

MatchProxy::~MatchProxy()
{

}

std::int32_t MatchProxy::OnMessage(PlayerInterface * player, const assistx2::Stream& packet)
{
    if (nullptr == player->GetMatchProxy())
    {
        return 0;
    }

    assistx2::Stream clone(packet);
    const auto cmd = clone.GetCmd();
    switch (cmd)
    {
    case RunFast::CLIENT_ENTER_MATCH_HALL:
        OnEnterMatchHall(player, packet);
        break;
    case LEAVE_FROM_MATCH_SERVER:
    {
        player->SetMatchProxy(nullptr);
//        leave_func_(player);
        OnLeaveMatchHall(player);
    }
        break;
    default:
        RouteMatchServer(player, clone);
    	break;   
    case ENTER_TO_MATCH_SERVER:
        player->SetMatchProxy(this);
        break;
    }

    return 0;
}

void MatchProxy::SetLeaveCallBack(std::function<void(PlayerInterface *)> func)
{
    leave_func_ = func;
}

void MatchProxy::OnEnterMatchHall(PlayerInterface *player, const assistx2::Stream &packet)
{
    if (player->GetRoomObject() != nullptr)
    {
        assistx2::Stream stream(RunFast::SERVER_ENTER_MATCH_HALL);
        stream.Write(player->GetUID());
        stream.Write(RunFast::ErrorCode::ERROR_ENTER_MATCH_HAS_ROOM);
        stream.End();

        connect_->SendTo(stream.GetNativeStream());
        return;
    }

//    OnMessage(player, packet);
}

void MatchProxy::OnLeaveMatchHall(PlayerInterface *player)
{
    if (player->GetMatchProxy() == nullptr &&
        player->GetLoginStatus() == false)
    {
        playermgr_->RemovePlayer(player);
    }
}

void MatchProxy::RouteMatchServer(PlayerInterface * player, assistx2::Stream& packet)
{
    //packet.Insert(player->GetUID());

    assistx2::Stream stream(GAMESERVER_ROUTE_PACKET);
    stream.Write(1301);
    stream.WriteBinary(std::string(packet.GetNativeStream().GetData(), packet.GetNativeStream().GetSize()));
    stream.End();

    connect_->SendTo(stream.GetNativeStream());
}
