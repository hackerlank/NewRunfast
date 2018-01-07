#ifndef _RUNFAST4_ROOM_H_
#define _RUNFAST4_ROOM_H_

#include "private_room.h"
#include "GameDataMgr.h"

class RunFast4RoomMgr;
class Run4Referee;
class RunFast4Room :public PrivateRoom
{
public:
    RunFast4Room(const boost::int32_t roomid, const runfastroomcfg & cfg);

    virtual ~RunFast4Room();
    virtual boost::int32_t OnMessage(PlayerInterface * player, assistx2::Stream * packet);

    virtual void OnTimer(boost::shared_ptr<assistx2::timer2::TimerContext > context);
protected:
    virtual void SendTableSnapShot(PlayerInterface * player);

    virtual void OnStartGame();

    virtual void OnGameOver(std::int32_t seatno);

    virtual void OnRoomAccount(bool isreturn = false);
private:
    void OnDeal();

    void OnPlay(PlayerInterface * player, assistx2::Stream * packet);

    void BroadCastPlayedCard(std::int32_t seatno, const Cards& cards,bool isend = false);

    void BroadCastNextPlayer(std::int32_t seatno, PlayerInterface * player = nullptr);

    void SendPlayResult(PlayerInterface * player, std::int32_t seatno, std::int32_t err);

    void OnForceGameOver();

    void OnBombAccountEvent(int32_t seatno);

    void OnAccountEvent(int32_t seatno);

    void OnDataRecord(int32_t winner);

    bool OnNextSeatPlay(std::int32_t seatno,bool& isyaobuqi);

    void OnEndPlay(Seat* seat);

    void OnPlayedNil(Seat* seat);
private:
    Run4Referee* rfReferee_;
    std::int32_t card_count_ = 16;
};

#endif
