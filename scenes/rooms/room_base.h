#ifndef _XPOKER_SRC_ROOMBASE_H_
#define _XPOKER_SRC_ROOMBASE_H_

#include <assistx2/tcphandler_wrapper.h>

#include "xpoker.h"
#include "room_interface.h"

class PlayerInterface;
class Table;
class Seat;
class RoomEventListener;
class EventContext;
class RoomBase : public RoomInterface
{
public:
    RoomBase(const boost::int32_t roomid, const roombasecfg_type & cfg);
	virtual ~RoomBase(void);

	//广播消息
	virtual void BroadCast(assistx2::Stream & packet, PlayerInterface * exclude = NULL);

	virtual boost::int32_t Enter(PlayerInterface * user);

	virtual boost::int32_t Leave(PlayerInterface * user, std::int32_t err);

	//玩家坐下
	boost::int32_t SitDown(PlayerInterface * user, boost::int32_t seat);

	//玩家离开坐位
	boost::int32_t SitUp(PlayerInterface * user);

	void SendTo(PlayerInterface * player, const assistx2::Stream & stream);

    virtual void ReSetTable(Table* table);

public:
	//坐在座位上的用户数
	virtual boost::int32_t GetSeatPlayerCount()const;

	//旁观用户数
	virtual boost::int32_t GetVisitorCount()const;

	virtual void update()const;

  virtual void  SetOwner(int32_t owner) { owner_ = owner; };
  virtual int32_t GetOwner() { return owner_;  };

	virtual PlayerInterface * GetPlayer(const uid_type mid)
	{
		std::map<uid_type, PlayerInterface * >::iterator it = players_.find(mid);
		if (it != players_.end())
		{
			return it->second;
		}
		else
		{
			return nullptr;
		}
	}

	virtual Seat * GetSeat(const boost::int32_t seat);

	virtual const std::map<uid_type, PlayerInterface * > & GetPlayers()const
	{
		return players_;
	}

	virtual bool HasRealPlayer();

	virtual Table * GetTable()
	{
		return table_;
	}

	virtual void RegisterRoomEventObserver(RoomEventListener * observer)
	{
		observer_.push_back(observer);
	}

    virtual void Signal(boost::int32_t event, PlayerInterface * trigger, EventContext * context = nullptr) override;

    //设置/获取基数
    virtual void SetCardinalScore(std::int32_t score) {
        cardinal_score_ = score;
    }
    virtual std::int32_t GetCardinalScore() {
        return cardinal_score_;
    }

    virtual bool IsRobotRoom() { return is_robot_room; }

protected:
	//返回下一个非空座位
	boost::int32_t GetNextPlayer(const boost::int32_t seat);


    void BroadCast(assistx2::Stream & packet, const std::vector<std::int32_t > & mids);

protected:
	Table * table_ = nullptr;
  std::uint32_t owner_ = 0;
	const roombasecfg_type roombasecfg_;

	std::vector<RoomEventListener * > observer_;

	typedef std::map<uid_type, PlayerInterface * > Players;
	Players players_ ;

    std::int32_t cardinal_score_ = 1;

    bool is_robot_room = true;
};

#endif //_XPOKER_SRC_ROOMBASE_H_



