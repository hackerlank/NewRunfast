#ifndef _XPOKER_PLAYERBASE_H_
#define _XPOKER_PLAYERBASE_H_

#include <assistx2/delay_loader.h>

#include "player_interface.h"

#include "data_def.h"

class RoomInterface;
class Seat;
class PlayerBase : public PlayerInterface
{
public:
	PlayerBase(uid_type mid, std::int32_t type);

	virtual ~PlayerBase() {}

public:
    int32_t OnMessage(assistx2::Stream& packet);

	virtual void SitDown(boost::int32_t seat)override;

	void SitUp();

	virtual boost::int32_t GetType()const
	{
		return type_;
	}

	virtual boost::int32_t GetSeat()const 
	{ 
		return seat_; 
	}

	virtual void SetRoomObject(RoomInterface * roomobject)
	{
		roomobject_ = roomobject;
		if (roomobject_ != nullptr)
		{
			login_time_ = time(nullptr);
		}
	}

	virtual uid_type GetUID()const
	{
		return uid_;
	}

	virtual RoomInterface * GetRoomObject()
	{
		return roomobject_;
	}

	virtual/* const */MemberFides & getRoleInfo()/* const*/
	{
		return  (roleinfo_);
	}
	
	virtual  std::set<uid_type> & GetFriend()
	{
		return *friends_;
	}

	virtual const std::string & GetLoginAddr()const
	{
		return login_ip_;
	}

    virtual void SetLoginAddr(const std::string& addr)
    {
        login_ip_ = addr;
    }

	virtual bool GetConnectStatus()const 
	{
		return closed_;
	}

	virtual void SetConnectStatus(bool status) 
	{
		closed_ = status;
	}

    virtual void SetLoginStatus(bool status)
    {
        login_ = status;
    }

    virtual bool GetLoginStatus() const
    {
        return login_;
    }

	virtual time_t GetLoginTime()const
	{
		return login_time_;
	}

    virtual void SetPoints(std::int32_t points) override
    {
        points_ = points;
    }

    virtual std::int32_t GetPoints()const override;

	virtual void DoHandHog() override
	{

	}

	virtual LoginType GetLoginType()const
	{
		return login_source_ > 100 ? LOGIN_FROM_MOBILE : LOGIN_FROM_WEB;
	}

	virtual void SetLoginSource(std::int32_t gp)
	{
		login_source_ = gp;
	}

	virtual std::int32_t GetLoginSource()/*const*/;

    virtual void SetMatchProxy(std::shared_ptr<MatchProxy> obj);
    virtual std::shared_ptr<MatchProxy> GetMatchProxy();

    virtual std::int32_t PlayCost(PlayerInterface * player,std::int32_t cost,bool isPayByOther,std::int32_t& proxy_mid);

    virtual const std::string& GetGPS() const
    {
        return gps_;
    }
    virtual void SetGPS(const std::string& gps)
    {
        gps_ = gps;
    }

protected:
	//用户ID
	uid_type		uid_;

	boost::int32_t type_;

	//最近一次坐下的时间 
	time_t sitdowntime_;

	time_t login_time_;

	//坐位ID
	boost::int32_t seat_;

	bool closed_;

    bool login_;

	RoomInterface * roomobject_;

	boost::int32_t win_rank_;

	boost::int32_t riches_rank_;

	std::string login_ip_;

	MemberFides roleinfo_;

    std::int32_t points_ = 0;

	std::int32_t login_source_ = 0;

	assistx2::DelayLoader< std::set<uid_type >  > friends_;

    std::shared_ptr<MatchProxy> match_proxy_ = nullptr;

    std::string gps_;
};

#endif //_XPOKER_PLAYERBASE_H_


