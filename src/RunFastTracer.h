#ifndef _RUNFAST_TRACER_H_
#define _RUNFAST_TRACER_H_

#include <json_spirit.h>
#include <assistx2/singleton.h>
#include <assistx2/tcp_handler.h>
#include <assistx2/tcphandler_wrapper.h>
#include "card_interface.h"
#include <set>
#include "RunReferee.h"

class Table;
class PlayerInterface;
class PrivateRoom;
class GameRoomBase;
class RunFastTracer : public Singleton<RunFastTracer >
{
    friend class Singleton<RunFastTracer >;
public:
    enum
    {
        EVENT_GAME_START = 0 ,							//游戏开始
        EVENT_PLAY,
        EVENT_GAME_OVER,
        EVENT_GAME_ACCOUNT
    };
public:
    RunFastTracer();
    virtual ~RunFastTracer();
    int Init(assistx2::TcpHanlderWrapper * connector);
    void WirteGameRecordSub(std::string id, int32_t num_of_games, const std::string& data_game, int32_t winner,GameRoomBase * room);
    void WirteGameRecord(std::int32_t roomid, int32_t num, Table* table, const std::string& data_game,
                         std::string uid,std::int32_t ownner,std::int32_t pay,std::string type,std::int32_t club_id= 0);
    void OnGameStart(PrivateRoom * room);
    void OnGameOver(GameRoomBase * room);
    void WritePlayRecord(int32_t roomid, int32_t mid, int32_t seatno, const Cards& playedcards, const Cards& handcards, CardType type);
    void OnGameAccount(const std::int32_t roomid, assistx2::Stream stream);
    void WriteGoldLog(int32_t mid, int64_t gold_incr, int64_t now_gold,int32_t type,int32_t target = 0);
    std::string GetPlayRecord(int32_t roomid);
    void SendRoomRecord(std::int32_t roomid,std::string uuid);
    void SendPlayingRecord(std::int32_t mid, std::int32_t gp,int32_t time);
    void SendPlayerRecord(GameRoomBase * room, std::string uuid, const std::int32_t club_id= 0);
    void SendDisbandMessage(std::int32_t mid);
    void addDisbandPlayers(std::int32_t mid, std::int32_t roomid);
    std::map<std::int32_t, std::int32_t>& GetDisbandPlayers();
    void OnGoldChange(PlayerInterface *player);
   // void SystemBroadCast(std::string type,std::string msg);
    void PushNotifyMsg(std::int32_t mid, std::string msg);
    void SendNotifyMsg(std::int32_t mid);
    void UpdateLoginPlayer(std::int32_t gp,std::int32_t mid,bool isadd = true);
    void UpdatePlayingPlayer(std::int32_t gp,std::int32_t mid, bool isadd = true);
    void WriteOnlineRecord();
    //老数据-begin
    void SaveRoomAccount(std::int32_t mid, assistx2::Stream stream);
    void RemoveRoomAccount(std::int32_t mid);
    void SendRoomAccount(std::int32_t mid);
    void SaveAccount(std::int32_t mid, assistx2::Stream stream);
    void RemoveAccount(std::int32_t mid);
    void SendAccount(std::int32_t mid);
    //老数据-end

    void SaveMatchMessage(std::int32_t mid, assistx2::Stream stream);
    void RemoveMatchMessage(std::int32_t mid);
    void SendMatchMessage(std::int32_t mid);
    //新数据
    void SaveRoomMessage(std::int32_t mid, assistx2::Stream stream);
    void RemoveRoomMessage(std::int32_t mid,const std::int16_t cmd);
    void SendRoomMessage(std::int32_t mid);

    void IncrCreateRoomCount(const std::int32_t mid);
    void WriteUserDay(const std::int32_t mid, std::int32_t times, const std::int32_t playcount);
	void UpdateUserDay(const std::int32_t mid, const std::int32_t joinroom);
    void SendSqlMessage(const std::string sql);
    void WriteGameIntegral(const std::int32_t &mid, const std::int32_t glod);
    void WriteChuntianBomb(const std::int32_t mid, const std::int32_t paly_type, const std::int32_t mingtang);
private:
	int32_t GetSystemTime();

    template< class T>
    json_spirit::Array & WriteRoomRecord(const int room, const T & t)
    {
        std::map<int, json_spirit::Array >::iterator it = room_records_.find(room);
        if (it != room_records_.end())
        {
            it->second.push_back(t);
        }
        else
        {
            json_spirit::Array tmp;
            tmp.push_back(t);
            it = room_records_.insert(make_pair(room, tmp)).first;
        }

        return it->second;
    }

    assistx2::TcpHanlderWrapper * gatewayconnector_;
    std::map<int, json_spirit::Array > room_records_;
    time_t start_time_;
    std::map<std::int32_t,std::int32_t> players_;
    std::map<std::int32_t, std::string > msg_;
    std::map<std::int32_t, std::set<std::int32_t> > login_num_;
    std::map<std::int32_t, std::set<std::int32_t> > playing_num_;
    std::map<std::int32_t, assistx2::Stream> room_account_players_;
    std::map<std::int32_t, assistx2::Stream> account_players_;
    std::map<std::int32_t, assistx2::Stream> match_messages_;
    std::map<std::int32_t, std::vector< std::pair<std::int16_t, assistx2::Stream> >> vc_save_stream_;
};
#endif
