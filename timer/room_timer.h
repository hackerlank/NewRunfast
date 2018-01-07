#ifndef _XPOKER_SRC_ROOM_TIMER_H_
#define _XPOKER_SRC_ROOM_TIMER_H_

#include <assistx2/timer2.h>
#include <assistx2/stream.h>

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

#include "xpoker.h"

class Seat;

//trigger
typedef boost::function<void (boost::shared_ptr<assistx2::timer2::TimerContext > ) > Trigger_type;

class RoomTimerContext : public assistx2::timer2::TimerContext
{
public:
	enum TimerType
	{
		BUY_TIMER, 
		BET_TIMER,								//下注时间
		HAND_HOG_TIMER,				//抢庄时间
		SHOW_CARDS_TIMER,			//明牌时间
		FLUSH_TIMER,							//游戏结束，定时器
		START_TIMER,							//游戏开始
		SYNC_TIMER,							//
		DELAY_BROADCAST_TIMER, //延迟广播时间
		RAND_TIMER,							//投色子时间
		//SettleAccounts
		SETTLEACCOUNTS_TIMER,		//结算时间
		NEXT_TAKER,
    TIME_OUT_PLAY,  //出牌超时
    TIME_OUT_ZHANLI, //暂离超时
    TIME_END_PLAY,
    TIME_GAME_OVER,
    TIME_ON_PASS,
    DELAY_START,
    PLAY_NIL, //要不起
    TIME_DISBAND_ROOM
	};

public:
	RoomTimerContext(const Trigger_type & trigger, boost::int32_t room, TimerType type)
		:TimerContext(trigger), room_(room), type_(type)
	{

	}

	virtual ~RoomTimerContext()
	{

	}

	boost::int32_t room_;
	TimerType type_;
};

class EventTimerContext : public RoomTimerContext
{
public:
	EventTimerContext(const Trigger_type & trigger, boost::int32_t room, TimerType type, Seat * seat)
		:RoomTimerContext(trigger, room, type), seat_(seat)
	{

	}

	virtual ~EventTimerContext() {}

	virtual bool Equal(const TimerContext * right)const;

	Seat * seat_;
};

class DelayTimerContext : public RoomTimerContext
{
public:
	enum BroadCastType
	{
		BROADCAST_TO_ALL,
		BROADCAST_TO_VISITOR,
		BROADCAST_TO_TARGET,
		BROADCAST_TO_INGAME
	};

	DelayTimerContext(const Trigger_type & trigger, boost::int32_t room, boost::shared_ptr<assistx2::Stream > msg, BroadCastType type, uid_type mid = 0)
		:RoomTimerContext(trigger, room, DELAY_BROADCAST_TIMER), msg_(msg), broadcast_type_(type), mid_(mid)
	{

	}

	virtual ~DelayTimerContext() {}

	virtual bool Equal(const TimerContext * /*right*/)const
	{
		return false;
	}

	boost::shared_ptr<assistx2::Stream > msg_;

	BroadCastType broadcast_type_;
	
	uid_type mid_;
};

class RoomTimer
{
	RoomTimer(void);
	RoomTimer(const RoomTimer &);
	RoomTimer & operator=(const RoomTimer &);
public:
	explicit RoomTimer(boost::asio::io_service & ios);
	~RoomTimer(void);

	boost::int32_t Init();

	boost::int32_t NewTimer(long expires_from_now_millisecond, Trigger_type trigger, 
		boost::int32_t room, RoomTimerContext::TimerType type,  Seat * seat);

	boost::int32_t NewTimer(long expires_from_now_millisecond, Trigger_type trigger, 
		boost::int32_t room, boost::shared_ptr<assistx2::Stream > stream, DelayTimerContext::BroadCastType type, uid_type mid = 0);

	bool CancelTimer(const RoomTimerContext & id);

private:
	boost::asio::io_service & ios_;

	assistx2::timer2::TimerQueue * queue_;
};

#endif //_XPOKER_SRC_ROOM_TIMER_H_

