#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#include "PlayerRoomManager.h"
#include <vector>
#include <map>
#include <algorithm>
#include "ConfigMgr.h"
#include <assistx2/configure.h>
#include <json_spirit_reader_template.h>
#include <json_spirit_writer_template.h>
#include <assistx2/json_wrapper.h>
#include "DataLayer.h"

class PlayerRoomManagerImpl
{
public:
    PlayerRoomManagerImpl(PlayerRoomManager* owner);
    ~PlayerRoomManagerImpl();
    void InsertRoom(const std::int32_t roomid,std::vector<std::int32_t>& rooms);
    void RemoveRoom(const std::int32_t roomid, std::vector<std::int32_t>& rooms);
    void UpdateCreatorListToCache();
    void UpdateCreatorRoomListToCache(const std::int32_t mid, std::vector<std::int32_t>& rooms);
    void ParseCreatorList(const std::string& list);
    void ParseCreatorRoomList(const std::int32_t mid);
public:
    PlayerRoomManager* owner_;
    std::map<std::int32_t, std::vector<std::int32_t>> player_rooms_;
    std::int32_t max_room_num_ = 10;
};

PlayerRoomManager::PlayerRoomManager():
 pImpl_(new PlayerRoomManagerImpl(this))
{

}

PlayerRoomManager::~PlayerRoomManager()
{

}

void PlayerRoomManager::Init()
{
    UpdateConfig();

    std::string list;
    if (DataLayer::getInstance()->GetCreatorList(list) == true)
    {
        pImpl_->ParseCreatorList(list);
    }
}

void PlayerRoomManager::UpdateConfig()
{
    auto cfg_reader = ConfigMgr::getInstance()->GetAppCfg();
    cfg_reader->getConfig("room", "max_create", pImpl_->max_room_num_);
    DLOG(INFO) << "PlayerRoomManager::UpdateConfig max_room_num_:="
        << pImpl_->max_room_num_;
}

void PlayerRoomManager::UpdateCreatorRoomList(const std::int32_t mid)
{
    std::string list;
    if (DataLayer::getInstance()->
        GetCreatorRoomList(mid, list) == false)
    {
        pImpl_->player_rooms_.erase(mid);
        return;
    }

    json_spirit::Value json;
    auto res = json_spirit::read_string(list, json);
    if (res == false) return;

    if (json.type() != json_spirit::array_type) return;

    std::vector<std::int32_t> rooms;
    auto array = json.get_array();
    for (auto iter : array)
    {
        rooms.push_back(assistx2::ToInt32(iter));
    }

    auto iter = pImpl_->player_rooms_.find(mid);
    if (iter != pImpl_->player_rooms_.end())
    {
        iter->second = rooms;
    }
}

bool PlayerRoomManager::CanCreateRoom(const std::int32_t mid)
{
    UpdateCreatorRoomList(mid);

    auto iter = pImpl_->player_rooms_.find(mid);
    if (iter != pImpl_->player_rooms_.end())
    {
        if (static_cast<std::int32_t>(iter->second.size())
            < pImpl_->max_room_num_)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    return true;
}

void PlayerRoomManager::AddRoom(const std::int32_t mid, const std::int32_t roomid)
{
    auto iter = pImpl_->player_rooms_.find(mid);
    if (iter != pImpl_->player_rooms_.end())
    {
        pImpl_->InsertRoom(roomid, iter->second);
        pImpl_->UpdateCreatorRoomListToCache(mid, iter->second);
    }
    else
    {
        auto vec = std::vector<std::int32_t>(1, roomid);
        pImpl_->player_rooms_.emplace(mid, vec);
        pImpl_->UpdateCreatorListToCache();
        pImpl_->UpdateCreatorRoomListToCache(mid, vec);
    }
}

void PlayerRoomManager::DeleteRoom(const std::int32_t mid, const std::int32_t roomid)
{
    auto iter = pImpl_->player_rooms_.find(mid);
    if (iter != pImpl_->player_rooms_.end())
    {
        pImpl_->RemoveRoom(roomid, iter->second);

        if (iter->second.size() == 0)
        {
            pImpl_->player_rooms_.erase(iter);
            pImpl_->UpdateCreatorListToCache();
        }

        pImpl_->UpdateCreatorRoomListToCache(mid, iter->second);
    }
}

bool PlayerRoomManager::IsOwnerRoom(const std::int32_t mid, const std::int32_t roomid)
{
    auto iter = pImpl_->player_rooms_.find(mid);
    if (iter != pImpl_->player_rooms_.end())
    {
        auto it = std::find_if(iter->second.begin(), iter->second.end(), 
            [roomid](const std::int32_t value) {
                return value == roomid;
        });
        if (it != iter->second.end())
        {
            return true;
        }
    }

    return false;
}

PlayerRoomManagerImpl::PlayerRoomManagerImpl(PlayerRoomManager* owner):
 owner_(owner)
{
}

PlayerRoomManagerImpl::~PlayerRoomManagerImpl()
{
}

void PlayerRoomManagerImpl::InsertRoom(const std::int32_t roomid,std::vector<std::int32_t>& rooms)
{
    auto iter = std::find_if(rooms.begin(), rooms.end(), [roomid](const std::int32_t value) {
        return value == roomid;
    });
    if (iter == rooms.end())
    {
        rooms.push_back(roomid);
    }
}

void PlayerRoomManagerImpl::RemoveRoom(const std::int32_t roomid, std::vector<std::int32_t>& rooms)
{
    auto iter = std::find_if(rooms.begin(), rooms.end(), [roomid](const std::int32_t value) {
        return value == roomid;
    });
    if (iter != rooms.end())
    {
        rooms.erase(iter);
    }
}

void PlayerRoomManagerImpl::UpdateCreatorListToCache()
{
    json_spirit::Array array;
    for (auto iter : player_rooms_)
    {
        array.push_back(iter.first);
    }

    DataLayer::getInstance()->UpdateCreatorList(json_spirit::write_string(json_spirit::Value(array)));
}

void PlayerRoomManagerImpl::UpdateCreatorRoomListToCache(const std::int32_t mid, std::vector<std::int32_t>& rooms)
{
    json_spirit::Array array;
    for (auto iter : rooms)
    {
        array.push_back(iter);
    }

    if (rooms.empty())
    {
        DataLayer::getInstance()->DeleteCreatorRoomList(mid);
    }
    else
    {
        DataLayer::getInstance()->UpdateCreatorRoomList(mid, json_spirit::write_string(json_spirit::Value(array)));
    }
}

void PlayerRoomManagerImpl::ParseCreatorList(const std::string& list)
{
    json_spirit::Value json;
    auto res = json_spirit::read_string(list, json);
    if (res == false) return;
  
    if (json.type() != json_spirit::array_type) return;
   
    auto array = json.get_array();
    for (auto iter :  array)
    {
        ParseCreatorRoomList(assistx2::ToInt32(iter));
    }
}

void PlayerRoomManagerImpl::ParseCreatorRoomList(const std::int32_t mid)
{
    std::string list;
    if (DataLayer::getInstance()->
        GetCreatorRoomList(mid,list) == false)
    {
        return;
    }

    json_spirit::Value json;
    auto res = json_spirit::read_string(list, json);
    if (res == false) return;

    if (json.type() != json_spirit::array_type) return;

    auto array = json.get_array();
    for (auto iter : array)
    {
        auto roomid = assistx2::ToInt32(iter);
        auto it = player_rooms_.find(mid);
        if (it != player_rooms_.end())
        {
            InsertRoom(roomid, it->second);
        }
        else
        {
            player_rooms_.emplace(mid, std::vector<std::int32_t>(1, roomid));
        }
    }
}
