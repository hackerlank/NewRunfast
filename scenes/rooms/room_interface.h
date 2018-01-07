#ifndef _XPORT_SRC_IMESSAGEHANDLER_H_
#define _XPORT_SRC_IMESSAGEHANDLER_H_

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <assistx2/stream.h>

#include "referee.h"
#include "handle_obj.h"

class PlayerInterface;
class Seat;
class Table;
class RoomEventListener;
class EventContext;
typedef std::function<int (PlayerInterface *,std::int32_t) > KickCallbcak;

class RoomInterface : public HandleObj
{
private:
	void operator=(const RoomInterface &) = delete;

public:
	enum State
	{
        EMPTY,
		WAITING,				//游戏准备开始(等待玩家进入)
		PLAYING,
		Freezing
	} ;

	//Hand hog
	enum BettingRound 
	{
		IDLE_ROUND,			//等侍状态
		HAND_HOG = 1,		//抢庄
		BETTING,					//下注
		SELECT_RAND,			//选牌
		SHOW,						//亮牌
		OVER,						//结束
		COMP_CARDS,		//比牌演示
	};

public:
	explicit RoomInterface(const int roomid):roomid_(roomid), state_(EMPTY) {}

	virtual ~RoomInterface() {}

	//成功处理消息返回0， 否则返回-1
	virtual int OnMessage(PlayerInterface * /*user*/, assistx2::Stream * /*packet*/) = 0;

	int GetID()const
	{
		return roomid_;
	}

  State GetState() const
  {
    return state_;
  }

  void SetState(const State& state)
  {
    state_ = state;
  }

	virtual const roomcfg_type & getRoomCfg()const = 0;
  virtual const runfastroomcfg & getRunFastRoomCfg()const = 0;
	//进入房间
	virtual int Enter(PlayerInterface *) = 0;

	//离开房间
	virtual int Leave(PlayerInterface *, std::int32_t err) = 0;

	//强制踢出， 成功返回 0， 
	virtual int Kick(const uid_type mid, bool) = 0;

  virtual int Disband(PlayerInterface *) = 0;

	//坐在座位上的用户数
	virtual int GetSeatPlayerCount()const = 0;

    virtual void ReSetTable(Table* table) = 0;

	//旁观用户数
	virtual int GetVisitorCount()const = 0; 

	//将房间信息同步到缓存（玩家列表）
	virtual void update()const = 0;

	//获取税收总额，并清零
	virtual chips_type CollectTaxes() = 0;

	//返回牌局编号
	virtual std::string GetRoundID()const = 0;

	virtual void Init(KickCallbcak & /*call*/) = 0;

	//广播消息
	virtual void BroadCast(assistx2::Stream & /*packet*/, PlayerInterface * /*exclude = NULL*/) = 0;

	virtual PlayerInterface * GetPlayer(const uid_type mid) = 0;

  virtual void SetOwner(int32_t  owner) = 0;
  virtual int32_t GetOwner() = 0;

	virtual const std::map<uid_type, PlayerInterface * > & GetPlayers()const = 0;

	virtual bool HasRealPlayer() = 0;

	virtual Seat * GetSeat(const int /*seat*/) = 0;

	virtual Table * GetTable() = 0;

	virtual void PeekWinlist( std::vector<HandStrength> & ) = 0;

	virtual BettingRound GetBettingRound()const = 0;

	virtual void RegisterRoomEventObserver(RoomEventListener * /*observer*/) = 0;

	virtual chips_type CollectRobotGoldIncr() = 0;

	virtual time_t GetGameStartTime()const = 0;

	virtual Referee * GetReferee() = 0;

	virtual std::size_t GetHandHogCount()const = 0;

    virtual PlayerInterface * GetDealer() = 0;

    virtual chips_type GetBetAmount()const = 0;

	virtual std::time_t GetIdlingTime() = 0;

    virtual void Signal(boost::int32_t event, PlayerInterface * trigger, EventContext * context = nullptr) = 0;

	virtual void ForEachLivePlayer(std::function< bool(PlayerInterface *)  >) = 0;

	//参与游戏的人数
	virtual std::int32_t GetLivePlayerCount() = 0;

	virtual chips_type CollectCarryGold()
	{
		return 0;
	}
    //设置/获取基数
    virtual void SetCardinalScore(std::int32_t score) = 0;
    virtual std::int32_t GetCardinalScore() = 0;

    virtual bool IsRobotRoom() = 0;
private:
	const int roomid_;
  
protected:
    KickCallbcak kickcallback_;
    State state_;
};

#endif
