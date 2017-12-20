#ifndef _XNNPOKER_SRC_PLAYER_INTERFACE_H_
#define _XNNPOKER_SRC_PLAYER_INTERFACE_H_

#include <set>

#include <assistx2/tcphandler_wrapper.h>

#include "xPoker.h"

#include "membercommongame.pb.h"
#include "membergame.pb.h"
#include "memberfides.pb.h"
#include "player_day_data.h"

#include "player_mingtang.h"

class RoomInterface;
class Seat;
struct PropsBase_type;
class MatchProxy;
class PlayerInterface
{
public:
    PlayerInterface(void) {};
	virtual ~PlayerInterface(void) {};

public:
	enum PlayerAction
	{
		None,				
		Check,	//看牌				
		Fold,	//弃牌				
		Call,	//跟注		
		Raise,	//加注				
		Allin,	//全押
		Bet,	//压注						
		HandHog,	//抢庄
		ShowCards,	//亮牌
		CompareCards,//比牌
	};

	static const std::int32_t 	DISABLE_ROBOT		= -1;
	static const std::int32_t	ROBOT_VS_PLAYER	= 0;
	static const std::int32_t	ROBOT_VS_ROBOT	= 1;

    //玩家名堂数据
    PlayerMingTang mingtang_data_;

	enum
	{
		ROBOT_TYPE,
		REAL_PLAYER_TYPE
	};

	enum LoginType
	{
		LOGIN_FROM_WEB,
		LOGIN_FROM_MOBILE,
		LOGIN_FROM_ALL
	};

	//初始化玩家数据
	virtual boost::int32_t Serialize(bool /*loadorsave*/) = 0;

	virtual void SitDown(boost::int32_t /*seat*/) = 0;

	virtual void SitUp() = 0;

	virtual boost::int32_t GetSeat()const = 0;

	virtual void SetRoomObject(RoomInterface * /*roomobject*/) = 0;

	virtual uid_type GetUID()const = 0;

	virtual RoomInterface * GetRoomObject() = 0;

	virtual void SendTo(const assistx2::Stream &);

	virtual MemberFides & getRoleInfo() /*const*/ = 0;

	virtual void OnGameOver(bool winner) = 0;

	virtual  std::set<uid_type> & GetFriend() = 0;

	virtual const MemberGame & GetGameInfo()const = 0;

	virtual bool GoldPay(const chips_type /*gold*/) = 0;

    virtual bool UpdateScore(const chips_type /*score*/) = 0;

    virtual bool PropsPay(std::int32_t pcate, std::int32_t pframe, std::int32_t num,bool isPay = true) = 0;

	virtual bool ForceGoldPay(const chips_type /*gold*/,chips_type&) = 0;

	virtual boost::int32_t GetRichesRanking()const = 0;

	virtual boost::int32_t GetWinPointRanking()const = 0;

	virtual const PropsBase_type & GetTableProp()const = 0;

	virtual void SetTableProp(const PropsBase_type & /*prop*/) = 0;

	virtual boost::int32_t GetType()const = 0;

	virtual void Destroy() = 0;

	virtual const std::string & GetLoginAddr()const = 0;
    virtual void SetLoginAddr(const std::string& addr) = 0;

    virtual const std::string& GetGPS() const = 0;
    virtual void SetGPS(const std::string& gps) = 0;

	virtual PlayerDayData & GetTodayData() = 0;

	//连接是否断开
	virtual bool GetConnectStatus()const = 0;

	virtual void SetConnectStatus(bool ) = 0;

    virtual void SetLoginStatus(bool)  = 0;
    virtual bool GetLoginStatus() const = 0;

    virtual void SetMatchProxy(std::shared_ptr<MatchProxy> obj) = 0;
    virtual std::shared_ptr<MatchProxy> GetMatchProxy() = 0;

	virtual time_t GetLoginTime()const = 0;

    virtual void SetPoints(std::int32_t points) = 0;

    virtual std::int32_t GetPoints()const = 0;

	virtual void DoHandHog() = 0;

	static bool IsRobot(const PlayerInterface * player);

	virtual LoginType GetLoginType()const = 0;

	virtual std::int32_t GetLoginSource()/*const*/ = 0;

	virtual void SetLoginSource(std::int32_t gp) = 0;

	virtual const MemberCommonGame & GetGameBaseInfo()const = 0;

    virtual int32_t PlayCost(PlayerInterface *player, int32_t cost, bool isPayByOther, int32_t &proxy_mid) = 0;

    virtual int32_t OnMessage(boost::shared_ptr< assistx2::Stream > packet) = 0;

	static PlayerInterface * CreateRealPlayer(uid_type mid);

	static PlayerInterface * CreateRobot(uid_type mid, 
		boost::function<void (PlayerInterface * player, boost::shared_ptr< assistx2::Stream > packet, RoomInterface * room, time_t delay) > send_callback ,
		boost::function<void (PlayerInterface *) >  kickout);
};

#endif

