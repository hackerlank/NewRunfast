#include "RunFastTracer.h"
#include "ProxyCmd.h"
#include "PokerCmd.h"
#include <json_spirit_writer_template.h>
#include "PrivateRoom.h"
#include "GameRoomBase.h"
#include "player_interface.h"
#include <boost/format.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "xPoker.h"
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#include <string>
#include "base64/base64.h"

#include "DataLayer.h"
#include "datemanager.h"

const static std::int16_t XLOGGER_EXEC_SQL = 9900;

RunFastTracer::RunFastTracer() :
    gatewayconnector_(nullptr)
{

}

RunFastTracer::~RunFastTracer()
{

}
int RunFastTracer::Init(assistx2::TcpHanlderWrapper * connector)
{
    gatewayconnector_ = connector;
    room_account_players_.clear();

    return 0;
}

void RunFastTracer::OnGameStart(PrivateRoom * room)
{
    json_spirit::Array array;

    array.push_back(EVENT_GAME_START);
    array.push_back(room->GetID());
    for (Table::Iterator seat = room->GetTable()->begin(); seat != room->GetTable()->end(); ++seat)
    {
        if (seat->ingame() == true)
        {
            const auto & mg = seat->user_->GetGameBaseInfo();
            json_spirit::Array unit;
            unit.push_back(seat->no_);
            unit.push_back(static_cast<std::int32_t>(seat->user_->GetUID()));
            unit.push_back(seat->room_score_);
            unit.push_back(mg.gold());

            std::string strHandCards;
            for (auto iter : seat->holecards_)
            {
                strHandCards += iter->getName() + ",";
            }
            unit.push_back(strHandCards);
            array.push_back(unit);
        }
    }
    array.push_back(room->getRunFastRoomCfg().type);
    array.push_back(room->getRunFastRoomCfg().ju);
    array.push_back(room->GetPlayType());
    array.push_back(room->GetOperation());

    WriteRoomRecord(room->GetID(), array);
}

void RunFastTracer::OnGameOver(GameRoomBase * room)
{
    json_spirit::Array array;

    array.push_back(EVENT_GAME_OVER);
    array.push_back(room->GetID());
    for (Table::Iterator seat = room->GetTable()->begin(); seat != room->GetTable()->end(); ++seat)
    {
        auto paly_time = static_cast<std::int32_t>( time(nullptr) - seat->sitdown_time_);

        const auto & mg = seat->user_->GetGameBaseInfo();
        json_spirit::Array unit;
        unit.push_back(seat->no_);
        unit.push_back(static_cast<std::int32_t>(seat->user_->GetUID()));
        unit.push_back(seat->room_score_);
        unit.push_back(mg.gold());
        unit.push_back(static_cast<std::int32_t>(paly_time));

        const auto & baseinfo = seat->user_->getRoleInfo();
        array.push_back(unit);

		//WriteUserDay(seat->user_->GetUID(), paly_time);
        SendPlayingRecord(seat->user_->GetUID(), baseinfo.gp(), 
            static_cast<std::int32_t>(paly_time));
    }

    WriteRoomRecord(room->GetID(), array);
}

void RunFastTracer::OnGameAccount(const std::int32_t roomid, assistx2::Stream stream)
{
    json_spirit::Array array;

    array.push_back(EVENT_GAME_ACCOUNT);
    auto account = base64_encode((unsigned char const*)stream.GetNativeStream().GetData(), stream.GetNativeStream().GetSize());
    array.push_back(account);

    WriteRoomRecord(roomid, array);
}

void RunFastTracer::WirteGameRecord(std::int32_t roomid, int32_t num,Table* table, 
    const std::string& data_game, std::string uid, std::int32_t ownner, std::int32_t pay, std::string type, int32_t club_id)
{
    if (num == 0 || pay == -1)
    {
        return;
    }
//    assistx2::Stream stream(xProxy::STANDARD_ROUTE_PACKET);
//    stream.Write(static_cast<boost::int32_t>(xProxy::SESSION_TYPE_LOGGER));
//    stream.Write(0);
//    stream.Write(RunFast::Log::XLOGGER_GAME_RECORD);
//    stream.Write(roomid);
//    stream.Write(num);
//    auto count = 0;
//    for (Table::Iterator seat = table->begin(); seat != table->end(); ++seat)
//    {
//        if (seat->user_ == nullptr)
//        {
//            continue;
//        }
//        count += 1;
//        stream.Write(seat->user_->GetUID());
//    }
//    for (auto i = 0; i < 4 - count; ++i)
//    {
//        stream.Write(0);
//    }
//    stream.Write(data_game);
//    stream.Write(uid);
//    stream.Write(ownner);
//    stream.Write(pay);
//    stream.Write(type);
//    stream.Write(club_id);
//    stream.End();

//    gatewayconnector_->SendTo(stream.GetNativeStream());


    /********************************多线程写数据库************************************/
    std::vector<std::int32_t> player_mid(4);
    int i = 0;
    for (Table::Iterator seat = table->begin(); seat != table->end(); ++seat, ++i)
    {
        if (seat->user_ == nullptr)
        {
            continue;
        }
        player_mid[i] = seat->user_->GetUID();
    }

    std::string sql;
    try
    {
      sql = boost::str(boost::format("INSERT INTO  `runfast_log`.`game_record` ("
           " `roomid`, `num`, `player1`, `player2`, `player3`,`player4`,`data_game`,`uid`,`master`,`pay`,`club_id`) VALUES "
           "(%d, %d, %d, %d, %d,%d, '%s','%d',%d,%d,%d);")
           %roomid %num %player_mid[0] %player_mid[1] %player_mid[2] %player_mid[3] %data_game %uid %ownner %pay %club_id);

      LOG(INFO) << "SQL:=" << sql;
    }
    catch (std::exception& e)
    {
      LOG(INFO) << "SetRunFastGameRecord, FAILED, exception:=" << e.what();
      return;
    }

    DataManager::getInstance()->ExecSql(sql);
}

void RunFastTracer::WirteGameRecordSub(std::string id, int32_t num_of_games, const std::string& data_game, int32_t winner,GameRoomBase * room)
{
    if (num_of_games == 0)
    {
        return;
    }
    boost::uuids::random_generator  generator;
    std::stringstream ss;
    ss << generator();

//    assistx2::Stream stream(xProxy::STANDARD_ROUTE_PACKET);
//    stream.Write(static_cast<boost::int32_t>(xProxy::SESSION_TYPE_LOGGER));
//    stream.Write(0);
//    stream.Write(RunFast::Log::XLOGGER_GAME_RECORD_SUB);
//    stream.Write(id);
//    stream.Write(num_of_games);
//    stream.Write(data_game);
//    stream.Write(winner);
//    stream.Write(ss.str());
//    stream.End();

//    gatewayconnector_->SendTo(stream.GetNativeStream());



    /********************************多线程写数据库************************************/
    const std::string& uuid = ss.str();
    const std::string& uid = id;

    std::string sql;
    try
    {
        sql = boost::str(boost::format("INSERT INTO  `runfast_log`.`game_record_sub` "
                                       "(`uid`, `num_of_games`, `data_game`,`winner`,`uuid`) VALUES "
                                       "('%s', %d, '%s',%d, '%s');")
                         % uid % num_of_games %data_game %winner %uuid);

        DLOG(INFO) << "SQL:=" << sql;
    }
    catch (std::exception& e)
    {
        LOG(INFO) << "DataLayer::WriteRunFastGameRecordSub, FAILED, exception:=" << e.what();
        return;
    }

    DataManager::getInstance()->ExecSql(sql);
    /********************************************************************************/


    SendPlayerRecord(room, ss.str());
    SendRoomRecord(room->GetID(), ss.str());
}

void RunFastTracer::SendRoomRecord(std::int32_t roomid, std::string uuid)
{
    auto records = GetPlayRecord(roomid);

//    assistx2::Stream stream(xProxy::STANDARD_ROUTE_PACKET);
//    stream.Write(static_cast<boost::int32_t>(xProxy::SESSION_TYPE_LOGGER));
//    stream.Write(0);
//    stream.Write(RunFast::Log::XLOGGER_ROOM_RECORD);
//    stream.Write(roomid);
//    stream.Write(uuid);
//    stream.Write(records);
//    stream.Write(0);
//    stream.End();

//    gatewayconnector_->SendTo(stream.GetNativeStream());



    /********************************多线程写数据库************************************/
    std::int32_t type = 0;
    auto time_now = time(nullptr);
    std::string sql;
    try
    {
        sql = boost::str(boost::format("INSERT INTO `runfast_log`.`room_record` (`room`, `uuid`, `info`, `timestamp`, `type`) "
                                       " VALUES (%d, '%s', '%s', %d, '%s');")
                         % roomid % uuid % records %time_now % type );

        DLOG(INFO) << "sql= " << sql;
    }
    catch (std::exception& e)
    {
        LOG(INFO) << "DataLayer::WriteRoomRecord, FAILED, exception:=" << e.what();
        return;
    }

    DataManager::getInstance()->ExecSql(sql);
}

void RunFastTracer::SendPlayingRecord(std::int32_t mid,std::int32_t gp,int32_t time)
{
//    assistx2::Stream stream(xProxy::STANDARD_ROUTE_PACKET);
//    stream.Write(static_cast<boost::int32_t>(xProxy::SESSION_TYPE_LOGGER));
//    stream.Write(0);
//    stream.Write(RunFast::Log::XLOGGER_PLAYING_RECORD);
//    stream.Write(mid);
//    stream.Write(time);
//    stream.Write(1);
//    stream.Write(gp);
//    stream.Write(0);

//    stream.End();

//    gatewayconnector_->SendTo(stream.GetNativeStream());


    /********************************多线程写数据库************************************/

    std::int32_t type = 0;
    std::int32_t round_total = 1;
    std::string sql;
    try
    {
        sql = boost::str(boost::format("INSERT INTO `runfast_log`.`playing_log` "
                                       "VALUES (%d, %d, %d, '%d', '%s',CURRENT_DATE()) ON DUPLICATE KEY UPDATE "
                                       " `time_total` = `time_total` + %d, `round_total` = `round_total` +  1")
                        % mid % time % round_total %gp % type %time);

        DLOG(INFO) << sql;
    }
    catch (std::exception& e)
    {
        LOG(INFO) << "DataLayer::WritePlayingRecord Insert, FAILED, exception:=" << e.what();
        return;
    }

     DataManager::getInstance()->ExecSql(sql);
}

void RunFastTracer::SendPlayerRecord(GameRoomBase * room,std::string uuid, const std::int32_t club_id)
{
//    for (Table::Iterator seat = room->GetTable()->begin(); seat != room->GetTable()->end(); ++seat)
//    {
//        assistx2::Stream stream(xProxy::STANDARD_ROUTE_PACKET);
//        stream.Write(static_cast<boost::int32_t>(xProxy::SESSION_TYPE_LOGGER));
//        stream.Write(0);
//        stream.Write(RunFast::Log::XLOGGER_PLAYER_RECORD);
//        stream.Write(static_cast<std::int32_t>(seat->user_->GetUID()));
//        stream.Write(room->GetID());
//        stream.Write(seat->no_);
//        stream.Write(uuid);
//        stream.Write(0);

//        stream.End();

//        gatewayconnector_->SendTo(stream.GetNativeStream());
//    }


    /********************************多线程写数据库************************************/
    for (Table::Iterator seat = room->GetTable()->begin(); seat != room->GetTable()->end(); ++seat)
    {
        auto time_now = time(nullptr);
        std::int32_t mid = static_cast<std::int32_t>(seat->user_->GetUID());
        std::int32_t roomid = static_cast<std::int32_t>(room->GetID());
        std::int32_t seatno = seat->no_;
        std::int32_t type = 0;
        std::string sql;
        try
        {
            sql = boost::str(boost::format("INSERT INTO `runfast_log`.`player_record` "
                                           "(`mid`, `room`, `seat`, `uuid`, `timestamp`,`type`,`club_id`) "
                                           " VALUES (%d, %d , %d, '%s', %d,%d,%d);")
                             % mid % roomid %seatno % uuid %time_now % type %club_id);

            DLOG(INFO) << sql;
        }
        catch (std::exception& e)
        {
            LOG(INFO) << "DataLayer::WritePlayerRecord, FAILED, exception:=" << e.what();
            return;
        }

        DataManager::getInstance()->ExecSql(sql);
    }
}

void RunFastTracer::WritePlayRecord(int32_t roomid, int32_t mid, int32_t seatno, const Cards& playedcards, const Cards& handcards, CardType type)
{
    json_spirit::Array array;

    array.push_back(EVENT_PLAY);
    array.push_back(roomid);

    json_spirit::Array info;
    info.push_back(mid);
    info.push_back(seatno);

    std::string strPlayedCards = "";
    for (auto iter : playedcards)
    {
		//锟斤拷锟斤拷?锟斤拷??锟斤拷锟斤拷锟教★拷3锟斤拷锟斤拷???
		if (iter->getChangeName().size() > 0)
		{
			strPlayedCards += iter->getChangeName() + ",";
		}
		else
		{
			strPlayedCards += iter->getName() + ",";
		}      
    }
    info.push_back(strPlayedCards);
    info.push_back(static_cast<std::int32_t>(type));

    array.push_back(info);
    WriteRoomRecord(roomid, array);
}

void RunFastTracer::WriteGoldLog(int32_t mid, int64_t gold_incr, int64_t now_gold,int32_t type, int32_t target)
{
//    assistx2::Stream stream(xProxy::STANDARD_ROUTE_PACKET);
//    stream.Write(static_cast<boost::int32_t>(xProxy::SESSION_TYPE_LOGGER));
//    stream.Write(0);
//    stream.Write(RunFast::Log::XLOGGER_GOLD_RECORD);
//    stream.Write(mid);
//    stream.Write(type); //type
//    stream.Write(gold_incr);
//    stream.Write(now_gold);
//    stream.Write(0); //action
//    stream.Write(0); //gametype
//    stream.Write(target);
//    stream.End();

//    gatewayconnector_->SendTo(stream.GetNativeStream());



    /********************************多线程写数据库************************************/
    std::int32_t action = 0;
    std::int32_t gametype = 0;
    std::string sql;
    try
    {
        sql = boost::str(boost::format("INSERT INTO `runfast_log`.`gold_log` (`mid`, `type`, `gold`, `nowgold`, `action`,`remark`,`gametype`,`formid`) "
                                       " VALUES (%d, %d, %d, %d, %d, '',%d,%d);")
                         % mid % type % gold_incr % now_gold %action %gametype %target);

        DLOG(INFO) << sql;
    }
    catch (std::exception& e)
    {
        LOG(INFO) << "DataLayer::WriteGoldLog, FAILED, exception:=" << e.what();
        return;
    }

    DataManager::getInstance()->ExecSql(sql);
}

std::string RunFastTracer::GetPlayRecord(int32_t roomid)
{
    std::string records = "";
    auto iter = room_records_.find(roomid);
    if (iter != room_records_.end())
    {
        records = json_spirit::write_string(json_spirit::Value(iter->second));
        room_records_.erase(iter);
    }

    return records;
}

void RunFastTracer::addDisbandPlayers(std::int32_t mid, std::int32_t roomid)
{
    auto iter = players_.find(mid);
    if (iter == players_.end())
    {
        players_.insert(std::make_pair(mid,roomid));
    }
}

void RunFastTracer::SendDisbandMessage(std::int32_t mid)
{
    auto iter = players_.find(mid);
    if (iter != players_.end())
    {
        DLOG(INFO) << "RunFastTracer::SendDisbandMessage: mid:=" << mid;
        assistx2::Stream stream(RunFast::SERVER_NOTIFY_DISBAND);
        stream.Write(mid);
        stream.Write(iter->second);
        stream.End();

        gatewayconnector_->SendTo(stream.GetNativeStream());
        players_.erase(iter);
    }
}

std::map<std::int32_t, std::int32_t>& RunFastTracer::GetDisbandPlayers()
{
    return players_;
}

void RunFastTracer::OnGoldChange(PlayerInterface *player)
{
    if (PlayerInterface::IsRobot(player) == true)
    {
        return;
    }
    assistx2::Stream stream(RunFast::SERVER__UPDATE_GOLD);
    stream.Write(player->GetUID());
    stream.Write(player->GetGameBaseInfo().gold());
    stream.End();
    gatewayconnector_->SendTo(stream.GetNativeStream());
}

void RunFastTracer::SendNotifyMsg(std::int32_t mid)
{
    auto iter = msg_.find(mid);
    if (iter != msg_.end())
    {
        assistx2::Stream msg(iter->second.data(),iter->second.size());
        msg.Insert(mid);
        gatewayconnector_->SendTo(msg.GetNativeStream());
        msg_.erase(iter);
    }
}

void RunFastTracer::PushNotifyMsg(std::int32_t mid, std::string msg)
{
    auto iter = msg_.find(mid);
    if (iter == msg_.end())
    {
        msg_.insert(std::make_pair(mid, msg));
    }
}

void RunFastTracer::UpdateLoginPlayer(std::int32_t gp, std::int32_t mid,bool isadd)
{
    if (gp == 0)
    {
        return;
    }
    
    auto iter = login_num_.find(gp);
    if (iter != login_num_.end())
    {
       if (isadd == true)
       {
           iter->second.insert(mid);
       }
       else
       {
           iter->second.erase(mid);
       }
    }
    else
    {
        if (isadd == true)
        {
            std::set<std::int32_t > tmp;
            tmp.insert(mid);
            login_num_.insert(make_pair(gp, tmp));
        }
    }
}

void RunFastTracer::UpdatePlayingPlayer(std::int32_t gp, std::int32_t mid, bool isadd)
{
    if (gp == 0)
    {
        return;
    }
    auto iter = playing_num_.find(gp);
    if (iter != playing_num_.end())
    {
        if (isadd == true)
        {
            iter->second.insert(mid);
        }
        else
        {
            iter->second.erase(mid);
        }
    }
    else
    {
        if (isadd == true)
        {
            std::set<std::int32_t > tmp;
            tmp.insert(mid);
            playing_num_.insert(make_pair(gp, tmp));
        }
    }
}

void RunFastTracer::WriteOnlineRecord()
{
//    for (auto iter : login_num_)
//    {
//        auto playingnum = 0;
//        auto it = playing_num_.find(iter.first);
//        if (it != playing_num_.end())
//        {
//            playingnum = it->second.size();
//        }
//        assistx2::Stream stream(xProxy::STANDARD_ROUTE_PACKET);
//        stream.Write(static_cast<boost::int32_t>(xProxy::SESSION_TYPE_LOGGER));
//        stream.Write(0);
//        stream.Write(RunFast::Log::XLOGGER_PLAYER_ONLINE);
//        stream.Write(static_cast<std::int32_t>(time(nullptr)));
//        stream.Write(playingnum);
//        stream.Write(iter.first);
//        stream.Write(std::string(""));
//        stream.Write(static_cast<std::int32_t>(iter.second.size()));
//        stream.Write(0); //gametype
//        stream.End();

//        gatewayconnector_->SendTo(stream.GetNativeStream());
//    }


     /********************************多线程写数据库************************************/
    for (auto iter : login_num_)
    {
        auto playing = 0;
        auto it = playing_num_.find(iter.first);
        if (it != playing_num_.end())
        {
            playing = it->second.size();
        }
        std::int32_t day_time = static_cast<std::int32_t>(time(nullptr));
        std::int32_t gp = iter.first;
        std::string type = "";
        std::int32_t lookon = static_cast<std::int32_t>(iter.second.size());
        std::int32_t gametype = 0;
        std::string sql;
        try
        {
            sql = boost::str(boost::format("INSERT INTO `runfast_log`.`player_online` "
                                           " (`day_time`, `playing`, `gp`, `type`, `lookon`,`gametype`) "
                                           " VALUES (%d, %d , %d, '%s', %d,%d);")
                             % day_time % playing %gp % type %lookon % gametype);

            DLOG(INFO) << sql;
        }
        catch (std::exception& e)
        {
            LOG(INFO) << "DataLayer::WriteOnlineRecord, FAILED, exception:=" << e.what();
            return;
        }

        DataManager::getInstance()->ExecSql(sql);
    }
}

void RunFastTracer::SaveRoomAccount(std::int32_t mid, assistx2::Stream stream)
{
    auto iter = room_account_players_.find(mid);
    if (iter != room_account_players_.end())
    {
        iter->second = stream;
    }
    else
    {
        room_account_players_.emplace(mid, stream);
        //room_account_players_.insert(std::make_pair(mid, stream));
    }
}

void RunFastTracer::RemoveRoomAccount(std::int32_t mid)
{
    auto iter = room_account_players_.find(mid);
    if (iter != room_account_players_.end())
    {
        room_account_players_.erase(iter);
    }
}

void RunFastTracer::SendRoomAccount(std::int32_t mid)
{
    auto iter = room_account_players_.find(mid);
    if (iter != room_account_players_.end())
    {
        auto stream = iter->second;
        stream.Insert(mid);
        gatewayconnector_->SendTo(stream.GetNativeStream());
    }
}

void RunFastTracer::SaveAccount(std::int32_t mid, assistx2::Stream stream)
{
    auto iter = account_players_.find(mid);
    if (iter != account_players_.end())
    {
        iter->second = stream;
    }
    else
    {
        account_players_.emplace(mid, stream);
    }
}

void RunFastTracer::RemoveAccount(std::int32_t mid)
{
    auto iter = account_players_.find(mid);
    if (iter != account_players_.end())
    {
        account_players_.erase(iter);
    }
}

void RunFastTracer::SendAccount(std::int32_t mid)
{
    auto iter = account_players_.find(mid);
    if (iter != account_players_.end())
    {
        auto stream = iter->second;
        stream.Insert(mid);
        gatewayconnector_->SendTo(stream.GetNativeStream());
    }
}

void RunFastTracer::SaveMatchMessage(std::int32_t mid, assistx2::Stream stream)
{
    auto iter = match_messages_.find(mid);
    if (iter != match_messages_.end())
    {
        iter->second = stream;
    }
    else
    {
        match_messages_.emplace(mid, stream);
    }
}

void RunFastTracer::RemoveMatchMessage(std::int32_t mid)
{
    auto iter = match_messages_.find(mid);
    if (iter != match_messages_.end())
    {
        match_messages_.erase(iter);
    }
}

void RunFastTracer::SendMatchMessage(std::int32_t mid)
{
    auto iter = match_messages_.find(mid);
    if (iter != match_messages_.end())
    {
        auto stream = iter->second;
        stream.Insert(mid);
        gatewayconnector_->SendTo(stream.GetNativeStream());
    }
}

void RunFastTracer::SaveRoomMessage(std::int32_t mid, assistx2::Stream stream)
{
    assistx2::Stream clone(stream);
    const auto cmd = clone.GetCmd();

    auto iter = vc_save_stream_.find(mid);
    if (iter == vc_save_stream_.end())
    {
        vc_save_stream_.emplace(mid,
            std::vector<std::pair<std::int16_t, assistx2::Stream>>(1, std::make_pair(cmd, clone)));
    }
    else
    {
        auto it = std::find_if(iter->second.begin(), iter->second.end(),
            [cmd](const std::pair<std::int16_t, assistx2::Stream>& value) {
            return cmd == value.first;
        });
        if (it != iter->second.end())
        {
            it->second = clone;
        }
        else
        {
            iter->second.push_back(std::make_pair(cmd, clone));
        }
    }
}

void RunFastTracer::RemoveRoomMessage(std::int32_t mid, const std::int16_t cmd)
{
    auto iter = vc_save_stream_.find(mid);
    if (iter != vc_save_stream_.end())
    {
        auto it = std::find_if(iter->second.begin(), iter->second.end(),
            [cmd](const std::pair<std::int16_t, assistx2::Stream>& value) {
            return cmd == value.first;
        });
        if (it != iter->second.end())
        {
            iter->second.erase(it);
        }
        if (iter->second.size() == 0)
        {
            vc_save_stream_.erase(iter);
        }
    }
}

void RunFastTracer::SendRoomMessage(std::int32_t mid)
{
    auto iter = vc_save_stream_.find(mid);

    if (iter != vc_save_stream_.end())
    {
        for (auto it : iter->second)
        {
            auto stream = it.second;
            stream.Insert(mid);
            gatewayconnector_->SendTo(stream.GetNativeStream());
        }
    }
}

void RunFastTracer::IncrCreateRoomCount(const std::int32_t mid)
{
    std::string sql;
    try
    {
		sql = boost::str(boost::format("INSERT INTO   `runfast_log`.`user_day0` (`mid`, `date`, `paytotal`, `paytimes`, `croom`, `playcount`, `playtime`) VALUES "
			"(%d, CURDATE(), 0, 0, 1, 0,0) ON DUPLICATE KEY UPDATE `croom` = `croom` + 1") % mid);

        DLOG(INFO) << "SQL:=" << sql;
    }
    catch (std::exception& e)
    {
        LOG(INFO) << "WriteMatchLog, FAILED, exception:=" << e.what();
    }

//    SendSqlMessage(sql);
    DataManager::getInstance()->ExecSql(sql);
}

void RunFastTracer::WriteUserDay(const std::int32_t mid, const std::int32_t times, const std::int32_t playcount)
{ 
	std::int32_t paytimes;
	if (-1 == ( paytimes = GetSystemTime() ))
	{
		DLOG(INFO) << "GetSystemTime error: paytimes = " << paytimes;
		return;
	}

	std::string sql;
    try
    {
        sql = boost::str(boost::format("INSERT INTO   `runfast_log`.`user_day0` (`mid`, `date`, `paytotal`, `paytimes`, `croom`, `playcount`, `playtime`) VALUES "
            "(%d, CURDATE(), 0, %d, 0, 1,%d) ON DUPLICATE KEY UPDATE `paytimes` = %d, `playtime` = `playtime` + %d,`playcount` = `playcount` + 1")
             % mid %paytimes %times %paytimes %times);

        DLOG(INFO) << "SQL:=" << sql;
    }
    catch (std::exception& e)
    {
        LOG(INFO) << "WriteUserDay, FAILED, exception:=" << e.what();
    }

//    SendSqlMessage(sql);
    DataManager::getInstance()->ExecSql(sql);
}

void RunFastTracer::UpdateUserDay(const std::int32_t mid, const std::int32_t joinroom)
{
	std::string sql;
	try
	{
        sql = boost::str(boost::format("UPDATE `runfast_log`.`user_day0` SET "
                                       "`joinroom` = `joinroom` + %d  WHERE `mid` = %d AND `date` = CURDATE()") %joinroom  %mid);

		DLOG(INFO) << "SQL:=" << sql;
	}
	catch (std::exception& e)
	{
		LOG(INFO) << "WriteUserDay, FAILED, exception:=" << e.what();
	}

//	SendSqlMessage(sql);
    DataManager::getInstance()->ExecSql(sql);

    std::stringstream key;
    key << "JOINROOMDAY|" << mid;

    DataLayer::getInstance()->removeKey(key.str());
}

void RunFastTracer::SendSqlMessage(const std::string sql)
{
//    assistx2::Stream stream(xProxy::STANDARD_ROUTE_PACKET);
//    stream.Write(static_cast<boost::int32_t>(xProxy::SESSION_TYPE_LOGGER));
//    stream.Write(0);
//    stream.Write(XLOGGER_EXEC_SQL);
//    stream.Write(sql);
//    stream.Write(0);
//    stream.End();

//    gatewayconnector_->SendTo(stream.GetNativeStream());


    /********************************多线程写数据库************************************/
    DataManager::getInstance()->ExecSql(sql);
}

void RunFastTracer::WriteGameIntegral(const std::int32_t& mid, const std::int32_t glod)
{
	std::int32_t last_time = static_cast<std::int32_t>(time(nullptr));
	std::string sql;
	try
	{
		sql = boost::str(boost::format("INSERT INTO `runfast_log`.`user_integral0` "
			"VALUES (0, %d, CURRENT_DATE(), %d, %d)ON DUPLICATE KEY UPDATE `last_time`=%d,`integral`=`integral`+%d")
			% mid % last_time % glod % last_time % glod);

		DLOG(INFO) << "SQL:=" << sql;
	}
	catch (std::exception & e)
	{
		LOG(INFO) << "GetMemberGameFromDB, FAILED, exception:=" << e.what();
	}

//	SendSqlMessage(sql);
    DataManager::getInstance()->ExecSql(sql);
}

void RunFastTracer::WriteChuntianBomb(const std::int32_t mid, const std::int32_t paly_type, const std::int32_t mingtang)
{
	//std::uint8_t times = static_cast<std::uint8_t>(1u);	
	std::string sql;
	try
	{
		sql = boost::str(boost::format("INSERT INTO `runfast_log`.`mingtang0` "
			"VALUES (%d, %d, 1, %d, CURRENT_DATE())ON DUPLICATE KEY UPDATE `times`=`times`+1")
			% mid % paly_type % mingtang);

		DLOG(INFO) << "SQL:=" << sql;
	}
	catch (std::exception & e)
	{
		LOG(INFO) << "GetMemberGameFromDB, FAILED, exception:=" << e.what();
	}

//	SendSqlMessage(sql);
    DataManager::getInstance()->ExecSql(sql);
}

int32_t RunFastTracer::GetSystemTime()
{
	std::string str_time = boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time());

	int pos = str_time.find('T') + 1;
	std::string new_time = str_time.substr(pos);
	if (0 == new_time.size())
	{
		return -1;
	}

	//str_time.replace(pos,1,std::string("-"));  
	//str_time.replace(pos + 3,0,std::string(":"));  
	//str_time.replace(pos + 6,0,std::string(":"));  

	int32_t time = -1;
	try
	{
		time = boost::lexical_cast<int32_t>(new_time.c_str());
	}
	catch (boost::bad_lexical_cast &e)
	{
		DLOG(INFO) << "boost::lexical_cast error: " << e.what();
	}

	return time;
}
