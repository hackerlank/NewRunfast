#ifndef _RUNFAST4_ROOM_LAIZI_H_
#define _RUNFAST4_ROOM_LAIZI_H_

#include "PrivateRoom.h"
#include "GameDataMgr.h"
#include "Run4RefereeLaizi.h"

class Run4RefereeLaizi;
class RunFast4RoomMgr;
class Run4Referee;
class RunFast4RoomLaizi :public PrivateRoom
{
public:
    RunFast4RoomLaizi(const boost::int32_t roomid, const runfastroomcfg & cfg);

    virtual ~RunFast4RoomLaizi();
    virtual boost::int32_t OnMessage(PlayerInterface * player, assistx2::Stream * packet);

    virtual void OnTimer(boost::shared_ptr<assistx2::timer2::TimerContext > context);

protected:
    virtual void SendTableSnapShot(PlayerInterface * player);

    virtual void OnStartGame();

    virtual void OnGameOver(std::int32_t seatno);
	
	virtual void OnRoomAccount(bool isreturn = false);

	virtual std::string getName(const std::shared_ptr<CardInterface>& card);

private:
    void OnDeal();

    void OnPlay(PlayerInterface * player, assistx2::Stream * packet);

	void On_Laizi_Play(PlayerInterface * player, assistx2::Stream * packet);
	bool jype_Judgment(const Cards& cards, CardType& type, CardInterface::Face& firstFace, int32_t& outCount, const CardType& prev_type);

	void BroadCastPlayedCard(std::int32_t seatno,/* const Cards& cards,*/ const PlayedCard& playdecard, bool isend = false);
	

    void BroadCastNextPlayer(std::int32_t seatno, PlayerInterface * player = nullptr);

    void SendPlayResult(PlayerInterface * player, std::int32_t seatno, std::int32_t err);

	void SendPlayResult_Laizi(PlayerInterface * player, std::int32_t seatno, std::int32_t err);

    void OnForceGameOver();

    void OnBombAccountEvent(int32_t seatno);

    void OnAccountEvent(int32_t seatno);

    void OnDataRecord(int32_t winner);

    bool OnNextSeatPlay(std::int32_t seatno,bool& isyaobuqi);

    void OnEndPlay(Seat* seat);

    void OnPlayedNil(Seat* seat);

	void OnReSend_Laizi(PlayerInterface * player);
private:
	Run4RefereeLaizi* rfReferee_;
    std::int32_t card_count_ = 16;
	std::vector<std::pair<int32_t, const CardType>> bomb_seatno_type_;
};

#endif
