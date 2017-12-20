#pragma once

#include "GameRoom.h"
#include "GameDataMgr.h"

//0:牌局已打玩,1:解散超时,2:所有人同意解散
enum class DisbandType { NUM_NULL, TIME_OUT, ALL_AGREE, NOT_START };

class RunFastRoomMgr;
class PDKPokerReferee;
class PrivateRoom :public GameRoom
{
public:
    PrivateRoom(const boost::int32_t roomid, const runfastroomcfg & cfg);

    virtual ~PrivateRoom();
    virtual boost::int32_t OnMessage(PlayerInterface * player, assistx2::Stream * packet);

    virtual boost::int32_t Enter(PlayerInterface * player);

    virtual boost::int32_t Leave(PlayerInterface * user, std::int32_t err);

    virtual boost::int32_t Kick(const uid_type mid, bool add_to_blacklist);

    virtual void OnTimer(boost::shared_ptr<assistx2::timer2::TimerContext > context);

    virtual void Init(KickCallbcak & call);

    virtual int Disband(PlayerInterface * player);

    virtual const runfastroomcfg & getRunFastRoomCfg() const { return rfroomcfg_; }

    void SetGameNum(std::int32_t num) { num_of_games_ = num; }

    void SetWinnerMid(std::int32_t mid) { winner_mid_ = mid; };

    void SetCreateTime(std::int32_t time) { create_time_ = time; };
    const std::int32_t GetCreateTime() const { return create_time_; };
    void SetPlayType(const std::int32_t type);
    void SetOperation(const std::int32_t value) { operation_ = value; };
    void SetProxyMid(const std::int32_t mid) { proxy_mid_ = mid; };
    void EnableDisbandTimer();
    std::int32_t GetOperation() { return operation_; };
    std::int32_t GetPlayType() { return play_type_; };
protected:
    virtual void SendTableSnapShot(PlayerInterface * player);

    virtual void OnStartGame();

    virtual void OnGameOver(std::int32_t seatno);

    virtual void  OnRoomAccount(bool isreturn = false);

	virtual std::string getName(std::shared_ptr<CardInterface> card);

	enum Mingtang
	{
		CHUNTIAN = 1,
		BOMB
	};

protected:
	void OnReConnect(PlayerInterface * player);

    std::int32_t OnEnterRoom(PlayerInterface * player);

    void SendRoomSnapShot(PlayerInterface * player);

    void BroadCastOnEnterPlayer(PlayerInterface * player);
	
    void OnConnectClose(PlayerInterface* player);

    void OnReady(PlayerInterface* player);

    void OnUnReady(PlayerInterface* player);

    Seat* prevSeat(std::int32_t seatno);

    Seat* nextSeat(std::int32_t seatno);

    void RemoveCards(Seat* seat, const Cards& cards);

    void OnZanli(PlayerInterface * player);

    void OnTuoGuan(PlayerInterface * player, assistx2::Stream * packet);

    void OnBroadMessage(PlayerInterface * player, assistx2::Stream * packet);

	void OnBroadProp(assistx2::Stream* packet);

    void OnComeBack(PlayerInterface * player);

    void OnAddRobot(PlayerInterface * player);

    void ResetWinner(std::int32_t mid);

    void SendMsgList(PlayerInterface * player, std::int32_t begin = -1, std::int32_t size = 10);

    void ReturnGold();

    void OnQueryIfSetGps(PlayerInterface * player, assistx2::Stream* packet);
    void OnSetGps(PlayerInterface * player, assistx2::Stream* packet);
    Seat* GetPrevPlayedSeat(std::int32_t seatno);

    void OnRequestDisbandRoom(PlayerInterface * player);
    void OnVote(PlayerInterface * player, assistx2::Stream* packet);
    void SendVoteMessage(PlayerInterface * player, std::int32_t err);
    void ClearVoteData();
    void OnDisbandRoom(DisbandType type);
    void ClearGameData();
    void ClearRoomData();
    void OnPiaoScore();
protected:
    runfastroomcfg rfroomcfg_;

    PlayerInterface* winner_;

    std::int32_t winner_mid_;

    std::int32_t num_of_games_;

    std::int32_t bet_;

    GameDataMgr game_data_mgr_;

    RunFastRoomMgr* roommgr_;

    Players zanli_players_;

    bool isRobotMode_ = false;

    std::int32_t author_ = 0;
    std::int32_t create_time_ = 0;

    std::vector<std::int32_t> bomb_info_;
    std::int32_t type_ = 0;

    std::int32_t play_type_ = 0;
    std::vector<std::pair<std::string, std::string>> msg_list_;
    std::int32_t start_vote_time_ = 0;
    std::int32_t operation_ = 0;//首局是否显出黑桃3
    std::int32_t room_state_ = 0;
    bool IsPlayed = false;
    std::int32_t proxy_mid_ = 0;
    std::set<uid_type> players_is_set_gps_;
    std::int32_t disband_author_ = Table::INVALID_SEAT;//解散房间发起者座位号
    time_t disband_start_time_ = 0;//解散房间发起时间
};
