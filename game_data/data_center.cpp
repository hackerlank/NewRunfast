#include "data_center.h"
#include <sstream>
#include "data_layer.h"
#include "private_room.h"
#include <json_spirit_writer_template.h>
#include <json_spirit_reader_template.h>
#include "runfast_room_mgr.h"
#include "table.h"
#include "player_interface.h"

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>


class DataCenterImpl
{
public:
    typedef struct
    {
        std::int32_t winner = 0;
        std::int32_t owner = 0;
        std::int32_t num = 0;
        std::int32_t createtime = 0;
        std::int32_t playtype = 0;
        std::int32_t operation = 0;
        std::int32_t players_num = 0;
        std::int32_t proxy_mid = 0;
        std::vector<std::pair<std::int32_t, std::int32_t>> players;
    }stRoomInfo;
public:
    DataCenterImpl();
    ~DataCenterImpl();
    json_spirit::Value StringToJson(const std::string& str);
    void ParseRoomInfo(const std::string& str, stRoomInfo& room_info);
    void ParseRoomData(std::int32_t roomid);
    std::vector<std::string> split(std::string str, std::string pattern);
public:
    std::map<std::int32_t, stUserInfo> user_info_;
    std::map<std::int32_t,std::vector<std::int32_t>> full_room_;
    std::map<std::int32_t, GameDataMgr::GameData> user_game_data_;
    RunFastRoomMgr* room_mgr_;
};

DataCenter::DataCenter():
   pImpl_(new DataCenterImpl)
{

}

DataCenter::~DataCenter()
{

}

void DataCenter::init(RunFastRoomMgr* mgr)
{
    pImpl_->room_mgr_ = mgr;
}

bool DataCenter::ReSetRoom(RoomInterface* room,std::int32_t& owner)
{
    std::string info;
    auto res = DataLayer::getInstance()->GetRoomInfo(room->GetID(), info);
    if (res == false)
    {
        return false;
    }

    DataCenterImpl::stRoomInfo room_info;
    pImpl_->ParseRoomInfo(info, room_info);
    
    auto runfastroom = dynamic_cast<PrivateRoom*>(room);
    runfastroom->SetGameNum(room_info.num);
    runfastroom->SetWinnerMid(room_info.winner);
    runfastroom->SetCreateTime(room_info.createtime);
    runfastroom->SetPlayType(room_info.playtype);
    runfastroom->SetOperation(room_info.operation);
    runfastroom->ReSetTable(new Table("",room_info.players_num));
    runfastroom->SetProxyMid(room_info.proxy_mid);

    owner = room_info.owner;

    std::vector<std::int32_t> players;
    for (auto iter : room_info.players)
    {
        stUserInfo user_info;
        user_info.room = room;
        user_info.score = iter.second;
        pImpl_->user_info_.insert(std::make_pair(iter.first, user_info));
        players.push_back(iter.first);
    }

    if (static_cast<std::int32_t>(room_info.players.size())
        == room_info.players_num)
    {
        pImpl_->full_room_.insert(std::make_pair(room->GetID(), players));
    }

    return true;
}

void DataCenter::RemoveUserInfo(std::int32_t mid)
{
    auto iter = pImpl_->user_info_.find(mid);
    if (iter != pImpl_->user_info_.end())
    {
        pImpl_->user_info_.erase(iter);
    }
}

void DataCenter::RemoveFullRoom(std::int32_t roomid)
{
    auto iter = pImpl_->full_room_.find(roomid);
    if (iter != pImpl_->full_room_.end())
    {
        pImpl_->full_room_.erase(iter);
    }
}

bool DataCenter::CheckRoomIsFull(std::int32_t mid, std::int32_t roomid)
{
    auto iter = pImpl_->full_room_.find(roomid);
    if (iter == pImpl_->full_room_.end())
    {
        return false;
    }
    auto it = std::find_if(iter->second.begin(), iter->second.end(), [&mid](const std::int32_t value) {
        return mid == value;
    });
    if (it == iter->second.end())
    {
        return true;
    }

    return false;
}

bool DataCenter::FindMyRoom(std::int32_t mid, stUserInfo& userinfo)
{
    auto iter = pImpl_->user_info_.find(mid);
    if (iter != pImpl_->user_info_.end())
    {
        userinfo.room = iter->second.room;
        userinfo.score = iter->second.score;
        auto it = pImpl_->user_game_data_.find(mid);
        if (it != pImpl_->user_game_data_.end())
        {
            userinfo.game_data = it->second;
        }
        else
        {
            userinfo.game_data.bomb_count = 0;
            userinfo.game_data.game_count = 0;
            userinfo.game_data.max_score = 0;
            userinfo.game_data.sum_scroe = 0;
            userinfo.game_data.win_count = 0;
            userinfo.game_data.lost_count = 0;
        }
        return true;
    }
    

    return false;
}

std::string DataCenter::MakeRoomInfo(std::int32_t num, std::int32_t owner, std::int32_t playtype, std::int32_t operation,
    std::int32_t winner, std::int32_t createtime, std::int32_t players, std::int32_t proxymid, Table* table)
{
    std::stringstream ss;
    ss << "{";
    ss << "\"num\" : \"" << num << "\",";
    ss << "\"owner\" : \"" << owner << "\",";
    ss << "\"winner\" : \"" << winner << "\",";
    ss << "\"playtype\" : \"" << playtype << "\",";
    ss << "\"operation\" : \"" << operation << "\",";
    ss << "\"createtime\" : \"" << createtime << "\",";
    ss << "\"players\" : \"" << players << "\",";
    ss << "\"proxymid\" : \"" << proxymid << "\",";
    ss << "\"info\" : [";

    std::stringstream info;
    for (auto seat = table->begin(); seat != table->end(); ++seat)
    {
        if (seat->user_ == nullptr)
        {
            continue;
        }
        if ( !info.str().empty() )
        {
            info << ",";
        }

        info << "{\"mid\" : \"" << seat->user_->GetUID() << "\"," << "\"score\" : \"" << seat->room_score_ << "\"}";
    }
    ss << info.str() << "]}";

    return ss.str();
}

void DataCenter::UpdateUserInfo(const std::int32_t mid)
{
    auto iter = pImpl_->user_info_.find(mid);
    if (iter == pImpl_->user_info_.end())
    {
        return;
    }

    auto runfastroom = dynamic_cast<PrivateRoom*>(iter->second.room);

    std::string info;
    auto res = DataLayer::getInstance()->GetRoomInfo(runfastroom->GetID(), info);
    if (res == false)
    {
        runfastroom->SetGameNum(runfastroom->getRunFastRoomCfg().ju);
        pImpl_->room_mgr_->RemovePrivateRoom(iter->second.room->GetID());
        pImpl_->user_info_.erase(iter);
        auto it = pImpl_->full_room_.find(runfastroom->GetID());
        if (it != pImpl_->full_room_.end())
        {
            pImpl_->full_room_.erase(it);
        }
        return;
    }

    DataCenterImpl::stRoomInfo room_info;
    pImpl_->ParseRoomInfo(info, room_info);
    pImpl_->ParseRoomData(runfastroom->GetID());

    runfastroom->SetGameNum(room_info.num);
    runfastroom->SetWinnerMid(room_info.winner);

    std::vector<std::int32_t> players;
    for (auto it : room_info.players)
    {
        if (iter->first == it.first)
        {
            iter->second.score = it.second;
        }
    }
}

DataCenterImpl::DataCenterImpl()
{
}

DataCenterImpl::~DataCenterImpl()
{
}

void DataCenterImpl::ParseRoomInfo(const std::string& str, stRoomInfo& room_info)
{
    auto json = StringToJson(str);
    auto & obj = json.get_obj();

    json_spirit::Array arr_info;
    for (auto iter : obj)
    {
        if (iter.name_ == "num")
        {
            room_info.num = std::stoi(iter.value_.get_str());

            DLOG(INFO) << "DataCenter::ReSetRoom:num" << room_info.num;
            continue;
        }
        if (iter.name_ == "owner")
        {
            room_info.owner = std::stoi(iter.value_.get_str());
            DLOG(INFO) << "DataCenter::ReSetRoom:owner" << room_info.owner;
            continue;
        }
        if (iter.name_ == "winner")
        {
            room_info.winner = std::stoi(iter.value_.get_str());
            DLOG(INFO) << "DataCenter::ReSetRoom:winner" << room_info.winner;
            continue;
        }
        if (iter.name_ == "playtype")
        {
            room_info.playtype = std::stoi(iter.value_.get_str());
            DLOG(INFO) << "DataCenter::ReSetRoom:type" << room_info.playtype;
            continue;
        }
        if (iter.name_ == "operation")
        {
            room_info.operation = std::stoi(iter.value_.get_str());
            DLOG(INFO) << "DataCenter::ReSetRoom:operation" << room_info.operation;
            continue;
        }
        if (iter.name_ == "createtime")
        {
            room_info.createtime = std::stoi(iter.value_.get_str());
            DLOG(INFO) << "DataCenter::ReSetRoom:tag" << room_info.createtime;
            continue;
        }
        if (iter.name_ == "players")
        {
            room_info.players_num = std::stoi(iter.value_.get_str());
            DLOG(INFO) << "DataCenter::ReSetRoom:players_num" << room_info.players_num;
        }
        if (iter.name_ == "proxymid")
        {
            room_info.proxy_mid = std::stoi(iter.value_.get_str());
            DLOG(INFO) << "DataCenter::ReSetRoom:proxy_mid" << room_info.proxy_mid;
        }
        if (iter.name_ == "info")
        {
            arr_info = iter.value_.get_array();
            continue;
        }
    }
    for (auto iter : arr_info)
    {
        auto mid = 0;
        auto score = 0;
        for (auto it : iter.get_obj())
        {
            if (it.name_ == "mid")
            {
                mid = atoi(it.value_.get_str().c_str());
                DLOG(INFO) << "DataCenter::ReSetRoom:mid" << mid;
                continue;
            }
            if (it.name_ == "score")
            {
                score = atoi(it.value_.get_str().c_str());
                DLOG(INFO) << "DataCenter::ReSetRoom:score" << score;
                continue;
            }
        }
        if (mid == 0)
        {
            continue;
        }
        room_info.players.push_back(std::make_pair(mid,score));
    }
}

void DataCenterImpl::ParseRoomData(std::int32_t roomid)
{
    std::string info;
    auto res = DataLayer::getInstance()->GetRoomRecordInfo(roomid, info);
    if (res == false)
    {
        return ;
    }

    auto mid_vec = split(info, ";");
    for (auto iter : mid_vec)
    {
        auto info_vec = split(iter, ":");
        if (info_vec.size() != 2)
        {
            continue;
        }
        auto mid = atoi(info_vec[0].c_str());
        auto data_vec = split(info_vec[1],",");
        if (data_vec.size() != 6)
        {
            continue;
        }
       
        GameDataMgr::GameData game_data;
        game_data.game_count = std::stoi(data_vec[0]);
        game_data.bomb_count = std::stoi(data_vec[1]);
        game_data.max_score = std::stoi(data_vec[2]);
        game_data.win_count = std::stoi(data_vec[3]);
        game_data.lost_count = std::stoi(data_vec[4]);
        game_data.sum_scroe = std::stoi(data_vec[5]);
        user_game_data_.insert(std::make_pair(mid, game_data));
    }

}

json_spirit::Value DataCenterImpl::StringToJson(const std::string& str)
{
    json_spirit::Value json;
    if (str.empty() == false && json_spirit::read_string(str, json) == true)
    {
        if (json.type() == json_spirit::obj_type)
        {
            return json;
        }
    }
    return json;
}

//字符串分割函数
std::vector<std::string> DataCenterImpl::split(std::string str, std::string pattern)
{
    std::string::size_type pos;
    std::vector<std::string> result;
    str += pattern;
    auto size = str.size();

    for (size_t i = 0; i < size; i++)
    {
        pos = str.find(pattern, i);
        if (pos < size)
        {
            std::string s = str.substr(i, pos - i);
            if (s.empty())
            {
                continue;
            }
            DLOG(INFO) << s;
            result.push_back(s.c_str());
            i = pos + pattern.size() - 1;
        }
    }
    return result;
}
