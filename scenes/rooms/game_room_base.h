#ifndef _XPOKER_SRC_GAME_ROOMBASE_H_
#define _XPOKER_SRC_GAME_ROOMBASE_H_

#include <set>

#include <assistx2/tcp_handler.h>

#include "room_base.h"
#include "room_timer.h"
#include "table.h"

typedef boost::function<void ( ) > DelayCallbackHandler_type;

class RoomTimer;
class RoomTimerContext;
class Referee;
class CardGenerator;

class GameRoomBase : public RoomBase
{
	friend class Tracer;

public:
    GameRoomBase(const boost::int32_t roomid, const roomcfg_type & cfg);

	virtual ~GameRoomBase(void);

	virtual std::string GetRoundID()const override;

	virtual void OnTimer( boost::shared_ptr<assistx2::timer2::TimerContext > context ) = 0;

	void OnDelayPush( boost::shared_ptr<assistx2::timer2::TimerContext > context );

	virtual const roomcfg_type & getRoomCfg()const
	{
		return roomcfg_;
	}

	virtual chips_type CollectTaxes()
	{
		chips_type tmp = taxation_count_;
		taxation_count_ = 0;
		return tmp;
	}

	virtual std::size_t GetHandHogCount()const
	{
		return 0;
	}

	boost::int32_t GetMultiplying(const char room_type, Ranking ranking);

protected:
	//打包桌面数据
	void TableSnapShot(assistx2::Stream & packet);

	//游戏结束的时候，通知所有事情监听者
	void	TarggerGameOverEvent();

	void BroadCastToVisitor(assistx2::Stream & packet);

	void BroadCastToInGame(assistx2::Stream & packet);

	boost::int32_t BuyChips(PlayerInterface * user, const chips_type chips);

	virtual BettingRound GetBettingRound()const
	{
		return betting_round_;
	}

	virtual chips_type CollectRobotGoldIncr();

	virtual time_t GetGameStartTime()const
	{
		return game_start_time_;
	}

    PlayerInterface * GetDealer() override;

    virtual chips_type GetBetAmount()const override
    {
        return 0;
    }

public:
    //大于或等于返回TRUE
	bool CompareCards(Seat * dealer, Seat * seat);

protected:
	void DelayHandle(long expires_from_now, boost::shared_ptr<assistx2::Stream > & stream, 
		DelayTimerContext::BroadCastType type,  PlayerInterface * target);

	void WriteChipsLog();

	void WriteBetLog( Seat * seat );

protected:
	void NewTimer(long expires_from_now, RoomTimerContext::TimerType type,  Seat * seat = NULL);

	bool CancelTimer(RoomTimerContext::TimerType type,  Seat * seat = NULL);

    void NewRound();

protected:
	//房间配置数据
	roomcfg_type roomcfg_;

	RoomTimer  * room_timer_;

  Referee * referee_;

	//庄家座位号
	boost::int32_t	dealer_;

	//庄家是否弃庄
	bool cancel_dealer_;

	chips_type robot_gold_incr_;

	//当前玩家
	boost::int32_t	activeplayer_ = Table::INVALID_SEAT;

	//本轮在玩玩家数.不包括弃牌的玩家,包括ALLIN玩家
	boost::int32_t	liveplayers_;

	//牌局标识
	std::string round_id_;

	//游戏开始时间
	time_t game_start_time_;

	BettingRound	betting_round_;

	long baselinedelay_;

	//当前合回的抽水
	chips_type round_taxation_;

	//牌
	CardGenerator  *	card_generator_;

	Trigger_type delay_push_trigger_;

public:
	//抽水总额
	chips_type	taxation_count_;

	// bet_group_[0] 为机器压注数， bet_group_[1] 为玩家下注数
	chips_type bet_group_[2];
};

#endif //_XPOKER_SRC_GAME_ROOMBASE_H_

