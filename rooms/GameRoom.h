#ifndef _XPOKER_SRC_GAMEROOM_H_
#define _XPOKER_SRC_GAMEROOM_H_

#include "GameRoomBase.h"

class RoomEventListener;

class GameRoom : public GameRoomBase
{
public:
    GameRoom(const boost::int32_t roomid, const roomcfg_type & cfg);

	virtual ~GameRoom(void);

	virtual boost::int32_t OnMessage(PlayerInterface * player, assistx2::Stream * packet);

	virtual void PeekWinlist( std::vector<HandStrength> & winlist );

	void RoomSnapShot(PlayerInterface * player);

	virtual Referee * GetReferee() 
	{
		return referee_;
	}

	virtual std::time_t GetIdlingTime() override
	{
		return 0;
	}

	virtual void Init(KickCallbcak & call);

public:

	//申请加为好友
	void OnRequestAddFriend(PlayerInterface * player, assistx2::Stream * packet);

	//response
	void OnResponseAddFriend(PlayerInterface * player, assistx2::Stream * packet);


	static void PackShowCardsPacket(boost::shared_ptr<assistx2::Stream > & stream, Seat * seat);

	void OnUseFace( PlayerInterface * player, assistx2::Stream * packet );

	virtual void ForEachLivePlayer(std::function< bool(PlayerInterface *)  >) override;

	virtual std::int32_t GetLivePlayerCount() override;

protected:
	void RoomSnapShot( PlayerInterface * player,  Players & players );

	boost::int32_t BlackListCheck( PlayerInterface * player );

	void BroadCastPlayerEnter(PlayerInterface * player);

	chips_type DeductedTaxes( const chips_type win_amount );

protected:
	//被踢出去的玩家名单
	std::map<uid_type, time_t >blacklisk_; 

	//  荷官
	static const std::int32_t  DEALER_MID = 0;
};

#endif //_XPOKER_SRC_GAMEROOM_H_


