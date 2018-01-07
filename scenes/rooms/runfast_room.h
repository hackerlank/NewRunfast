#ifndef _RUNFAST_ROOM_H_
#define _RUNFAST_ROOM_H_

#include "private_room.h"
#include "GameDataMgr.h"

class RunFastRoomMgr;
class PDKPokerReferee;
class RunFastRoom :public PrivateRoom
{
public:
    RunFastRoom(const boost::int32_t roomid, const runfastroomcfg & cfg, const std::int32_t card_count);

    virtual ~RunFastRoom();
    virtual boost::int32_t OnMessage(PlayerInterface * player, assistx2::Stream * packet);

    virtual void OnTimer(boost::shared_ptr<assistx2::timer2::TimerContext > context);
protected:
    virtual void SendTableSnapShot(PlayerInterface * player);

    virtual void OnStartGame();

    virtual void OnGameOver(std::int32_t seatno);

    virtual void OnRoomAccount(bool isreturn = false);

private:
    void OnDeal();

    typedef boost::shared_ptr<assistx2::Stream> Stream_Ptr;
    typedef boost::shared_ptr<std::map<std::int32_t, Stream_Ptr>> Map_Ptr;

    void DistributionCard(std::int32_t& first_play_seat);
    void OnDistributionCard(std::int32_t& first_play_seat, Map_Ptr& map_stream_ptr);

    void OnPlay(PlayerInterface * player, assistx2::Stream * packet);

    void BroadCastPlayedCard(std::int32_t seatno, const Cards& cards,bool isend = false);

    void BroadCastNextPlayer(std::int32_t seatno,PlayerInterface* player = nullptr);

    void SendPlayResult(PlayerInterface * player, std::int32_t seatno, std::int32_t err);

    void OnForceGameOver();

    void OnBombAccountEvent(int32_t seatno);

    void OnAccountEvent(int32_t seatno);

    void OnDataRecord(int32_t winner);

    bool OnNextSeatPlay(std::int32_t seatno,bool& isyaobuqi);

    void OnEndPlay(Seat* seat);

    void OnPlayedNil(Seat* seat);
private:
    PDKPokerReferee* rfReferee_;
    std::int32_t card_count_ = 16;

    //一个房间内每个用户炸弹限制值
    const std::int32_t limit_value_ = 2;
};

#endif
