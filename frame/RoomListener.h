#ifndef _XPOKER_SRC_ROOMLISTENER_H_
#define _XPOKER_SRC_ROOMLISTENER_H_

#include "xPoker.h"

class RoomInterface;
class PlayerInterface;

class EventContext
{
public:
    virtual ~EventContext()
    {

    }

private:

};

class PlayRecord : public EventContext
{
public:
    PlayRecord()
    {

    }

    explicit PlayRecord(std::int32_t mid) :mid_(mid), play_start_(std::time(nullptr) )
    {

    }

    virtual ~PlayRecord()
    {

    }

    std::int32_t mid_ = 0;
    
    std::time_t play_start_ = 0;

    chips_type win_ = 0;
};

class RoomEventListener 
{
public:
	enum event_type
	{
    PLAYER_ENTER_EVNET,
    PLAYER_LEAVE_EVNET,
    PLAYER_READY_EVNET,
		PLAYER_SITDOWN_EVENT, 
		PLAYER_SITUP_EVENT,
		ROOM_INIT_EVENT,
		REQUEST_CREAT_TWOPLAYER_ROOM,					//创建两人房，不要让玩家等机器人进来
		PLAYER_WIN,																//胜利了
		GAME_OVER,
		ROUND_BET,//一局的总下注
	};

public:
	virtual ~RoomEventListener() {}

	 virtual void OnEvent(const int /*event*/, PlayerInterface *, RoomInterface * , EventContext * ) = 0;
};

#endif