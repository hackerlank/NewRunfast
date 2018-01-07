#include "player.h"

#include <algorithm>

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

#include <json_spirit_writer_template.h>

#include <assistx2/string_wrapper.h>

#include "data_layer.h"
#include "config_mgr.h"
#include "game_room_base.h"
#include "table.h"
#include "poker_cmd.h"
#include "proxy_cmd.h"
#include "runfast_tracer.h"

extern int g_server_id;

Player::Player(const uid_type mid):PlayerBase(mid, PlayerInterface::REAL_PLAYER_TYPE)
{
	
}

Player::~Player(void)
{
	
}

int Player::Serialize(bool loadorsave)
{
	if (loadorsave == true)
	{
		std::string tmp;
		if (DataLayer::getInstance()->GetPlayerBaseInfo(uid_, tmp, roleinfo_) != 0)
		{
			DLOG(INFO) << "GetPlayerBaseInfo failed mid:=" << uid_;
			return -1;
		}

		if (DataLayer::getInstance()->GetCommonGameInfo(uid_, game_base_) != 0)
		{
			DLOG(INFO) << "GetCommonGameInfo failed mid:=" << uid_;
			return -1;
		}

//		if (DataLayer::getInstance()->GetPlayerGameInfo(uid_, gameinfo_) != 0)
//		{
//			DLOG(INFO) << "GetPlayerGameInfo failed mid:=" << uid_;
//			return -1;
//		}

		std::string data;
		DataLayer::getInstance()->GetDayData(uid_, data);

        DataLayer::getInstance()->GetLoginIP(uid_, login_ip_);
        login_ip_ = assistx2::remove_space(login_ip_);

		PlayerDayData::FromString(data, today_data_);
	}
	else
	{
		
		//DataLayer::getInstance()->UpdateDayData(uid_, today_data_.ToString() );
		
		riches_rank_ = 0;
		win_rank_ = 0;
		cards_.clear();
	}

	return 0;
}

//gold加金币为负，减金币为正
bool Player::GoldPay( const chips_type gold)
{
	chips_type amount = 0;
	chips_type real_pay = 0;
	const int err = DataLayer::getInstance()->Pay(uid_, gold,  amount, real_pay, false);
	if ( err == 0)
	{
		LOG(INFO) << "GoldPay " << ", mid:=" << uid_ << ",gold_befor:" << game_base_.gold() << ",delta:" << gold << ",amount:" << amount;
		game_base_.set_gold(amount);
		RunFastTracer::getInstance()->OnGoldChange(this);
		return true;
	}
	else
	{
		LOG(ERROR)<<"Player::GoldPay FALIED, err:="<<err<<", mid:="<<uid_<<", gold:="<<gold;

		return false;
	}
}

bool Player::UpdateScore(const chips_type score)
{
    const int new_score = DataLayer::getInstance()->UpdateRunFastScore(uid_, score);
  
    LOG(INFO) << "UpdateScore " << ", mid:=" << uid_ << ",old_score:" << gameinfo_.jifen() << ",score:" << score << ",new_score:" << new_score;
    gameinfo_.set_jifen(new_score);
    return true;
}

bool Player::PropsPay(std::int32_t pcate, std::int32_t pframe, std::int32_t num,bool isPay)
{
    auto res = DataLayer::getInstance()->PayProps(uid_, pcate, pframe, num,isPay);
    LOG(INFO) << "Player::PropsPay " << ", mid:=" << uid_ << ",pcate:" 
        << pcate << ",pframe:" << pframe << ",num:" << num << ",isPay:" << isPay << ",res:" << res;
    if (res != 0)
    {
        return false;
    }
    return true;
}

bool Player::ForceGoldPay(const chips_type gold, chips_type& real_delta)
{
	chips_type amount = 0;
	const int err = DataLayer::getInstance()->Pay(uid_, gold, amount, real_delta,true);
	if (err == 0)
	{
		LOG(INFO) << "SettleAccounts Player::ForceGoldPay " << ", mid:=" << uid_ << ",gold_befor:" << game_base_.gold()
			<< ",delta:" << gold << ",amount:" << amount << ",real_delta:" << real_delta;
		game_base_.set_gold(amount);
		//real_delta = -real_delta;

		if (real_delta != gold)
		{
			LOG(ERROR) << "SettleAccounts Player::ForceGoldPay ERROR" << ", mid:=" << uid_ << ",gold_befor:" << game_base_.gold()
				<< ",delta:" << gold << ",amount:" << amount << ",real_delta:" << real_delta;
		}

		return true;
	}
	else
	{
		LOG(ERROR) << "SettleAccounts Player::ForceGoldPay FALIED, err:=" << err << ", mid:=" << uid_ << ", gold:=" << gold;
		return false;
	}
}

void Player::OnGameOver(bool winner)
{
    ++round_count_;

    if (winner == true)
    {
        ++win_count_;
    }

    DLOG(INFO) << "OnGameOver mid:=" << GetUID() << ", winner:=" << (winner ? "true" : "false")
        << ", round_count_:=" << round_count_ << ", win_count_:=" << win_count_;

	today_data_.IncrRoundCount();
	//CFlyChessAct::getInstance()->OnCountPlayerGame(this);
}	

class ComparePrice : public std::binary_function<LevelItem, chips_type, bool>
{
public:
	bool operator()(const LevelItem & left, const chips_type & right) const
	{	// apply operator< to operands
		return (left.price <= right);
	}
};

void Player::SitUp()
{
	return PlayerBase::SitUp();
}

void Player::SitDown( int seat )
{
	return PlayerBase::SitDown(seat);
}

PlayerDayData & Player::GetTodayData()
{
    return today_data_;
}

void Player::SetTableProp(const PropsBase_type & prop)
{
    DCHECK(prop.pcate > 10 && prop.pcate <= 20);
    table_prop_ = prop;
}




