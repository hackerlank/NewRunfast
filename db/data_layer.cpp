#include "data_layer.h"

#include <thread>

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#include <glog/stl_logging.h>

#include <boost/format.hpp>

#include <assistx2/platform_wrapper.h>
#include <assistx2/database.h>
#include <assistx2/json_wrapper.h>
#include <assistx2/memcached_wrapper.h>
#include <assistx2/string_wrapper.h>
#include <assistx2/configure.h>
#include <assistx2/time_tracer.h>

#include <json_spirit_writer_template.h>
#include <json_spirit_reader_template.h>

#include <cpp_redis_util.h>

#include "config_mgr.h"
#include "protocol.h"
#include "poker_cmd.h"
#include "data_adapter.h"
//#include "runfast_tracer.h"

static const std::int32_t PROPS_TABLE_HAS_FACTOR		= 100000;
static const std::int32_t PROPS_DATABASE_HAS_FACTOR = 40000000;
//memberfides
static const std::int32_t MEMBER_FIDES_TABLE_HAS_FACTOR		= 2000000;
static const std::int32_t MEMBER_GAME_TABLE_HAS_FACTOR = 2000000;

static const std::int32_t MEMBER_GAME_TABLE_FACTOR = 2000000;


DataLayer::DataLayer(void):databaseconnector_(nullptr),  redisconnector_(nullptr), engine_(nullptr), memcached_agent_(nullptr)
{
}

DataLayer::~DataLayer(void)
{
	if (databaseconnector_ != nullptr)
	{
		databaseconnector_->Disconnect();
		delete databaseconnector_;
		databaseconnector_ = nullptr;
	}

	if (redisconnector_ != nullptr)
	{
		delete redisconnector_;
		redisconnector_ = nullptr;
	}
}

std::int32_t DataLayer::Init(boost::asio::io_service * engine)
{
	engine_ = engine;

	if (InitMemcachedAgent() != 0)
	{
		return -1;
	}

    auto cfg_reader = ConfigMgr::getInstance()->GetAppCfg();

	CHECK(cfg_reader->getConfig("system", "database", cfg_database_) == true);

	CHECK(cfg_reader->getConfig("system", "local", local_config_) == true);

	CHECK(cfg_reader->getConfig("system", "prefix", db_prefix_) == true);

	DCHECK(databaseconnector_ == nullptr);
	databaseconnector_ = new Database;
	CHECK_NOTNULL(databaseconnector_);

	const DBConfig_type & cfg = ConfigMgr::getInstance()->getDBConfig();
	if (databaseconnector_->Connect(cfg) == false)
	{
		return -2;
	}

	std::string host;
	std::string port;
    cfg_reader->getConfig("redis", "host", host);
    cfg_reader->getConfig("redis", "port", port);

	DCHECK(redisconnector_ == nullptr);
	redisconnector_ = new cpp_redis::Requestor<cpp_redis::REQUESTOR >(host, port, *engine_);
	if (redisconnector_ == nullptr)
	{
		LOG(ERROR)<<("Initialize INIT FAILED ASSERT(redisconnector_ != nullptr).");
		return -1;
	}

	/*chips_type pool = 0;
	UpdateAwardPool(1000000, pool);
	LOG(INFO) << "UpdateAwardPool pool:=" << pool;*/

	//Tracer::getInstance()->OnWinning(29, "ccc", "7s7cKc7h7d", 450, 1000, time(nullptr));

	return 0;
}

std::int32_t DataLayer::InitMemcachedAgent()
{
	std::string libmemcached_cfg;
	auto reader = ConfigMgr::getInstance()->GetAppCfg();
	CHECK(reader->getConfig("libmemcached", "memcached", libmemcached_cfg) );

	LOG(INFO) << "InitMemcachedAgent memcached:=" << libmemcached_cfg;

	memcached_agent_ = IMemcacheHandler::CreateMemcacheHandler(libmemcached_cfg);
	CHECK_NOTNULL(memcached_agent_.get() );

	std::stringstream key;
	key << "test" << std::this_thread::get_id();

	std::string  value;

	CHECK(memcached_agent_->set(key.str().c_str(), "memcached test OK!"));
	CHECK(memcached_agent_->get(key.str().c_str(), value));
	LOG(INFO) << "InitMemcachedAgent " << value;
	CHECK(memcached_agent_->remove(key.str().c_str()));

	return 0;
}

std::int32_t DataLayer::UpdateGameData( uid_type mid, std::int32_t win_count, std::int32_t round_count,  std::string handstrength, const chips_type maxwin)
{
	std::string sql;
	try
	{
		sql = boost::str(boost::format("UPDATE  `%s`.`membergame%d` "
			"SET `winGame` = `winGame` + %d, `sumGame` = `sumGame` +  %d,  `maxwin` = %d, `maxhs` = '%s' "
			" WHERE `mid` = %d;")
			% db_prefix_.c_str() % (mid / MEMBER_GAME_TABLE_HAS_FACTOR) % win_count  %round_count  %maxwin  %handstrength  %mid);

		DLOG(INFO) << "SQL:=" << sql;
	}
	catch (std::exception & e)
	{
		LOG(INFO) << "GetMemberGameFromDB, FAILED, exception:=" << e.what();
		return -1;
	}

	if (databaseconnector_->PExecute(sql.c_str())== assistx2::error_code::SQL_EXEC_FAILED)
	{
		LOG(INFO) << "UpdateGameData. FAILED. UID:=" << mid;
		return -1;
	}
	else
	{
		return 0;
	}
}

std::int32_t DataLayer::UpdateRunFastScore(uid_type mid, std::int32_t score)
{
    std::stringstream key;
    key << "TMGM" << mid << "|0";
    
    memcached_agent_->remove(key.str());

    std::unique_ptr<IQueryResult> result(databaseconnector_->PQuery("CALL `%s`.`update_score`(%d, %d, @old_score, @new_score); "
        " SELECT @old_score, @new_score;", db_prefix_.c_str(), mid, score));
    if (result.get() == nullptr || result->RowCount() == 0)
    {
        LOG(ERROR) << "Pay, FAILED mid:=" << mid << ", " << databaseconnector_->GetLastError();
        return -1;
    }

    //auto old_score = result->GetItemLong(0, "@old_score");
    auto new_score = result->GetItemLong(0, "@new_score");

    return new_score;
}

std::int32_t DataLayer::PayProps(const uid_type mid, std::int32_t pcate, std::int32_t pframe, std::int32_t num, bool isPay)
{
    std::stringstream key;
    key << "PROPS" << mid;

    memcached_agent_->remove(key.str());

    auto start_time = time(nullptr);
    
    std::unique_ptr<IQueryResult> result(databaseconnector_->PQuery("SELECT * FROM `%s`.`props` "
        "WHERE `mid` = %d AND `pcate` = %d AND `pframe` = %d AND `status` = %d;"
        , db_prefix_.c_str(), mid, pcate, pframe, !isPay));
    if (result.get() == nullptr || result->RowCount() < static_cast<size_t>(num))
    {
        LOG(ERROR) << "PayProps,SELECT FAILED mid:=" << mid << ",num:=" << result->RowCount() <<
            ", " << databaseconnector_->GetLastError();
        return -1;
    }

    if (isPay == true)
    {
        std::unique_ptr<IQueryResult> result(databaseconnector_->PQuery("UPDATE  `%s`.`props` "
            "SET `status` = 1, `usetime` = %d  WHERE `mid` = %d AND `pcate` = %d AND `pframe` = %d AND `status` = 0 LIMIT %d;"
            , db_prefix_.c_str(), static_cast<std::int32_t>(time(nullptr)), mid, pcate, pframe,num));
        if (result.get() == nullptr)
        {
            LOG(ERROR) << "PayProps,UPDATE FAILED mid:=" << mid << ", " << databaseconnector_->GetLastError();
            return -1;
        }
    }
    else
    {
        std::unique_ptr<IQueryResult> result(databaseconnector_->PQuery("UPDATE  `%s`.`props` "
            "SET `status` = 0, `usetime` = 0  WHERE `mid` = %d AND `pcate` = %d AND `pframe` = %d AND `status` = 1 ORDER BY usetime DESC LIMIT %d;"
            , db_prefix_.c_str(), mid, pcate, pframe, num));
        if (result.get() == nullptr)
        {
            LOG(ERROR) << "PayProps,UPDATE FAILED mid:=" << mid << ",num:="  << ", " << databaseconnector_->GetLastError();
            return -1;
        }
    }

    DLOG(INFO) << "DataLayer::PayProps time:=" << start_time - time(nullptr);
    return 0;
}

std::int32_t DataLayer::AddProps(const uid_type mid, std::int32_t pcate, std::int32_t pframe, std::int32_t num)
{
    std::stringstream key;
    key << "PROPS" << mid;

    memcached_agent_->remove(key.str());

    auto start_time = time(nullptr);

    std::string sql;
    try
    {
        sql = boost::str(boost::format("INSERT INTO   `%s`.`props` VALUES "
            "(%d, 0, %d, %d, 0, %d, 0,0,0,0);")
            % db_prefix_.c_str() % mid % pcate %pframe %static_cast<std::int32_t>(time(nullptr)));

        DLOG(INFO) << "SQL:=" << sql;
    }
    catch (std::exception& e)
    {
        LOG(INFO) << "SetRunFastGameRecord, FAILED, exception:=" << e.what();
        return -1;
    }

    for (auto i = 0; i < num; ++i)
    {
        if (databaseconnector_->PExecute(sql.c_str()) == assistx2::error_code::SQL_EXEC_FAILED)
        {

            LOG(INFO) << "SetRunFastGameRecord. FAILED. UID:=" << mid << " error:=" << databaseconnector_->GetLastError();
            return -1;
        }
    }

    DLOG(INFO) << "DataLayer::AddProps time:=" << start_time - time(nullptr);
    return 0;
}

std::int32_t DataLayer::SetRunFastGameRecord(std::int32_t roomid, int32_t num_of_games,
  int32_t player1_mid, int32_t player2_mid, int32_t player3_mid, const std::string& data_game, int32_t winner)
{
  std::string sql;
  try
  {
    sql = boost::str(boost::format("INSERT INTO  `%s_log`.`game_record` ("
      " `roomid`, `num_of_games`, `player1`, `player2`, `player3`, `data_game`, `winner`,`date`,`club_id`) VALUES "
      "(%d, %d, %d, %d, %d, \"%s\", %d,CURRENT_DATE());")
      % db_prefix_.c_str() % roomid % num_of_games % player1_mid %player2_mid %player3_mid %data_game %winner);

    DLOG(INFO) << "SQL:=" << sql;
  }
  catch (std::exception& e)
  {
    LOG(INFO) << "SetRunFastGameRecord, FAILED, exception:=" << e.what();
    return -1;
  }

  if (databaseconnector_->PExecute(sql.c_str()) == assistx2::error_code::SQL_EXEC_FAILED)
  {

    LOG(INFO) << "SetRunFastGameRecord. FAILED. UID:=" << roomid << " error:=" << databaseconnector_->GetLastError();
    return -1;
  }
  else
  {
    return 0;
  }
}

void DataLayer::AddRoomInfo(const std::int32_t roomid, const std::string& info)
{
    std::stringstream key;
    key << "PRIVATEROOM|" << roomid;

    DLOG(INFO) << "AddUserRoom:" << key.str() << ", " << info;

    if (memcached_agent_->set(key.str(), info, 0) == false)
    {
        LOG(INFO) << ("DataLayer::AddUserRoom. FAILED.");
    }
}

void DataLayer::RemoveRoomInfo(const std::int32_t roomid)
{
    std::stringstream key;
    key << "PRIVATEROOM|" << roomid;

    DLOG(INFO) << "RemoveRoomInfo:" << key.str();

    if (memcached_agent_->remove(key.str()) == false)
    {
        LOG(INFO) << ("DataLayer::RemoveRoomInfo. FAILED.");
    }
}

bool DataLayer::GetRoomInfo(const std::int32_t roomid, std::string& info)
{
    std::stringstream key;
    key << "PRIVATEROOM|" << roomid;

    if (memcached_agent_->get(key.str(), info) == true)
    {
        if (info.empty())
        {
            memcached_agent_->remove(key.str());
            return false;
        }
        else
        {
            DLOG(INFO) << "GetRoomInfo:" << key.str() << ", " << info;
            return true;
        }
    }
    else
    {
        return false;
    }
}

void DataLayer::AddRoomRecordInfo(const std::int32_t roomid, const std::string& info)
{
    std::stringstream key;
    key << "PRIVATERMR|" << roomid;

    DLOG(INFO) << "AddRoomRecordInfo:" << key.str() << ", " << info;

    if (memcached_agent_->set(key.str(), info, 0) == false)
    {
        LOG(INFO) << ("DataLayer::AddRoomRecordInfo. FAILED.");
    }

}

void DataLayer::RemoveRoomRecordInfo(const std::int32_t roomid)
{
    std::stringstream key;
    key << "PRIVATERMR|" << roomid;

    DLOG(INFO) << "RemoveRoomInfo:" << key.str();

    if (memcached_agent_->remove(key.str()) == false)
    {
        LOG(INFO) << ("DataLayer::RemoveRoomRecordInfo. FAILED.");
    }
}

bool DataLayer::GetRoomRecordInfo(const std::int32_t roomid, std::string& info)
{
    std::stringstream key;
    key << "PRIVATERMR|" << roomid;

    if (memcached_agent_->get(key.str(), info) == true)
    {
        if (info.empty())
        {
            memcached_agent_->remove(key.str());
            return false;
        }
        else
        {
            DLOG(INFO) << "GetRoomRecordInfo:" << key.str() << ", " << info;
            return true;
        }
    }
    else
    {
        return false;
    }
}

void DataLayer::AddPlayerStatus(const uid_type mid, const std::int32_t game_session)
{
    std::stringstream key;
    key << "PLAYERSTATUS|" << mid;

    std::stringstream val;
    val << game_session;

    DLOG(INFO) << "AddPlayerStatus:" << key.str() << ", " << game_session;

    std::string value;
    if (memcached_agent_->get(key.str(), value) == true)
    {
        if (memcached_agent_->replace(key.str(), val.str()) == false)
        {
            LOG(INFO) << ("DataLayer::AddPlayerStatus. FAILED.");
        }
    }
    else
    {
        if (memcached_agent_->set(key.str(), val.str()) == false)
        {
            LOG(INFO) << ("DataLayer::AddPlayerStatus. FAILED.");
        }
    }
}

void DataLayer::RemovePlayerStatus(const uid_type mid)
{
    std::stringstream key;
    key << "PLAYERSTATUS|" << mid;

    DLOG(INFO) << "RemovePlayerStatus:" << key.str();

    if (memcached_agent_->remove(key.str()) == false)
    {
        LOG(INFO) << ("DataLayer::RemovePlayerStatus. FAILED.");
    }
}

void DataLayer::UpdateRoomsList(const std::string & type, const std::string & info, const std::int32_t sid)
{
	std::stringstream key;
	key<<"ROOMS1|"<<type<<"|"<<sid;
	DLOG(INFO) << "UpdateRoomsList:" << key.str() << ", " << info;
	if (memcached_agent_->set(key.str(), info) == false)
	{
        LOG(INFO) << ("DataLayer::UpdateRoomsList. FAILED.");
	}
}

void DataLayer::UpdateRoom(std::int32_t roomid, const std::string & info)
{
	std::stringstream key;
	key<<"ROOM1|"<<roomid;
	DLOG(INFO) << "UpdateRoom:" << key.str() << ", " << info;
	if (memcached_agent_->set(key.str(), info) == false)
	{
        LOG(INFO) << ("DataLayer::UpdateRoom. FAILED.");
	}
}

void DataLayer::UpdateCreatorList(const std::string& list)
{
    std::stringstream key;
    key << "CREATORLIST|0";
    DLOG(INFO) << "UpdateCreaterList:" << key.str() << ", " << list;
    if (memcached_agent_->set(key.str(), list,0) == false)
    {
        LOG(INFO) << ("DataLayer::UpdateCreaterList. FAILED.");
    }
}

bool DataLayer::GetCreatorList(std::string& list)
{
    std::stringstream key;
    key << "CREATORLIST|0";

    if (memcached_agent_->get(key.str(), list) == true)
    {
        if (list.empty())
        {
            memcached_agent_->remove(key.str());
            return false;
        }
        else
        {
            DLOG(INFO) << "GetCreatorList:" << key.str() << ", " << list;
            return true;
        }
    }
    else
    {
        return false;
    }
}

void DataLayer::UpdateCreatorRoomList(const uid_type mid, const std::string& list)
{
    std::stringstream key;
    key << "ROOMS|" << mid << "|0";
    DLOG(INFO) << "UpdateCreaterRoomList:" << key.str() << ", " << list;
    if (memcached_agent_->set(key.str(), list, 0) == false)
    {
        LOG(INFO) << ("DataLayer::UpdateCreaterRoomList. FAILED.");
    }
}

bool DataLayer::GetCreatorRoomList(const uid_type mid, std::string& list)
{
    std::stringstream key;
    key << "ROOMS|" << mid << "|0";

    if (memcached_agent_->get(key.str(), list) == true)
    {
        if (list.empty())
        {
            memcached_agent_->remove(key.str());
            return false;
        }
        else
        {
            DLOG(INFO) << "GetCreatorRoomList:" << key.str() << ", " << list;
            return true;
        }
    }
    else
    {
        return false;
    }
}

void DataLayer::DeleteCreatorRoomList(const uid_type mid)
{
    std::stringstream key;
    key << "ROOMS|" << mid << "|0";

    DLOG(INFO) << "DeleteCreatorRoomList:" << key.str() ;
    if (memcached_agent_->remove(key.str()) == false)
    {
        LOG(INFO) << ("DataLayer::DeleteCreatorRoomList. FAILED.");
    }
}

void DataLayer::UpdateRoomData(const uid_type mid, std::int32_t roomid, const std::string& list)
{
    std::stringstream key;
    key << "ROOM|" << mid << "|" << roomid << "|0";
    DLOG(INFO) << "UpdateRoomData:" << key.str() << ", " << list;
    if (memcached_agent_->set(key.str(), list, 0) == false)
    {
        LOG(INFO) << ("DataLayer::UpdateRoomData. FAILED.");
    }
}

bool DataLayer::GetRoomData(const uid_type mid, std::int32_t roomid, std::string& list)
{
    std::stringstream key;
    key << "ROOM|" << mid << "|" << roomid << "|0";

    if (memcached_agent_->get(key.str(), list) == true)
    {
        if (list.empty())
        {
            memcached_agent_->remove(key.str());
            return false;
        }
        else
        {
            DLOG(INFO) << "GetRoomData:" << key.str() << ", " << list;
            return true;
        }
    }
    else
    {
        return false;
    }
}

void DataLayer::DeleteRoomData(const uid_type mid, std::int32_t roomid)
{
    std::stringstream key;
    key << "ROOM|" << mid << "|" << roomid << "|0";

    DLOG(INFO) << "DeleteRoomData:" << key.str();
    if (memcached_agent_->remove(key.str()) == false)
    {
        LOG(INFO) << ("DataLayer::DeleteRoomData. FAILED.");
    }
}

bool DataLayer::GetRopen(const uid_type mid,std::string& data)
{
    std::stringstream key;
    key << "ROPEN|" << mid;

    if (memcached_agent_->get(key.str(), data) == true)
    {
        if (data.empty())
        {
            memcached_agent_->remove(key.str());
            return false;
        }
        else
        {
            DLOG(INFO) << "GetRopen:" << key.str() << ", " << data;
            return true;
        }
    }
    else
    {
        return false;
    }
}

void DataLayer::removeKey(const std::string& key) const
{
    DLOG(INFO) << "removeKey:" << key.c_str();
    if (memcached_agent_->remove(key.c_str()) == false)
    {
        LOG(INFO) << "DataLayer::removeKey. FAILED." << "key = " << key;
    }
}

std::int32_t DataLayer::GetPlayerBaseInfo( uid_type mid, std::string  & json_str, MemberFides & info )
{
	NET_TIME_TRACE_POINT(GetPlayerBaseInfo, assistx2::TimeTracer::SECOND);

	std::stringstream key;
	key << "TMFIELD" << mid;

	if (memcached_agent_->get(key.str(), json_str) == true && json_str.empty() == false)
	{
		//DLOG(INFO) << "mid:=" << mid << ", " << json_str;

		json_spirit::Value json;
		if (json_str.empty() == false && json_spirit::read_string(json_str, json) == true)
		{
			if (json.type() == json_spirit::array_type)
			{
				poker::JsonToMessage(json, &info);
				return 0;
			}
		}
	}

	{
		std::unique_ptr<IQueryResult > result(databaseconnector_->PQuery("SELECT * FROM `%s`.`memberfides%d` "
			" WHERE `mid` = %d;",
            db_prefix_.c_str(),(mid / MEMBER_FIDES_TABLE_HAS_FACTOR), mid));

		if (result.get() == nullptr || result->RowCount() == 0)
		{
			LOG(INFO) << "GetPlayerBaseInfo FAILED READ FROM DB, mid:=" << mid;
			return -1;
		}

		json_spirit::Value array;
		auto descriptor = MemberFides::descriptor();

		poker::DataBaseToJson(result.get(), descriptor, array);

		const auto err = poker::JsonToMessage(array, &info);
		if (err == 0)
		{
			json_str = json_spirit::write_string(array, json_spirit::raw_utf8);

			DLOG(INFO) << "mid:=" << mid << ", " << json_str;
			memcached_agent_->set(key.str(), json_str, 0);
		}

		return err;
	}

	LOG(INFO) << "GetPlayerBaseInfo. FAILED. mid:=" << mid;
	return -1;
}

//std::int32_t DataLayer::GetMemberGameFromDB(uid_type mid, json_spirit::Value & array)
//{
//	NET_TIME_TRACE_POINT(GetMemberGameFromDB, assistx2::TimeTracer::SECOND);

//	std::string sql_buffer;

//	try
//	{
//		sql_buffer = (boost::format("SELECT  * FROM `%1%`.`membergame%2%`  WHERE `mid` = %3% AND `type` = 0;") %
//            db_prefix_.c_str() %(mid  / MEMBER_GAME_TABLE_FACTOR) % mid).str();

//		//DLOG(INFO) << "SQL:=" << sql_buffer;
//	}
//	catch (std::exception & e)
//	{
//		LOG(INFO) << "GetMemberGameFromDB, FAILED, exception:=" << e.what();
//		return -1;
//	}

//	std::unique_ptr<IQueryResult> result(databaseconnector_->PQuery(sql_buffer));
//	if (result.get() == nullptr || result->RowCount() == 0)
//	{
//		LOG(INFO) << "GetMemberGameFromDB, FAILED mid:=" << mid;
//		return -1;
//	}

//	poker::DataBaseToJson(result.get(), MemberGame::descriptor(), array);

//	return 0;
//}

std::int32_t DataLayer::GetGameInfoFromCache(uid_type mid, MemberGame & info)
{
	std::stringstream key;
	//0 为跑的快, 1 为跑胡子
	key << "TMGM" << mid << "|0";
	std::string value;
	if (memcached_agent_->get(key.str(), value) == true && value.empty() == false)
	{
		//DLOG(INFO) << "GetGameInfoFromCache mid:=" << mid << ", value:=" << value;
		json_spirit::Value json;
		if (json_spirit::read_string(value, json) == true)
		{
			poker::JsonToMessage(json, &info);
		}
	}

	return -1;

}

//std::int32_t DataLayer::GetPlayerGameInfo(uid_type mid, MemberGame & info, bool forcedflush /*= false*/)
//{
//	NET_TIME_TRACE_POINT(GetPlayerGameInfo, assistx2::TimeTracer::SECOND / 10);

//	if (forcedflush == false)
//	{
//		const int err = GetGameInfoFromCache(mid, info);
//		if (err == 0)
//		{
//			return err;
//		}
//	}

//	json_spirit::Value value;
//	if (GetMemberGameFromDB(mid, value) != 0)
//	{
//		return -1;
//	}

//	const int err = poker::JsonToMessage(value, &info);

//	DCHECK_EQ(err, 0);
//	if (err == 0)
//	{
//		SyncGameDataToCache(info.mid(), value);
//	}

//	return err;
//}

std::int32_t DataLayer::GetCommonGameInfo(uid_type mid, MemberCommonGame & info, bool forcedflush /*= false*/)
{
	NET_TIME_TRACE_POINT(GetCommonGameInfo, assistx2::TimeTracer::SECOND);

	if (forcedflush == false)
	{
		const int err = GetCommonGameInfoFromCache(mid, info);
		if (err == 0)
		{
			return err;
		}
	}

	json_spirit::Value array;
	if (GetCommonGameInfoFromDB(mid, array) != 0)
	{
		return -1;
	}

	const int err = poker::JsonToMessage(array, &info);

	DCHECK_EQ(err, 0);
	if (err == 0)
	{
		SetCommonGameInfoToCache(info.mid(), array);
	}

	return 0;
}

std::int32_t DataLayer::GetCommonGameInfoFromDB(uid_type mid, json_spirit::Value & array)
{
	NET_TIME_TRACE_POINT(GetCommonGameInfoFromDB, assistx2::TimeTracer::SECOND);

	std::string sql_buffer;

	try
	{
		sql_buffer = (boost::format("SELECT  * FROM `%1%`.`membercommongame%2%`  WHERE `mid` = %3%;") %
            db_prefix_.c_str() %(mid / MEMBER_GAME_TABLE_FACTOR) % mid).str();

		//DLOG(INFO) << "SQL:=" << sql_buffer;
	}
	catch (std::exception & e)
	{
		LOG(INFO) << "GetMemberGameFromDB, FAILED, exception:=" << e.what();
		return -1;
	}

	std::unique_ptr<IQueryResult> result(databaseconnector_->PQuery(sql_buffer));
	if (result.get() == nullptr || result->RowCount() == 0)
	{
		LOG(ERROR) << "GetMemberGameFromDB, FAILED mid:=" << mid;
		return -1;
	}

	poker::DataBaseToJson(result.get(), MemberCommonGame::descriptor(), array);

	return 0;
}

int DataLayer::SyncGameDataToCache(const uid_type mid, const json_spirit::Value & json_value)
{
	std::stringstream key;
	key << "TMGM" << mid << "|0";

	const std::string json_str = json_spirit::write_string(json_value);

	//DLOG(INFO) << json_str;

	if (memcached_agent_->set(key.str(), json_str, 0) == true)
	{
		return -1;
	}
	else
	{
		LOG(INFO) << "GetPlayerGameInfo. WRITE TO CACHED FAILED mid:=" << mid;
		return -1;
	}
}

std::int32_t DataLayer::GetCommonGameInfoFromCache(uid_type mid, MemberCommonGame & info)
{
	std::stringstream key;
	key << "TMGMCOM" << mid;
	std::string value;
	if (memcached_agent_->get(key.str(), value) == true && value.empty() == false)
	{
		DLOG(INFO) << "GetGameInfoFromCache mid:=" << mid << ", value:=" << value;
		json_spirit::Value json;
		if (json_spirit::read_string(value, json) == true)
		{
			poker::JsonToMessage(json, &info);
		}
	}

	return -1;
}

std::int32_t DataLayer::SetCommonGameInfoToCache(const uid_type mid, const json_spirit::Value & json_value)
{
	std::stringstream key;
	key << "TMGMCOM" << mid;

	const std::string json_str = json_spirit::write_string(json_value);

	//DLOG(INFO) << "SetCommonGameInfoToCache:=" << json_str << ", mid:=" << mid;
	if (memcached_agent_->set(key.str(), json_str, 0) == true)
	{
		return -1;
	}
	else
	{
		LOG(INFO) << "SetCommonGameInfoToCache. WRITE TO CACHED FAILED mid:=" << mid;
		return -1;
	}
}

std::int32_t DataLayer::GetUserPropsFromCache(const uid_type mid, Props_type & props)
{
	std::stringstream key;
	key<<"PROPS"<<mid;
	std::string value;
	if (memcached_agent_->get(key.str(), value) == false || value.empty() == true)
	{
		DLOG(INFO) << " GetUserPropsFromCache   memcached_agent_->get(key.str(), value) == false || value.empty() == true)   ";
		return -1;
	}

	bool must_flush_cache = false;

	try
	{
		std::set<std::int32_t > pids;

		json_spirit::Value json_props;
		json_spirit::read_string_or_throw(value, json_props);

		const json_spirit::Array & items = json_props.get_array();

		for (json_spirit::Array::const_iterator it = items.begin(); it != items.end(); ++it)
		{
			const json_spirit::Array & item = it->get_array();

			Prop_type prop;
			prop.pid	= assistx2::ToInt(item.at(0) );				//item.push_back(it->pid);

			if (pids.find(prop.pid) == pids.end())
			{
				pids.insert(prop.pid);
			}
			else
			{
                LOG(INFO) << "DataLayer::GetUserPropsFromCache, BUG mid:=" << mid;
				return -1;
			}

			prop.pcate		= assistx2::ToInt(item.at(1) );				//item.push_back(prop.pcate);
			prop.pframe		= assistx2::ToInt(item.at(2) );
			prop.status		= assistx2::ToInt(item.at(3) );
			prop.sptime		= assistx2::ToInt(item.at(4) );
			prop.sltime		= assistx2::ToInt(item.at(5) );
			prop.sendmid	= assistx2::ToInt(item.at(6) );

			if (prop.sltime == 0 || prop.sltime > time(nullptr))
			{
				props.push_back(prop);
			}
			else
			{
				must_flush_cache = true;
			}
		}

		if (must_flush_cache == true)
		{
			memcached_agent_->remove(key.str() );
		}

		return 0;
	}
	catch(...)
	{
		LOG(INFO)<<"CACHED FORMAT INVAILD. mid:="<<mid<<", "<<value;
		return -1;
	}
}

std::int32_t DataLayer::GetUserProps( const uid_type mid, Props_type & props )
{
	if (GetUserPropsFromCache(mid, props) == 0)
	{
		return 0;
	}
	else
	{
		return GetUserPropsFromDB(mid, props);
	}
}

std::int32_t DataLayer::GetUserPropsFromDB( const uid_type uid, Props_type & props)
{
	std::unique_ptr<IQueryResult> result(databaseconnector_->PQuery("SELECT * FROM `poker_props%d`.`props%d` WHERE `mid` = %d;  ",	
		 (uid / PROPS_DATABASE_HAS_FACTOR), (uid / PROPS_TABLE_HAS_FACTOR), uid ) );
	if (result.get() == nullptr)
	{
		LOG(INFO) << "ERR:=" << databaseconnector_->GetLastError() << ", mid:=" << uid;
		return -1;
	}

	for (unsigned long i = 0; i < result->RowCount(); ++i)
	{
		Prop_type prop;
		prop.pid			= result->GetItemLong(i, "pid");
		prop.pcate		= result->GetItemLong(i, "pcate");
		prop.pframe	= result->GetItemLong(i, "pframe");
		prop.status		= result->GetItemLong(i, "status");
		prop.sptime		= result->GetItemLong(i, "sptime");
		prop.sltime		= result->GetItemLong(i, "sltime");
		prop.sendmid	= result->GetItemLong(i, "sendmid");

		props.push_back(prop);
	}

	json_spirit::Value json_props;
	ToJson(props, json_props);

	std::stringstream key;
	key<<"PROPS"<<uid;
	if (memcached_agent_->set(key.str(), json_spirit::write_string(json_props).c_str()) == false)
	{
        LOG(INFO) << ("GetUserProps . WRITE TO CACHED FAILED.");
	}

	return 0;
}

std::int32_t DataLayer::GetFriends( const uid_type mid, std::set<uid_type> & friends )
{
	cpp_redis::SetRequest<cpp_redis::ResultExtractor> cmd(*redisconnector_);

	std::stringstream key;
	key<<"TMRNB"<<mid;

	try
	{
		cmd.smembers(key.str());
	}
	catch (...)
	{
        LOG(INFO) << "DataLayer::GetFriends, FAILED mid:=" << mid;
		return -1;
	}

	const cpp_redis::Result * result = cmd().getResults();
	if (result->resultCode_ == 0 && result->results_.empty() == false)
	{
		for (cpp_redis::Result::RESULTS_TYPE::const_iterator it =result->results_.begin(); it != result->results_.end(); ++it)
		{
			std::string str;
			friends.insert(std::stoi(assistx2::toString<cpp_redis::BYTES>(*it, str)));
		}

		return 0;
	}
	else
	{
		return -1;
	}
}

std::int32_t DataLayer::GetLevelCfg( LevelItems_type & cfg )
{
	std::unique_ptr<IQueryResult> result(databaseconnector_->PQuery("SELECT  `level`,  `mingold` FROM `%s`.`systemlevelcfg`; ", cfg_database_.c_str()));
	if (result.get() == nullptr)
	{
		LOG(ERROR)<<("DataLayer::GetLevelCfg. FAILED.");
		return -1;
	}

	for (unsigned long i = 0; i < result->RowCount(); ++i)
	{
		LevelItem item;
		item.level = result->GetItemLong(i, "level");
		item.price = result->GetItemLong(i, "mingold");

		cfg.push_back(item);
	}

	std::sort(cfg.begin(), cfg.end());

	return 0;
}

std::int32_t DataLayer::Pay( const uid_type mid, const chips_type incr, chips_type &  amount, chips_type& real_delta, bool bForce/* = false*/)
{
	NET_TIME_TRACE_POINT(Pay, assistx2::TimeTracer::SECOND / 10);

	std::stringstream key;
	key << "TMGMCOM" << mid;

	memcached_agent_->remove(key.str() );

	std::unique_ptr<IQueryResult> result(databaseconnector_->PQuery("CALL `%s`.`pay`(%d, %d, %s, @err, @old_gold, @new_gold); "
		" SELECT @err, @old_gold, @new_gold;", db_prefix_.c_str(),mid, -incr, (bForce ? "TRUE" : "FALSE") ) );
	if (result.get() == nullptr || result->RowCount() == 0)
	{
		LOG(ERROR) << "Pay, FAILED mid:=" << mid << ", " << databaseconnector_->GetLastError();
		return -1;
	}
	
	auto err = result->GetItemLong(0, "@err");
	amount = result->GetItemLong(0, "@new_gold");
	real_delta = result->GetItemLong(0, "@old_gold") - amount;

	DLOG(INFO) << "Pay mid:=" << mid << ", incr:=" << incr << ", new_gold:=" << amount << ", real_delta:=" << real_delta
		<< ", bForce:=" << (bForce ? "TRUE" : "FALSE") << ", err:=" << err
		<< ", old_gold:=" << result->GetItemLong(0, "@old_gold");

	return err;
}

std::int32_t DataLayer::GetRank( const uid_type mid, std::int32_t & win_rank, std::int32_t & riches_rank )
{
	std::stringstream key;
	key<<"RANK"<<mid;

	std::string str;
	if (memcached_agent_->get(key.str(), str) == true)
	{
		json_spirit::Value json_value;
		if (str.empty() == false && json_spirit::read_string(str, json_value) == true)
		{
			try
			{
				win_rank = assistx2::ToInt(json_value.get_array().at(0) );
				riches_rank = assistx2::ToInt(json_value.get_array().at(1) );

				return 0;
			}
			catch (std::exception & /*e*/)
			{
                LOG(INFO) << "DataLayer::GetRank. CATCH std::exception, mid:=" << mid;
			}
		}
	}

	win_rank = 0;
	riches_rank = 0;

	return -1;
}

std::int32_t DataLayer::UpdateAwardPool( chips_type award , chips_type & award_pool)
{
	std::stringstream key;
	key << "AWARD_POOL1|";

	DLOG(INFO) << "UpdateAwardPool, key:=" << key.str() << ", award:=" << award;

	//RedisIncrInt64
	std::int32_t incr_award = award;
	std::int32_t incr_after_awardpool = award_pool;

	int error = RedisIncr(key.str(), incr_award, incr_after_awardpool);
	award_pool = incr_after_awardpool;

	return error;
}

void DataLayer::GetAward( std::int32_t thousand, chips_type & award, chips_type & award_pool, chips_type & old_award_pool)
{
}


std::int32_t DataLayer::GetPokerFriends( const uid_type mid, std::set<uid_type> & friends )
{
	cpp_redis::SetRequest<cpp_redis::ResultExtractor> cmd(*redisconnector_);

	std::stringstream key;
	key<<"C_FRIENDS"<<mid;

	try
	{
		cmd.smembers(key.str());
	}
	catch (...)
	{
		LOG(ERROR)<<"DataLayer::GetPokerFriends, FAILED mid:="<<mid;
		return -1;
	}

	const cpp_redis::Result * result = cmd().getResults();
	if (result->resultCode_ == 0 && result->results_.empty() == false)
	{
		for (cpp_redis::Result::RESULTS_TYPE::const_iterator it =result->results_.begin(); it != result->results_.end(); ++it)
		{
			std::string str;
			friends.insert(assistx2::atoi_s(assistx2::toString<cpp_redis::BYTES>(*it, str)));
		}

		return 0;
	}
	else
	{
		return -1;
	}
}

std::int32_t DataLayer::DeletePokerFriend( const uid_type mid, const uid_type fmid )
{
	try
	{
		cpp_redis::SetRequest<cpp_redis::ResultExtractor> cmd(*redisconnector_);

		std::stringstream key;
		key<<"C_FRIENDS"<<mid;

		cpp_redis::PAIR p = cpp_redis::makePair(key.str(),  fmid);
		cmd.srem(p);

		const cpp_redis::Result * result = cmd().getResults();
		if (result == nullptr ||  result->intResults_.empty() || result->intResults_[0] != 1)
		{
			return -1;
		}

		key.str("");
		key<<"C_FRIENDS"<<fmid;

		p = cpp_redis::makePair(key.str(),  mid);
		cmd.srem(p);

		result = cmd().getResults();
		if (result == nullptr || result->intResults_.empty() || result->intResults_[0] != 1)
		{
			return -1;
		}
	}
	catch (...)
	{
		LOG(ERROR)<<"DataLayer::DeletePokerFriend, FAILED from:="<<mid<<", to:="<<fmid;
		return -1;
	}

	return 0;
}

std::int32_t DataLayer::AddPokerFriend( const uid_type mid, const uid_type fmid )
{
	try
	{
		cpp_redis::SetRequest<cpp_redis::ResultExtractor> cmd(*redisconnector_);

		std::stringstream key;
		key<<"C_FRIENDS"<<mid;

		cpp_redis::PAIR p = cpp_redis::makePair(key.str(),  fmid);
		cmd.sadd(p);

		const cpp_redis::Result * result = cmd().getResults();
		if (result == nullptr ||  result->intResults_.empty() || result->intResults_[0] != 1)
		{
			return -1;
		}

		key.str("");
		key<<"C_FRIENDS"<<fmid;

		p = cpp_redis::makePair(key.str(),  mid);
		cmd.sadd(p);

		result = cmd().getResults();
		if (result == nullptr || result->intResults_.empty() || result->intResults_[0] != 1)
		{
			return -1;
		}
	}
	catch (...)
	{
		LOG(ERROR)<<"DataLayer::AddFriend, FAILED from:="<<mid<<", to:="<<fmid;
		return -1;
	}

	return 0;
}

void DataLayer::GetDayData( const uid_type mid, std::string & data )
{
	std::stringstream key;
	key<<"LEARNER"<<mid;

	memcached_agent_->get(key.str(), data);
}

void DataLayer::LockGold( const uid_type mid )
{
	const std::string keygameservers("GAME_SERVERS");
	std::string strgameservers;
	memcached_agent_->get(keygameservers, strgameservers);

	std::stringstream key;
	key<<"C_LOCK"<<mid;

	memcached_agent_->set(key.str(), strgameservers);
}

void DataLayer::UnlockGold( const uid_type mid )
{
	std::stringstream key;
	key << "C_LOCK" << mid;

	memcached_agent_->remove(key.str());
}

std::int32_t DataLayer::GetRoomGiftCfg( RoomGiftCfg & cfgs )
{
	std::unique_ptr<IQueryResult >  result(databaseconnector_->PQuery("SELECT  `stime`,  `etime`,  `content`, `id`, `ctime` FROM `%s`.systemplaysendgiftcfg "
		" LEFT JOIN `%s`.`systemroomcfg` ON systemplaysendgiftcfg.`rid` = `systemroomcfg`.`id` WHERE `status` = 1;", 
		cfg_database_.c_str(), cfg_database_.c_str()) );
	if (result.get() == nullptr)
	{
		LOG(ERROR)<<"ERR:="<<databaseconnector_->GetLastError();
		return -1;
	}

	for(std::size_t i = 0; i < result->RowCount(); ++i)
	{
		GiftCfg cfg;

		std::string str	= result->GetItemString(i, "content");

		std::vector<std::string > gift_list;
		assistx2::SplitString(str, ",", gift_list);

		for (std::vector<std::string >::iterator it = gift_list.begin(); it != gift_list.end(); ++it)
		{
			std::vector<std::string > item;
			assistx2::SplitString(*it, "-", item);

			if (item.size() == 4)
			{
				GiftProps gift;
				gift.prop_.pcate			= assistx2::atoi_s(item[0]);
				gift.prop_.pframe		= assistx2::atoi_s(item[1]);
				gift.num_						= assistx2::atoi_s(item[2]);
				gift.probability_			= assistx2::atoi_s(item[3]);
				gift.expire_					= result->GetItemLong(i, "ctime");

				cfg.props_.push_back(gift);
			}
		}

		cfg.begin_time_		= result->GetItemLong(i, "stime");
		cfg.end_time_			= result->GetItemLong(i, "etime");

		cfg.room_type_id_ = result->GetItemLong(i, "id");

		cfgs.push_back(cfg);
	}

	return 0;
}

std::int32_t DataLayer::GetLoginIP( uid_type mid, std::string & login_ip )
{
	std::stringstream key;
	key<<"IP"<<mid;
	return memcached_agent_->get(key.str(), login_ip) == true ? 0 : -1;
}

void DataLayer::UpdateDayData( const uid_type mid, const std::string & data )
{
	std::stringstream key;
	key<<"LEARNER"<<mid;

	memcached_agent_->set(key.str(), data, 3 * 24 * 60 * 60 );
}


bool Check(boost::shared_ptr< assistx2::Stream > & result, assistx2::Streams & steams)
{
	if (steams.empty() == true)
	{
		return false;
	}

	result = boost::shared_ptr< assistx2::Stream >(new assistx2::Stream(steams.front()));

	return true;
}

std::int32_t DataLayer::IncrPoints(std::int32_t mid, const std::int32_t incr, std::int32_t & value)
{
	std::stringstream ss;
	ss << "C_RPM" << mid;

	const auto err = RedisIncr(ss.str(), incr, value);

	LOG(INFO) << "IncrPoint mid:=" << mid << ", value:=" << value << ", err:=" << err;

	return err;
}

std::int32_t DataLayer::GetPoints(std::int32_t mid, std::string & value)
{
	std::stringstream ss;
	ss << "C_RPM" << mid;

	const auto err = DataLayer::getInstance()->RedisGet(ss.str(), value);

	DLOG(INFO) << "GetPoints mid:=" << mid << ", value:=" << value << ", err:=" << err;

	return err;
}

std::int32_t DataLayer::StartGoldStat(chips_type /*carry_gold*/)
{
	return -1;
}

std::int32_t DataLayer::RedisSet(const std::string &  key, const std::string & value)
{
    DLOG(INFO) << "DataLayer::RedisSet, key:=" << key;

    NET_TIME_TRACE_POINT(DataLayer::RedisSet, assistx2::TimeTracer::SECOND);

    cpp_redis::StringRequest<cpp_redis::ResultExtractor> cmd(*redisconnector_);

    try
    {
        cpp_redis::PAIRS pair(1, cpp_redis::makePair(key, value));
        cmd.set(pair);

        return 0;
    }
    catch (...)
    {
        LOG(INFO) << "DataLayer::RedisSet, FAILED key:=" << key;
        return Texas::error_code::ERROR_WRITE_REDIS_ERROR;
    }
}

std::int32_t DataLayer::RedisGetKeys(std::vector<std::string > &  keys, std::vector<std::string > & value)
{
    DLOG(INFO) << "DataLayer::RedisGet, key:=" << keys;

    NET_TIME_TRACE_POINT(DataLayer::RedisGetKeys, assistx2::TimeTracer::SECOND);

    cpp_redis::StringRequest<cpp_redis::ResultExtractor> cmd(*redisconnector_);

    try
    {
        cmd.get(keys);
    }
    catch (...)
    {
        LOG(ERROR) << "DataLayer::RedisGet, FAILED key:=" << keys;
        return Texas::error_code::ERROR_READ_REDIS_ERROR;
    }

    const cpp_redis::Result * result = cmd().getResults();
    if (result->resultCode_ == 0 && result->results_.empty() == false)
    {
        for (auto & item : result->results_)
        {
            value.push_back(std::string(""));
            if (item != cpp_redis::NIL())
            {
                assistx2::toString<cpp_redis::BYTES>(item, value.back());
            }

            DLOG(INFO) << value.back();
        }

        return 0;
    }
    else
    {
        LOG(ERROR) << "DataLayer::RedisGet, FAILED key:=" << keys << ", resultCode_:=" << result->resultCode_
            << ", " << result->err_;
        return Texas::error_code::ERROR_READ_REDIS_ERROR;
    }
}

std::int32_t DataLayer::RedisGet(const std::string &  key, std::string & value)
{
    DLOG(INFO) << "DataLayer::RedisGet, key:=" << key;

    NET_TIME_TRACE_POINT(DataLayer::RedisGet, assistx2::TimeTracer::SECOND);

    cpp_redis::StringRequest<cpp_redis::ResultExtractor> cmd(*redisconnector_);

    try
    {
        cpp_redis::KEYS keys;
        keys.push_back(key);
        cmd.get(keys);
    }
    catch (...)
    {
        LOG(ERROR) << "DataLayer::RedisGet, FAILED key:=" << key;
        return Texas::error_code::ERROR_READ_REDIS_ERROR;
    }

    const cpp_redis::Result * result = cmd().getResults();
    if (result->resultCode_ == 0 && result->results_.empty() == false)
    {
        if (result->results_[0] == cpp_redis::NIL())
        {
            return 0;
        }

        assistx2::toString<cpp_redis::BYTES>(result->results_[0], value);

        return 0;
    }
    else
    {
        LOG(ERROR) << "DataLayer::RedisGet, FAILED key:=" << key << ", resultCode_:=" << result->resultCode_
            << ", " << result->err_;
        return Texas::error_code::ERROR_READ_REDIS_ERROR;
    }
}

std::int32_t DataLayer::RedisDel(const std::string &  key)
{
    DLOG(INFO) << "DataLayer::RedisDel, key:=" << key;

    NET_TIME_TRACE_POINT(DataLayer::RedisIncr, assistx2::TimeTracer::SECOND);

    cpp_redis::GenericRequest<cpp_redis::ResultExtractor> cmd(*redisconnector_);

    try
    {
        cpp_redis::KEYS keys;
        keys.push_back(key);
        cmd.del(keys);

        return 0;
    }
    catch (...)
    {
        LOG(ERROR) << "DataLayer::RedisDel, FAILED key:=" << key;
        return Texas::error_code::ERROR_WRITE_REDIS_ERROR;
    }
}

std::int32_t DataLayer::RedisIncr(const std::string & key, const std::int32_t incr, std::int32_t & amount)
{
    DLOG(INFO) << "DataLayerImpl::RedisIncr, key:=" << key << ", incr:=" << incr;

    NET_TIME_TRACE_POINT(DataLayer::RedisIncr, assistx2::TimeTracer::SECOND);

    cpp_redis::StringRequest<cpp_redis::ResultExtractor> cmd(*redisconnector_);

    try
    {
        cmd.incr(key, incr);
    }
    catch (...)
    {
        LOG(ERROR) << "DataLayerImpl::RedisIncr, FAILED key:=" << key << ", incr:=" << incr;
        return Texas::error_code::ERROR_WRITE_REDIS_ERROR;
    }

    const cpp_redis::Result * result = cmd().getResults();
    if (result != nullptr &&  result->resultCode_ == 2 && result->intResults_.empty() == false)
    {
        amount = result->intResults_[0];

        DLOG(INFO) << "DataLayerImpl::RedisIncr, key:=" << key << ", incr:=" << incr << ", amount:=" << amount;

        return Texas::error_code::ERROR_SUCCEED;
    }
    else
    {
        LOG(ERROR) << "DataLayerImpl::RedisIncr. FAILD, key:=" << key << ", incr:=" << incr;
        return Texas::error_code::ERROR_WRITE_REDIS_ERROR;
    }
}

void DataLayer::set_player_gps(const uid_type mid, const std::string& data)
{
    std::stringstream key;
    key << "PLAYERGPS|" << mid << "|0";

    DLOG(INFO) << "set_player_gps:" << key.str() << ", " << data;

    std::string value;
    if (memcached_agent_->get(key.str(), value) == true)
    {
        if (memcached_agent_->replace(key.str(), data) == false)
        {
            LOG(INFO) << ("DataLayer::set_player_gps. FAILED.");
        }
    }
    else
    {
        if (memcached_agent_->set(key.str(), data) == false)
        {
            LOG(INFO) << ("DataLayer::set_player_gps. FAILED.");
        }
    }
}

std::string DataLayer::player_gps(const uid_type mid)
{
    std::stringstream key;
    key << "PLAYERGPS|" << mid << "|0";

    std::string data;
    if (memcached_agent_->get(key.str(), data) == true)
    {
        if (data.empty())
        {
            memcached_agent_->remove(key.str());
            return data;
        }
        else
        {
            DLOG(INFO) << "player_ip_addr:" << key.str() << ", " << data;
            return data;
        }
    }

    return data;
}
