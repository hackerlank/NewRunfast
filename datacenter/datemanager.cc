#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

#include "datemanager.h"
#include "ThreadPool.hpp"
#include "DBThreadPool.hpp"
#include "private_room.h"
#include "table.h"
#include "httpclient.h"
#include "config_mgr.h"
#include <iostream>
#include <chrono>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include "timer_helper.h"
#include "memberfides.pb.h"
#include <assistx2/configure.h>
#include "player_interface.h"
#include "xpoker.h"
#include "data_layer.h"
#include <boost/format.hpp>

//春天=1，炸弹=2
static const std::vector<std::string> MINGTANG_NAME = {"", "春天", "炸弹"};

class DataManagerImpl
{
public:
    DataManagerImpl(DataManager * owner);
    ~DataManagerImpl();
    void TestTime();
    void TestDBPool();
    bool IsValid(const std::int32_t& mingtang);
public:
    DataManager * owner_;
    std::shared_ptr<ThreadPool> pool_ = nullptr;
    std::shared_ptr<DBThreadPool> db_pool_ = nullptr;
    std::shared_ptr<HttpClient> http_client_;
    std::string addr_url_;
    bool is_enable_center_ = false;
    bool is_enable_dbpool_ = false;
};

DataManager::DataManager():
 pImpl_(new DataManagerImpl(this))
{

}

DataManager::~DataManager()
{

}

void DataManager::Init()
{
    auto app_config = ConfigMgr::getInstance()->GetAppCfg();
    auto res = app_config->getConfig("DataCenter", "url", pImpl_->addr_url_);
    if (res == true)
    {
        pImpl_->is_enable_center_ = true;
        LOG(INFO) << "Enable DataManager url:=" << pImpl_->addr_url_ << ",is_enable_center_:="
            << pImpl_->is_enable_center_;

        pImpl_->pool_ = std::make_shared<ThreadPool>(4);
        pImpl_->http_client_ = std::make_shared<HttpClient>();

        //GlobalTimerProxy::getInstance()->NewTimer(std::bind(&DataManagerImpl::TestTime, pImpl_.get()), 1);
    }

    std::int32_t value = 2;
    res = app_config->getConfig("DataManager", "db_pool", value);
    if (res == true)
    {
        if (value > 4)
        {
            value = 2;
        }
        LOG(INFO) << "Enable DBThreadPool db_pool:=" << value;

        pImpl_->is_enable_dbpool_ = true;
        pImpl_->db_pool_ = std::make_shared<DBThreadPool>(value);

        //GlobalTimerProxy::getInstance()->NewTimer(std::bind(&DataManagerImpl::TestDBPool, pImpl_.get()), 1);
    }
}

void DataManager::PlayingPlayer(PrivateRoom* room)
{
    if (pImpl_->is_enable_center_ == false) return;

    auto table = room->GetTable();
    for (Table::Iterator iter = table->begin(); iter != table->end(); ++iter)
    {
        if (iter->user_ == nullptr)
        {
            continue;
        }
        
        rapidjson::Document doc;
        doc.SetObject();
        rapidjson::Document::AllocatorType &allocator = doc.GetAllocator();
        doc.AddMember("mid", static_cast<std::int32_t>(iter->user_->GetUID()), allocator);
        doc.AddMember("appId", "1", allocator);
        doc.AddMember("gameStartTime", static_cast<std::int32_t>(iter->sitdown_time_), allocator);
        doc.AddMember("gameEndTime", static_cast<std::int32_t>(time(nullptr)), allocator);
        doc.AddMember("playId", rapidjson::Value(room->getRunFastRoomCfg().type.c_str(), allocator).Move(), allocator);
        doc.AddMember("playName", rapidjson::Value(room->getRunFastRoomCfg().name.c_str(), allocator).Move() , allocator);
        doc.AddMember("logType", "4", allocator);
        doc.AddMember("terminalType", iter->user_->getRoleInfo().gp(), allocator);
        doc.AddMember("ip", rapidjson::Value(iter->user_->GetLoginAddr().c_str(),allocator).Move(), allocator);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer, rapidjson::Document::EncodingType, rapidjson::ASCII<>> writer(buffer);
        doc.Accept(writer);

        std::string str(buffer.GetString(), buffer.GetLength());

        pImpl_->pool_->AddTask([str,this]() {
             pImpl_->http_client_->Post_Json(pImpl_->addr_url_, str, nullptr);
        });
    }
    
}

void DataManager::CreateRoomPlayer(PrivateRoom* room, bool isfree)
{
    if (pImpl_->is_enable_center_ == false) return;

    auto cost = room->getRunFastRoomCfg().cost;
    if (isfree == true)
    {
        cost = 0;
    }
    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Document::AllocatorType &allocator = doc.GetAllocator();
    doc.AddMember("mid", static_cast<std::int32_t>(room->GetOwner()), allocator);
    doc.AddMember("appId", "1", allocator);
    doc.AddMember("createRoomTime", static_cast<std::int32_t>(room->GetCreateTime()), allocator);
    doc.AddMember("playId", rapidjson::Value(room->getRunFastRoomCfg().type.c_str(), allocator).Move(), allocator);
    doc.AddMember("playName", rapidjson::Value(room->getRunFastRoomCfg().name.c_str(), allocator).Move(), allocator);
    doc.AddMember("logType", "5", allocator);
    doc.AddMember("playNum", room->getRunFastRoomCfg().ju, allocator);
    doc.AddMember("gold", cost, allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer,rapidjson::Document::EncodingType,rapidjson::ASCII<>> writer(buffer);
    doc.Accept(writer);

    std::string str(buffer.GetString(),buffer.GetLength());

    pImpl_->pool_->AddTask([str, this]() {
        pImpl_->http_client_->Post_Json(pImpl_->addr_url_, str,nullptr);
    });
}

void DataManager::MingTang(const PlayerInterface *player, const std::int32_t mingtang, const PrivateRoom* room)
{
    if (pImpl_->is_enable_center_ == false) return;

    try
    {
        if (pImpl_->IsValid(mingtang) == false)
        {
            return;
        }
    DLOG(INFO) << "MingTang = " << MINGTANG_NAME[mingtang];
    DLOG(INFO) << "playName = " << room->getRunFastRoomCfg().name.c_str();
    DLOG(INFO) << "playId = " << room->getRunFastRoomCfg().type.c_str();
        rapidjson::Document doc;
        doc.SetObject();
        rapidjson::Document::AllocatorType &allocator = doc.GetAllocator();
        doc.AddMember("mid", static_cast<std::int32_t>(player->GetUID()), allocator);
        doc.AddMember("appId", "1", allocator);
        doc.AddMember("createTime", static_cast<std::int32_t>(time(nullptr)), allocator);
        doc.AddMember("name", rapidjson::Value(MINGTANG_NAME[mingtang].c_str(), allocator).Move(), allocator);
        doc.AddMember("playId", rapidjson::Value(room->getRunFastRoomCfg().type.c_str(), allocator).Move(), allocator);
        doc.AddMember("playName", rapidjson::Value(room->getRunFastRoomCfg().name.c_str(), allocator).Move() , allocator);
        doc.AddMember("type", static_cast<std::int32_t>(mingtang), allocator);
        doc.AddMember("logType", "6", allocator);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer, rapidjson::Document::EncodingType, rapidjson::ASCII<>> writer(buffer);
        doc.Accept(writer);

        std::string str(buffer.GetString(), buffer.GetLength());

        pImpl_->pool_->AddTask([str, this]() {
            pImpl_->http_client_->Post_Json(pImpl_->addr_url_, str, nullptr);
        });
    }
    catch (...)
    {
        LOG(ERROR) << "MingTang:=" << MINGTANG_NAME[mingtang];
    }
}

void DataManager::BombSpringStatistics(PrivateRoom *room)
{
    if (pImpl_->is_enable_center_ == false) return;

    try
    {
        int i = 0;
        rapidjson::Document doc;
        doc.SetObject();
        rapidjson::Document::AllocatorType &allocator = doc.GetAllocator();

        doc.AddMember("logType", "7", allocator);
        doc.AddMember("appId", "1", allocator);
        doc.AddMember("createTime", static_cast<std::int32_t>(room->GetCreateTime()), allocator);
        doc.AddMember("playId", rapidjson::Value(room->getRunFastRoomCfg().type.c_str(), allocator).Move(), allocator);
        doc.AddMember("playName", rapidjson::Value(room->getRunFastRoomCfg().name.c_str(), allocator).Move() , allocator);
        doc.AddMember("roomId", static_cast<std::int32_t>(room->GetID()), allocator);

        auto table = room->GetTable();
        for (Table::Iterator iter = table->begin(); iter != table->end(); ++iter)
        {
            if (iter->user_ == nullptr)
            {
                continue;
            }

            std::string tmp1= boost::str(boost::format("player%d_bomb1") %++i);
            rapidjson::Value bomb1(tmp1.c_str(), allocator);
            doc.AddMember(bomb1.Move(), static_cast<std::int32_t>(iter->user_->mingtang_data_.bomb_times_one), allocator);

            std::string tmp2 = boost::str(boost::format("player%d_bomb2") %i);
            rapidjson::Value bomb2(tmp2.c_str(), allocator);
            doc.AddMember(bomb2.Move(), static_cast<std::int32_t>(iter->user_->mingtang_data_.bomb_times_two), allocator);

            std::string tmp3 = boost::str(boost::format("player%d_bomb3") %i);
            rapidjson::Value bomb3(tmp3.c_str(), allocator);
            doc.AddMember(bomb3.Move(), static_cast<std::int32_t>(iter->user_->mingtang_data_.bomb_times_n), allocator);

            std::string tmp4 = boost::str(boost::format("player%d_spring") %i);
            rapidjson::Value spring(tmp4.c_str(), allocator);
            doc.AddMember(spring.Move(), static_cast<std::int32_t>(iter->user_->mingtang_data_.spring_times), allocator);

            DLOG(INFO) << "seat" << iter->no_ << " spring = " << iter->user_->mingtang_data_.spring_times;
            DLOG(INFO) << "seat" << iter->no_ << " bomb1 = " << iter->user_->mingtang_data_.bomb_times_one;
            DLOG(INFO) << "seat" << iter->no_ << " bomb2 = " << iter->user_->mingtang_data_.bomb_times_two;
            DLOG(INFO) << "seat" << iter->no_ << " bomb_n = " << iter->user_->mingtang_data_.bomb_times_n;

            DLOG(INFO) << "bomb1 = " << tmp1;
            DLOG(INFO) << "bomb2 = " << tmp2;
            DLOG(INFO) << "bomb3 = " << tmp3;
            DLOG(INFO) << "spring = " << tmp4;
        }
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer, rapidjson::Document::EncodingType, rapidjson::ASCII<>> writer(buffer);
        doc.Accept(writer);

        std::string str(buffer.GetString(), buffer.GetLength());

        pImpl_->pool_->AddTask([str,this]() {
            pImpl_->http_client_->Post_Json(pImpl_->addr_url_, str, nullptr);
        });
    }
    catch (...)
    {
        LOG(ERROR) << "Bomb and spring Statistics FAILED";
    }
}

void DataManager::ExecSql(const std::string& sql)
{
    if (pImpl_->is_enable_dbpool_ == false) return;

    pImpl_->db_pool_->AddTask([sql](const std::shared_ptr<Database> db) {
        DLOG(INFO) << "ExecSql sql:=" << sql;
        if (db->PExecute(sql.c_str()) == assistx2::error_code::SQL_EXEC_FAILED)
        {
            LOG(INFO) << "ExecSql.PExecute FAILED. error:=" << db->GetLastError()
                << ",sql:=" << sql;
        }
    });
}

void DataManager::Test()
{
    if (pImpl_->is_enable_center_ == false) return;

    auto mid = rand() % 1000 + 50000;
    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Document::AllocatorType &allocator = doc.GetAllocator();
    doc.AddMember("mid", mid, allocator);
    doc.AddMember("appId", "4", allocator);
    doc.AddMember("gameStartTime", static_cast<std::int32_t>(time(nullptr)), allocator);
    doc.AddMember("gameEndTime", static_cast<std::int32_t>(time(nullptr)), allocator);
    doc.AddMember("playId",20, allocator);
    std::string name = "testtesttest";
    doc.AddMember("playName", rapidjson::Value(name.c_str(), allocator).Move(), allocator);
    doc.AddMember("logType", "4", allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer, rapidjson::Document::EncodingType, rapidjson::ASCII<>> writer(buffer);
    doc.Accept(writer);

    std::string str(buffer.GetString(), buffer.GetLength());
    DLOG(INFO) << "Test:" << str;

    pImpl_->pool_->AddTask([str, this]() {
        pImpl_->http_client_->Post_Json(pImpl_->addr_url_, str, nullptr);
    });
}

DataManagerImpl::DataManagerImpl(DataManager * owner):
 owner_(owner)
{
}

DataManagerImpl::~DataManagerImpl()
{
}

void DataManagerImpl::TestTime()
{
    for (auto i = 0; i < 1000; i++)
    {
        owner_->Test();
    }

    GlobalTimerProxy::getInstance()->NewTimer(std::bind(&DataManagerImpl::TestTime, this), 1);
}

void DataManagerImpl::TestDBPool()
{
    if (is_enable_dbpool_ == false) return;

    for (auto i = 0; i <= 1000; i++)
    {
        std::string sql_buffer = "INSERT INTO `game_record` ( `roomid`, `num`, `player1`, `player2`, `player3`,`player4`, `data_game`,`uid`,`master`,`type`,`pay`) VALUES (383054, 2, 51972, 51936, 51969, 51958,'-12;12;0;0;','920649_1497870574',51972,11,1);";
        owner_->ExecSql(sql_buffer);
        sql_buffer = "INSERT INTO `player_record` (`mid`, `room`, `seat`, `uuid`, `timestamp`,`type`)  VALUES (824, 114968 , 4, 'efbc2303-6561-4087-9924-168963ea4290', 1497929412,3);";
        owner_->ExecSql(sql_buffer);
    }

    GlobalTimerProxy::getInstance()->NewTimer(std::bind(&DataManagerImpl::TestDBPool, this), 1);
}

bool DataManagerImpl::IsValid(const int32_t &mingtang)
{
    if(mingtang <= 0 || mingtang >= static_cast<std::int32_t>(MINGTANG_NAME.size())){
        return false;
    }

    return true;
}
