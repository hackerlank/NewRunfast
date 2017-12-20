#include "GameRoomBase.h"

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "playerinterface.h"
#include "Table.h"
#include "helper.h"
#include "ConfigMgr.h"
#include "RoomTimer.h"
#include "RoomListener.h"
#include "referee.h"
#include "PokerCmd.h"
#include "game_obj.h"

const static std::size_t ROBOT_GROUPD = 0;

const static std::size_t PLAYER_GROUPD = 1;

extern bool g_server_closed;

extern boost::int32_t g_server_id;

GameRoomBase::GameRoomBase(const boost::int32_t roomid, const roomcfg_type & cfg)
                           :RoomBase(roomid, cfg), roomcfg_(cfg),
                           room_timer_(new RoomTimer(obj_->service())), referee_(nullptr), dealer_(Table::INVALID_SEAT), cancel_dealer_(false),
						   robot_gold_incr_(0), activeplayer_(Table::INVALID_SEAT),
						   game_start_time_(0), betting_round_(IDLE_ROUND), baselinedelay_(0), round_taxation_(0),
						   card_generator_(nullptr), delay_push_trigger_(boost::bind(&GameRoomBase::OnDelayPush, this, _1) ),  
						   taxation_count_(0)
{
     DCHECK(timer != nullptr || timer->Init() == 0);
}

GameRoomBase::~GameRoomBase(void)
{
	
}

void GameRoomBase::BroadCastToVisitor(assistx2::Stream & packet)
{
	for (auto it = players_.begin(); it != players_.end(); ++it)
	{
		const std::int32_t seat_no = it->second->GetSeat();
		if (seat_no == Table::INVALID_SEAT ||  (GetSeat(seat_no)->ingame() == false && (GetSeat(seat_no)->status_ & Seat::PLAYER_STATUS_NET_CLOSE) == 0) )
		{
            SendTo(it->second, packet);
        }
	}
}

void GameRoomBase::BroadCastToInGame(assistx2::Stream & packet)
{
    for (Table::Iterator seat = table_->begin(); seat != table_->end(); ++seat)
    {
        if (seat->ingame() == true && (seat->status_ & Seat::PLAYER_STATUS_NET_CLOSE) == 0)
        {
            SendTo(seat->user_, packet);
        }
    }
}

boost::int32_t GameRoomBase::BuyChips( PlayerInterface * user, const chips_type chips )
{
	if (roomcfg_.type[0] == 'S' || roomcfg_.type[0] == 'J')
	{
		//假买入, 只有结算的时候，再去扣钱
		if (user->GetGameBaseInfo().gold() >= roomcfg_.minchips)
		{
			return 0;
		}
		else
		{
			return Texas::error_code::ERROR_GOLD_NOT_ENOUGH;
		}
	}
	else
	{
		DCHECK_EQ(roomcfg_.type[0], 'G');
		if (user->GoldPay(chips) == false)
		{
			//金币不足
			return Texas::error_code::ERROR_GOLD_NOT_ENOUGH;
		}
	}

	return 0;
}

void GameRoomBase::TarggerGameOverEvent()
{
	for (Table::Iterator seat = table_->begin(); seat != table_->end(); ++seat)
	{
        DCHECK(seat->bet_ == 0 || roomcfg_.type[0] == 'S' || roomcfg_.type[0] == 'J');

		if (seat->ingame() == true)
		{
			DCHECK_NOTNULL(seat->user_);
			seat->user_->OnGameOver(seat->ranking_ == 1);

			Signal(RoomEventListener::GAME_OVER, seat->user_);
		}

        DCHECK(seat->gift_chips_ >= 0);
		if (seat->gift_chips_ != 0)
		{
            //DCHECK(roomcfg_.type[0] == 'G');
            DCHECK(seat->user_ != nullptr);
			seat->bankroll_ +=seat->gift_chips_;
			seat->buy_chips_amount_ += seat->gift_chips_;
			seat->gift_chips_ = 0;
		}
	}

	//Tracer::getInstance()->OnGameOver(this);
}

std::string GameRoomBase::GetRoundID() const
{
	return round_id_;
}

void GameRoomBase::NewRound()
{
    boost::uuids::random_generator  generator;

    std::stringstream ss;
    ss << generator();

    round_id_ = ss.str();
}

bool GameRoomBase::CompareCards(Seat * dealer, Seat * seat)
{
	DCHECK(roomcfg_.type[0] == 'J' || roomcfg_.type[0] == 'B' || roomcfg_.type[0] == 'S' || roomcfg_.type[0] == 'Z');
	//不同的牛，牛大赢，相同的牛，比较最后一个值，成牌是按从小到大排列
	if (seat->show_ranking_ != dealer->show_ranking_)
	{
		return dealer->show_ranking_ > seat->show_ranking_;
	}
	else
	{
		DCHECK_EQ(dealer->show_ranking_, seat->show_ranking_);
		DCHECK_EQ(seat->holecards_.front()->Compare(*seat->holecards_.back() ), -1);
		DCHECK_EQ(dealer->holecards_.front()->Compare(*dealer->holecards_.back() ), -1);

		return seat->holecards_.back()->Compare(*dealer->holecards_.back() ) != 1;
	}
}

void GameRoomBase::NewTimer(long expires_from_now, RoomTimerContext::TimerType type,  Seat * seat)
{
	const boost::int32_t err = room_timer_->NewTimer(expires_from_now, boost::bind(&GameRoomBase::OnTimer, this, _1), GetID(), type, seat);
	if (err != 0)
	{
		EventTimerContext id(boost::bind(&GameRoomBase::OnTimer, this, _1), GetID(), type, seat);
		room_timer_->CancelTimer(id);

		room_timer_->NewTimer(expires_from_now, boost::bind(&GameRoomBase::OnTimer, this, _1), GetID(), type, seat);
	}
}

bool GameRoomBase::CancelTimer(RoomTimerContext::TimerType type,  Seat * seat)
{
	EventTimerContext id(boost::bind(&GameRoomBase::OnTimer, this, _1), GetID(), type, seat);
	return room_timer_->CancelTimer(id);
}

void GameRoomBase::WriteChipsLog()
{
	if (roomcfg_.type[0] == 'J' || roomcfg_.type[0] == 'S')
	{
		chips_type win_group[2] = { 0 };
		chips_type taxation_group[2] = { 0 };
		chips_type gold_balance = 0;
		for (Table::Iterator seat = table_->begin(); seat != table_->end(); ++seat)
		{
			if (seat->user_ != nullptr)
			{
				DCHECK_EQ(seat->bet_chips_amount_, 0);
				gold_balance += seat->win_chips_;

				win_group[ seat->is_robot_ == true ? 0 : 1] += seat->win_chips_;

				taxation_group[seat->is_robot_ == true ? 0 : 1] +=
					(seat->win_chips_ > 0 ? 0 : static_cast<chips_type>(boost::int64_t( -seat->win_chips_) * boost::int64_t(roomcfg_.taxation) / 1000 ) );
			}
		}

		if (roomcfg_.taxes_mode == FIXED_TAXES_MODE)
		{
			DCHECK_EQ(gold_balance, 0);
			DCHECK_EQ(abs(win_group[0]),  abs(win_group[1]) );

			robot_gold_incr_ += win_group[0];

			//固定模式下，机器人没收税
		}
		else
		{
			DCHECK_EQ(gold_balance + round_taxation_, 0);

			robot_gold_incr_ += (win_group[0] + taxation_group[0]);

			//扣除机器人交的税
			taxation_count_ -= taxation_group[0];
		}

// 		DLOG(INFO)<<" room:="<<GetID()<<", gold_balance:="<<gold_balance
// 			<<", round_taxation_:="<<round_taxation_<<", win_group[0]:="<<win_group[0]
// 		<<", taxation_group[0]:="<<taxation_group[0] <<", win_group[1]:="<<win_group[1]
// 		<<", taxation_group[1]:="<<taxation_group[1];
	}
	else
	{
		chips_type win_group[2] = {0};

#ifndef NDEBUG
		chips_type chips_amount = 0;
#endif

		for (Table::Iterator seat = table_->begin(); seat != table_->end(); ++seat)
		{
			DCHECK(seat->win_chips_ == 0 || seat->user_ != nullptr);

#ifndef NDEBUG
			chips_amount += seat->win_chips_ - seat->bet_chips_amount_;

// 			DLOG(INFO)<<"room:="<<GetID()<<", win_chips_:="<<seat->win_chips_
// 				<<", bet_chips_amount_:="<<seat->bet_chips_amount_<<", seat:="<<seat->no_;
#endif

			win_group[seat->is_robot_ ? ROBOT_GROUPD : PLAYER_GROUPD] += (seat->win_chips_ -  seat->bet_chips_amount_ );
		}

#ifndef NDEBUG
		if (roomcfg_.taxes_mode == FIXED_TAXES_MODE)
		{
			DCHECK_EQ(chips_amount, 0);
			DCHECK_EQ( abs(win_group[0]),  abs(win_group[1]) )<<", room:="<<GetID()<<", type:="<<roomcfg_.type;
		}
		else
		{
			DCHECK_EQ(chips_amount + round_taxation_, 0 )
				<<"chips_amount:="<<chips_amount<<", round_taxation_:="<<round_taxation_<<", room:="<<GetID();
		}
#endif

		DLOG(INFO)<<"room:="<<GetID()<<", type:="<<roomcfg_.type<<", robot_win:="<<win_group[0]<<", player_win:="<<win_group[1]
		<<", bet_group_[0]:="<<bet_group_[0]<<", bet_group_[1]:="<<bet_group_[1];

		robot_gold_incr_ += (win_group[0]);
	}
}

void GameRoomBase::WriteBetLog( Seat * seat )
{
	seat->bet_chips_amount_ += seat->bet_;

// 	DLOG(INFO)<<"GameRoomBase::WriteBetLog, room:="<<GetID()
// 		<<", mid:="<<(seat->user_ == nullptr ? 0 :  seat->user_->GetUID() )<< ", bet:="<<seat->bet_;

	bet_group_[seat->is_robot_ ? ROBOT_GROUPD : PLAYER_GROUPD] += seat->bet_;
}

chips_type GameRoomBase::CollectRobotGoldIncr()
{
	chips_type tmp = robot_gold_incr_;
	robot_gold_incr_ = 0;
	return tmp;
}

boost::int32_t GameRoomBase::GetMultiplying( const char room_type, Ranking ranking )
{
	if (room_type == 'S')
	{
		static const boost::int32_t multiplying[]  =
		{
			1,  1,  1,  1,  1, 1,  1,		//0-6 一倍
			2, 2, 2,								//7 - 9
			3,										//牛牛
			0,										// 	
			4,										//四炸
			5,										//满牛
			10									//五小牛
		};

		return multiplying[ranking];
	}
	else if (room_type == 'J')
	{
		DCHECK_NE(NIN_FULL, ranking);
		DCHECK_NE(NIN_SMALL, ranking);

		static const boost::int32_t multiplying[]  =
		{
			1,  1,  1,  1,  1, 1,  1, 1,		//0-7 一倍
			2, 									// 8
			3, 									// 9
			4,	4, 								//牛牛 - 白皮牛
			5										//四炸
		};

		return multiplying[ranking];
	}
	else if (room_type == 'B')
	{
		DCHECK_LE(NIN_None, ranking);
		DCHECK_GE(NIN_BOMB, ranking);

		static const boost::int32_t multiplying[]  =
		{
			1,				// 		没牛,
			1,				// 		牛一,
			1,				// 		牛二,
			1,				// 		牛三,					
			1,				// 		牛四,
			1,				// 		牛五,
			1,				// 		牛六,
			1,				// 		牛七,
			1,				// 		牛八,
			2,				// 		牛九
			3,			// 		牛牛
			4,			// 		白皮牛
			5,			// 		炸弹
		};

		return multiplying[ranking];
	}
	else
	{
		DCHECK(room_type == 'B' || room_type == 'J' || room_type == 'G' )<<", room_type:="<<room_type;
		return 0;
	}
}

void GameRoomBase::DelayHandle(long expires_from_now, boost::shared_ptr<assistx2::Stream > & stream,
							   DelayTimerContext::BroadCastType type, PlayerInterface * target )
{
	const uid_type mid = (target == nullptr ? 0 : target->GetUID() );
	const boost::int32_t err = room_timer_->NewTimer(expires_from_now, delay_push_trigger_, GetID(), stream, type, mid );

	DCHECK_EQ(err, 0);
}

void GameRoomBase::OnDelayPush( boost::shared_ptr<assistx2::timer2::TimerContext > context )
{
	DelayTimerContext * impl = dynamic_cast<DelayTimerContext *>(context.get() );

	switch (impl->broadcast_type_)
	{
	case DelayTimerContext::BROADCAST_TO_ALL:
		BroadCast(*impl->msg_);
		break;
	case DelayTimerContext::BROADCAST_TO_TARGET:
		{
			PlayerInterface * player = GetPlayer(impl->mid_);
			if (player != nullptr)
			{
				SendTo(player, *impl->msg_);
			}
		}
		break;
	case  DelayTimerContext::BROADCAST_TO_VISITOR:
		BroadCastToVisitor(*impl->msg_);
		break;
	case DelayTimerContext::BROADCAST_TO_INGAME:
		BroadCastToInGame(*impl->msg_);
	default:
		break;
	}
}

PlayerInterface * GameRoomBase::GetDealer()
{
    if (dealer_ == Table::INVALID_SEAT)
    {
        return nullptr;
    }
    else
    {
        return GetSeat(dealer_)->user_;
    }
}







