#include "ConfigMgr.h"

#include <ctime>

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#include <glog/stl_logging.h>

#include <gflags/gflags.h>

#include <assistx2/string_wrapper.h>
#include <assistx2/configure.h>
#include <assistx2/time_helper.h>

#include <boost/date_time/local_time_adjustor.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>

#include <tinyxml.h>
#include <tinystr.h>
#include <time.h>

#include "DataLayer.h"

DECLARE_string(cfg);

std::ostream & operator <<(std::ostream & stream, const PointLevel & l)
{
    return stream << "points_:=" << l.points_ << ", award_:=" << l.award_ << " ";
}

std::ostream & operator <<(std::ostream & stream, const boost::posix_time::ptime & pt)
{
   // auto  ld = boost::date_time::c_local_adjustor<boost::posix_time::ptime>::utc_to_local(pt);

    return stream << boost::posix_time::to_iso_extended_string(pt);
}


ConfigMgr::ConfigMgr(void)
{

}

ConfigMgr::~ConfigMgr(void)
{
	
}

boost::int32_t getAttribute(TiXmlElement * p, const char * pszName)
{
	if (p == nullptr)
	{
		return 0;
	}
	else
	{
		const char * pStr = p->Attribute(pszName);
		return atoi(pStr == nullptr ? "0" : pStr);
	}
}


std::int32_t ConfigMgr::LoadLocalCfg()
{
    try
    {
        LoadAppCfg();

        LoadGameCfg();

        LoadFreeDate();
    }
    catch (std::exception & e)
    {
        DLOG(INFO) << e.what();
        return -1;
    }

    return 0;
}

void ConfigMgr::LoadAppCfg()
{
    DCHECK(app_cfg_ == nullptr);

    app_cfg_ = new Configure;
    if (app_cfg_->Load(FLAGS_cfg) != 0)
    {
        LOG(ERROR) << ("ConfigMgr::update. FAILED READ config.xml");
        throw std::runtime_error("ConfigMgr::LoadAppCfg, assert(app_cfg_->Load(FLAGS_cfg) == false) ");
    }

    app_cfg_->getConfig("APP", "SID", sid_);

    app_cfg_->getConfig("APP", "PID", pid_file_path_);

    app_cfg_->getConfig("APP", "VERSION", server_version_);

	ReadDBCfg();
}

void ConfigMgr::LoadGameCfg()
{
    std::string game_cfg_path;
    app_cfg_->getConfig("cfg", "path", game_cfg_path);

    DCHECK(game_cfg_ == nullptr);
    game_cfg_ = new Configure;
    if (game_cfg_ == nullptr)
    {
        LOG(ERROR) << "ASSERT(game_reader_ == nullptr).";
        throw std::runtime_error("assert (game_cfg_ == nullptr)");
    }

    if (game_cfg_->Load(game_cfg_path) != 0)
    {
        LOG(ERROR) << "ASSERT(game_cfg_path == nullptr). game_cfg_path:=" << game_cfg_path;
        throw std::runtime_error("assert (game_cfg_->Load(game_cfg_path) != 0)");
    }

    ReadRunFastRoomsCfg();

    //ReadRoundPoint();

    //ReadRoomsCfg();

	//ReadSystemRedPacketCfg();

	//ReadAwardItemsCfg();

	//ReadRoundAward();

	//ReadMobileRoundPoint();
}

void ConfigMgr::ReadRunFastRoomsCfg()
{
  auto rooms = game_cfg_->GetElement("systemroomcfg");
  if (rooms == nullptr)
  {
    throw std::invalid_argument("ASSERT(rooms == nullptr)");
  }

  runfastroomscfg_type cfg;

  for (auto item = rooms->FirstChildElement(); item != nullptr; item = item->NextSiblingElement())
  {
    runfastroomcfg roomcfg;

    roomcfg.begin = atoi(item->Attribute("begin"));
    roomcfg.end = atoi(item->Attribute("end"));
    roomcfg.ju = atoi(item->Attribute("ju"));
    roomcfg.cost = atoi(item->Attribute("cost"));
    roomcfg.type = item->Attribute("type");
    roomcfg.name = item->Attribute("name");
    roomcfg.taxation = atoll(item->Attribute("taxation"));
  
    roomcfg.bettime = atoll(item->Attribute("bettime"));
    roomcfg.taxes_mode = atoi(item->Attribute("taxes_mode"));
    roomcfg.sb = atoi(item->Attribute("sb"));
    roomcfg.min = atol(item->Attribute("min"));
    roomcfg.max = atol(item->Attribute("max"));

    cfg.push_back(roomcfg);
  }

  if (cfg.empty() == false)
  {
    rfroomscfg_.swap(cfg);

    LOG(ERROR) << "roomscfg_:=" << rfroomscfg_.size();
  }
  else
  {
    throw std::invalid_argument("ASSERT(cfg.empty() == false)");
  }
}

void ConfigMgr::ReadRoomsCfg()
{
    auto rooms = game_cfg_->GetElement("systemroomcfg");
    if (rooms == nullptr)
    {
        throw std::invalid_argument("ASSERT(rooms == nullptr)");
    }

    roomscfg_type cfg;

    for (auto item = rooms->FirstChildElement(); item != nullptr; item = item->NextSiblingElement())
    {
        roomlevelcfg_type roomcfg;

        roomcfg.sb = atoll(item->Attribute("sb"));
        roomcfg.begin = atoi(item->Attribute("begin"));
        roomcfg.end = atoi(item->Attribute("end"));
        roomcfg.maxchips = atoll(item->Attribute("max"));
        roomcfg.minchips = atoll(item->Attribute("min")) ;
        roomcfg.maxplayer = atoll(item->Attribute("maxplayer"));
        roomcfg.bettime = atoll(item->Attribute("bettime")) ;
        roomcfg.taxation = atoll(item->Attribute("taxation")) ;

        roomcfg.type = item->Attribute("type");

        roomcfg.royalflush = atoi(item->Attribute("royalflush"));
        CHECK(roomcfg.royalflush >= 0 && roomcfg.royalflush < 1000);

        roomcfg.straightflush = atoi(item->Attribute("straightflush"));
        CHECK(roomcfg.straightflush >= 0 && roomcfg.straightflush < 1000);

        roomcfg.fourofakind = atoi(item->Attribute("fourofakind"));
        CHECK(roomcfg.fourofakind >= 0 && roomcfg.fourofakind < 1000);

        roomcfg.robot_mode = atoi(item->Attribute("robot_mode"));
        roomcfg.seat = atoll(item->Attribute("seat")) ;
        roomcfg.handhog = atoll(item->Attribute("handhog")) ;
        roomcfg.show = atoll(item->Attribute("showtime"));
        roomcfg.type_id = atoi(item->Attribute("id"));
        roomcfg.taxes_mode = atoi(item->Attribute("taxes_mode")) == 0 ? FIXED_TAXES_MODE : RATIO_TAXES_MODE;

        CHECK(roomcfg.taxation >= 0);
        //CHECK(roomcfg.taxes_mode != FIXED_TAXES_MODE || roomcfg.taxation < roomcfg.sb);
        CHECK(roomcfg.taxes_mode != RATIO_TAXES_MODE || roomcfg.taxation < 1000);

        roomcfg.max_robot_amount = atoll(item->Attribute("robot_amount"));

		// min_bet="5000" max_bet="25000"
		roomcfg.min_bet = atoll(item->Attribute("min_bet"));
		roomcfg.max_bet = atoll(item->Attribute("max_bet"));

// #ifdef NDEBUG
// 		//?¡ä???¨¤¡Á¡¥¨¦???o¨®?¨´¨¨£¤¦Ì?
// 		if (roomcfg.type.front() == 'K' || roomcfg.type.front() == 'Z')
// 		{
// 			continue;
// 		}
// #endif

        cfg.push_back(roomcfg);
    }

    if (cfg.empty() == false)
    {
        roomscfg_.swap(cfg);

        std::sort(roomscfg_.begin(), roomscfg_.end(), [](const roomlevelcfg_type & l, const roomlevelcfg_type & r)->bool
        {
            if (l.type[0] == r.type[0])
            {
                return l.maxchips > r.maxchips;
            }
            else
            {
                return l.type[0] > r.type[0];
            }
        });

        LOG(ERROR) << "roomscfg_:=" << roomscfg_.size();
    }
    else
    {
        throw std::invalid_argument("ASSERT(cfg.empty() == false)");
    }
}

void ConfigMgr::ReadDBCfg(  )
{
    if (dbcfg_.host.empty())
	{
		TiXmlElement * pGameItems = app_cfg_->GetElement("DB");
		if (pGameItems != nullptr)
		{
			dbcfg_.host = pGameItems->Attribute("host");
			dbcfg_.port = pGameItems->Attribute("port");
			dbcfg_.passwd = pGameItems->Attribute("passwd");
			dbcfg_.db = pGameItems->Attribute("db");
			dbcfg_.user = pGameItems->Attribute("user");
		}
	}

    LOG(INFO) << "HOST:=" << dbcfg_.host << ", PORT:=" << dbcfg_.port << ", USER:=" << dbcfg_.user
        << ", PW:=" << dbcfg_.passwd << ", DB:=" << dbcfg_.db;
}

std::int32_t ConfigMgr::Update()
{
	if (app_cfg_ == nullptr)
	{
		LOG(ERROR) << "ASSERT(app_reader_ == nullptr). ";
		return -1;
	}

	try
	{
		std::string game_cfg_path;
		app_cfg_->getConfig("cfg", "path", game_cfg_path);

		if (game_cfg_ != nullptr)
		{
			delete game_cfg_;
			game_cfg_ = nullptr;
		}

		game_cfg_ = new Configure;
		if (game_cfg_ == nullptr)
		{
			LOG(ERROR) << "ASSERT(game_reader_ == nullptr).";
			return -1;
		}

		if (game_cfg_->Load(game_cfg_path) != 0)
		{
			LOG(ERROR) << "ASSERT(game_cfg_path == nullptr). game_cfg_path:=" << game_cfg_path;
			return -1;
		}

		//ReadRoundAward();

		//ReadRoundPoint();

		//ReadMobileRoundPoint();

		//ReadSystemRedPacketCfg();

		//for (auto & item : listeners_)
		//{
			//item();
		//}

		return 0;
	}
	catch (std::exception & e)
	{
		LOG(ERROR) << "exception:=" << e.what();
		return -1;
	}
}

std::int32_t ConfigMgr::ReadRoundPoint()
{
    round_point_cfg_.room_groups_.clear();
    round_point_cfg_.levels_.clear();

    auto node = game_cfg_->GetElement("paijuchenhao");
    if (node == nullptr)
    {
        LOG(ERROR) << ("ConfigMgr::ReadRoundPoint. ASSERT(node == nullptr)");
        return -1;
    }

    round_point_cfg_.id_ = atoi(node->Attribute("id"));
    round_point_cfg_.max_ = atoi(node->Attribute("max"));
    round_point_cfg_.decay_ = atoi(node->Attribute("reduce"));

    {
        std::string str = node->Attribute("room");

        std::vector<std::string > ss;
        assistx2::SplitString(str, ",", ss);

        for (auto it : ss)
        {
            std::vector<std::string > cc;
            assistx2::SplitString(it, "-", cc);

            const auto tid = atoi(cc.front().c_str());
            const auto incr = atoi(cc.back().c_str());
            CHECK_GT(incr, 0);

            round_point_cfg_.room_groups_.insert(std::make_pair(tid, incr));
        }
    }

    LOG(INFO) << "ReadRoundPoint max:=" << round_point_cfg_.max_ << ", id:=" << round_point_cfg_.id_ << ", decay_:=" << round_point_cfg_.decay_
        << ", type_:=" << round_point_cfg_.login_type_ << ", rooms:=" << round_point_cfg_.room_groups_;

    {
        round_point_cfg_.begin_time_ = boost::posix_time::from_time_t(atoll(node->Attribute("stime")));
        round_point_cfg_.begin_time_ = boost::date_time::c_local_adjustor<boost::posix_time::ptime>::utc_to_local(round_point_cfg_.begin_time_);

        round_point_cfg_.end_time_ = boost::posix_time::from_time_t(atoll(node->Attribute("etime")));
        round_point_cfg_.end_time_ = boost::date_time::c_local_adjustor<boost::posix_time::ptime>::utc_to_local(round_point_cfg_.end_time_);

        round_point_cfg_.closed_time_ = boost::posix_time::from_time_t(atoll(node->Attribute("ctime")));
        round_point_cfg_.closed_time_ = boost::date_time::c_local_adjustor<boost::posix_time::ptime>::utc_to_local(round_point_cfg_.closed_time_);

        DLOG(INFO) << "stime:=" << atoll(node->Attribute("stime")) << ", etime:=" << atoll(node->Attribute("etime"))
            << ", ctime:=" << round_point_cfg_.closed_time_;

        CHECK_GT(round_point_cfg_.end_time_, round_point_cfg_.begin_time_);
        CHECK_GE(round_point_cfg_.closed_time_, round_point_cfg_.end_time_);
        if (round_point_cfg_.begin_time_ > round_point_cfg_.end_time_)
        {
            LOG(ERROR) << ("ASSERT(cfg.begin_time_ > cfg.end_time_).") << round_point_cfg_.begin_time_ << ", " << round_point_cfg_.end_time_;
            return -1;
        }
    }

        {
            std::string time = node->Attribute("time");
            std::vector<std::string > ss;
            assistx2::SplitString(time, "-", ss);

            if (ss.size() != 2U)
            {
                LOG(ERROR) << ("PARSER time FAILED. time:=") << time;
                return -1;
            }

            round_point_cfg_.begin_each_day_ = assistx2::FromStringToSecond(ss.at(0));
            round_point_cfg_.end_each_day_ = assistx2::FromStringToSecond(ss.at(1));

            if (round_point_cfg_.begin_each_day_ > round_point_cfg_.end_each_day_)
            {
                LOG(ERROR) << ("ASSERT(cfg.begin_each_day_ > cfg.end_each_day_). time:=") << time;
                return -1;
            }
        }


    for (auto sub = node->FirstChildElement(); sub != nullptr; sub = sub->NextSiblingElement())
    {
        PointLevel level;
        level.points_ = atoi(sub->Attribute("need"));
        level.award_ = atol(sub->Attribute("award"));

        round_point_cfg_.levels_.push_back(level);
    }

    std::sort(round_point_cfg_.levels_.begin(), round_point_cfg_.levels_.end(), [](const PointLevel & l, const PointLevel & r)->bool
    {
        return l.points_ > r.points_;
    });

    LOG(INFO) << "ReadRoundPoint begin_time_:=" << round_point_cfg_.begin_time_
        << ", end_time_:=" << round_point_cfg_.end_time_
        << ", begin_each_day_:=" << round_point_cfg_.begin_each_day_
        << ", end_each_day_:=" << round_point_cfg_.end_each_day_
        << ", " << round_point_cfg_.levels_;

    return 0;
}

void ConfigMgr::ReadSystemRedPacketCfg()
{
	auto levels = game_cfg_->GetElement("redpacket");
	if (levels == nullptr)
	{
		throw std::invalid_argument("ASSERT(redpacket == nullptr)");
	}

	system_redpacket_items_.clear();

	for (auto node = levels->FirstChildElement(); node != nullptr; node = node->NextSiblingElement())
	{
		SystemRedPacketItem item;
		item.room_type_id_ = atoll(node->Attribute("roomid"));
		item.num_ = atoll(node->Attribute("num"));
		item.gold_ = atoll(node->Attribute("totalnum"));

		std::vector<std::string > ss;
 		std::string str = node->Attribute("time");
		assistx2::SplitString(str, ",", ss);

		std::for_each(ss.begin(), ss.end(), [&item](std::string & tmp)
		{
			item.times_.push_back(assistx2::FromStringToSecond(tmp));
		});

		std::sort(item.times_.begin(), item.times_.end() );

		LOG(INFO) << "system_redpacket room:=" << item.room_type_id_ << ", times:=" << item.times_ << ", num:=" << item.num_ 
			<< ", gold:=" << item.gold_;

		system_redpacket_items_.push_back(item);
	}
	
	LOG(INFO) << "system_redpacket_items_:=" << system_redpacket_items_.size();
}

void ConfigMgr::ReadAwardItemsCfg()
{
	auto levels = game_cfg_->GetElement("hundredbowl");
	if (levels == nullptr)
	{
		throw std::invalid_argument("ASSERT(hundredbowl == nullptr)");
	}

	award_items_.clear();

	for (auto node = levels->FirstChildElement(); node != nullptr; node = node->NextSiblingElement())
	{
		AwardItem item;
		item.type_ = atoi(node->Attribute("player"));
		item.scale_ = atoi(node->Attribute("scale"));

		std::vector<std::string > ss;
		std::string str = node->Attribute("limits");
		assistx2::SplitString(str, "-", ss);

		if (ss.size() != 2u)
		{
			throw std::invalid_argument("ASSERT(ss.size() != 2u)");
		}

		item.begin_ = atoi(ss.at(0).c_str() );
		item.end_ = atoi(ss.at(1).c_str() );

		LOG(INFO) << "AwardItem type:=" << item.type_ << ", scale:=" << item.scale_ 
			<< ", begin:=" << item.begin_ << ", end:=" << item.end_;

		award_items_.push_back(item);
	}

	LOG(INFO) << "award_items_:=" << award_items_.size();
}

void ConfigMgr::ReadRoundAward()
{
	auto root = game_cfg_->GetElement("paijuaward");
	if (root == nullptr)
	{
		throw std::invalid_argument("ReadRoundAward. ASSERT(root == nullptr)");
	}

	round_award_cfg_.clear();

	for (auto node = root->FirstChildElement(); node != nullptr; node = node->NextSiblingElement())
	{
		RoundAwardCfg item;

		const auto enable = (atoi(node->Attribute("status")) != 0);

		LOG(INFO) << "ReadRoundAward enable:=" << (enable ? "true" : "false");
		if (enable == false)
		{
			continue;
		}

		item.id_ = atoi(node->Attribute("id"));
		item.login_type_ = atoi(node->Attribute("site"));
		item.round_ = atoi(node->Attribute("paiju"));
		item.calc_type_ = atoi(node->Attribute("isadd"));
		item.today_ = atoi(node->Attribute("isdayclear"));

		{
			std::string str = node->Attribute("room");

			std::vector<std::string > ss;
			assistx2::SplitString(str, ",", ss);

			for (auto it : ss)
			{
				item.room_groups_.push_back(atoi(it.c_str()));
			}
		}

		LOG(INFO) << "ReadRoundAward round:=" << item.round_ << ", id:=" << item.id_
			<< ", type_:=" << item.login_type_ << ", rooms:=" << item.room_groups_
			<< ", today_" << item.today_ << ", calc_type_:=" << item.calc_type_;

		{
			auto pt = boost::posix_time::from_time_t(atoll(node->Attribute("stime")));
			item.begin_time_ = boost::date_time::c_local_adjustor<boost::posix_time::ptime>::utc_to_local(pt);

			pt = boost::posix_time::from_time_t(atoll(node->Attribute("etime")));
			item.end_time_ = boost::date_time::c_local_adjustor<boost::posix_time::ptime>::utc_to_local(pt);

			pt = boost::posix_time::from_time_t(atoll(node->Attribute("ctime")));
			item.closed_time_ = boost::date_time::c_local_adjustor<boost::posix_time::ptime>::utc_to_local(pt);

			DLOG(INFO) << "stime:=" << atoll(node->Attribute("stime")) << ", etime:=" << atoll(node->Attribute("etime"))
				<< ", ctime:=" << item.closed_time_;

			if (item.begin_time_ > item.end_time_)
			{
				throw std::invalid_argument("ASSERT(cfg.begin_time_ > cfg.end_time_).");
			}
		}

		{
			std::string time = node->Attribute("time");
			std::vector<std::string > ss;
			assistx2::SplitString(time, "-", ss);

			if (ss.size() != 2U)
			{
				throw std::invalid_argument("PARSER time FAILED.");
			}

			item.begin_each_day_ = assistx2::FromStringToSecond(ss.at(0));
			item.end_each_day_ = assistx2::FromStringToSecond(ss.at(1));

			if (item.begin_each_day_ > item.end_each_day_)
			{
				throw std::invalid_argument("ASSERT(cfg.begin_each_day_ > cfg.end_each_day_). ");
			}
		}

		round_award_cfg_.push_back(item);

		LOG(INFO) << "ReadRoundAward begin_time_:=" << item.begin_time_
			<< ", end_time_:=" << item.end_time_
			<< ", begin_each_day_:=" << item.begin_each_day_
			<< ", end_each_day_:=" << item.end_each_day_;
	}
}

void ConfigMgr::ReadMobileRoundPoint()
{
	auto cfg = ConfigMgr::getInstance()->GetGameCfg();
	if (cfg == nullptr)
	{
		LOG(ERROR) << "ReadMobileRoundPoint assert(cfg == nullptr)";
		return;
	}

	std::int32_t enable = 0;
	cfg->getConfig("paijujifen", "isopen", enable);

	points_activity_cfg_.enable_ = (enable != 0);

	if (enable != 0)
	{
		time_t start = 0;
		time_t end = 0;

		cfg->getConfig("paijujifen", "stime", start);
		cfg->getConfig("paijujifen", "etime", end);

		auto start_pt = boost::date_time::c_local_adjustor<boost::posix_time::ptime>::utc_to_local(boost::posix_time::from_time_t(start));
		auto end_pt = boost::date_time::c_local_adjustor<boost::posix_time::ptime>::utc_to_local(boost::posix_time::from_time_t(end));

		points_activity_cfg_.period_ = new boost::posix_time::time_period(start_pt, end_pt);
		CHECK(points_activity_cfg_.period_->is_null() == false) << "start:=" << start << ", end:=" << end;

		cfg->getConfig("paijujifen", "lose", points_activity_cfg_.factor_);
		CHECK_GE(points_activity_cfg_.factor_, 1);

		LOG(INFO) << "paijujifen begin:=" << assistx2::TimeToString(start) << ", end:=" << assistx2::TimeToString(end)
			<< ", factor_:=" << points_activity_cfg_.factor_;
	}
	else
	{
		LOG(ERROR) << "paijujifen IS DISABLE ";
	}
}

void ConfigMgr::LoadFreeDate()
{
    game_free_date_.clear();
    auto freedate = app_cfg_->GetElement("FreeDate");
    if (freedate == nullptr)
    {
        return;
    }
    for (auto date = freedate->FirstChildElement(); date != nullptr; date = date->NextSiblingElement())
    {
        auto type = date->Attribute("type");
        FreeDate freedate;
        freedate.begin_date = date->Attribute("begin");
        freedate.end_date = date->Attribute("end");

        AddGameFreeDate(type, freedate);
    }
}

void ConfigMgr::AddGameFreeDate(std::string type, FreeDate date)
{
    auto iter = game_free_date_.find(type);
    if (iter != game_free_date_.end())
    {
        iter->second.push_back(date);
    }
    else
    {
        game_free_date_.emplace(type, std::vector<FreeDate>(1, date));
    }
}

const std::map<std::string, std::vector<FreeDate>>& ConfigMgr::game_free_date() const
{
    return game_free_date_;
}

bool ConfigMgr::IsFreeTime(std::string type)
{
    auto& game_free_date = ConfigMgr::getInstance()->game_free_date();
    auto giter = game_free_date.find(type);
    if (giter == game_free_date.end())
    {
        return false;
    }

    time_t nt_time;
    auto now = time(&nt_time);

    auto& vecFreeDate = giter->second;
    for (auto iter : vecFreeDate)
    {
        struct tm begintm;
        strptime(iter.begin_date.c_str(), "%Y-%m-%d %H:%M:%S", &begintm);
        auto begin = mktime(&begintm);

        struct tm endtm;
        strptime(iter.end_date.c_str(), "%Y-%m-%d %H:%M:%S", &endtm);
        auto end = mktime(&endtm);

        DLOG(INFO) << "IsFreeTime: begin:=" << begin << ",end:=" << end << ",now:=" << now;

        if (now >= begin && now <= end)
        {
            return true;
        }
    }

    return false;
}

bool ConfigMgr::IsInFreeTime(std::string type, std::int32_t creattime)
{
    auto& game_free_date = ConfigMgr::getInstance()->game_free_date();
    auto giter = game_free_date.find(type);
    if (giter == game_free_date.end())
    {
        return false;
    }

    auto& vecFreeDate = giter->second;
    for (auto iter : vecFreeDate)
    {
        struct tm begintm;
        strptime(iter.begin_date.c_str(), "%Y-%m-%d %H:%M:%S", &begintm);
        auto begin = mktime(&begintm);

        struct tm endtm;
        strptime(iter.end_date.c_str(), "%Y-%m-%d %H:%M:%S", &endtm);
        auto end = mktime(&endtm);

        DLOG(INFO) << "IsInFreeTime: begin:=" << begin << ",end:=" << end << ",creattime:=" << creattime;

        if (creattime >= begin && creattime <= end)
        {
            return true;
        }
    }

    return false;
}
