#ifndef  _NIUNIU_SRC_CONFIGMGR_H_
#define _NIUNIU_SRC_CONFIGMGR_H_

#include <assistx2/singleton.h>

#include <list>

#include <assistx2/database_wrapper.h>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>

#include "DataDef.h"
#include <string>
#include <map>
#include <vector>

struct FreeDate
{
    std::string begin_date;
    std::string end_date;
};

class Configure;

class PointLevel
{
public:
    std::int32_t points_ = 0;
    chips_type award_ = 0;
};

class AwardItem
{
public:
	chips_type begin_ = 0;
	chips_type end_ = 0;

	std::int32_t scale_ = 0;
	std::int32_t type_ = 0;
};

typedef std::vector<AwardItem > AwardItems;

class RoundPointCfg
{
public:
    std::map<std::int32_t, std::int32_t > room_groups_;

    boost::posix_time::ptime begin_time_;

    std::time_t begin_each_day_;

    boost::posix_time::ptime end_time_;

    std::time_t end_each_day_;

    //关闭面板时间
    boost::posix_time::ptime closed_time_;

    std::int32_t id_;

    //
    std::int32_t login_type_;

    void Clear()
    {
        room_groups_.clear();
    }

    std::int32_t max_ = 0;

    std::int32_t decay_ = 0;

    std::vector<PointLevel > levels_;
};

typedef std::vector<RoundPointCfg > RoundPointCfgs;

//百人场积分活动时间
class PointsActivitiesCfg
{
public:
	bool enable_ = false;

	std::int32_t factor_ = 0;

	boost::posix_time::time_period   *  period_ = nullptr;
};

class ConfigMgr : public Singleton<ConfigMgr >
{
private:
	friend class DefaultBuilder<ConfigMgr >;

	ConfigMgr(void);
	virtual ~ConfigMgr(void);
public:

	//加载本地配置文件
    std::int32_t LoadLocalCfg();

	std::int32_t Update();

	const DBConfig_type & getDBConfig()const
	{
		return dbcfg_;
	}

	boost::int32_t GetSID()const
	{
		return sid_;
	}

    const RoundPointCfg & GetRoundPointCfg()const
    {
        return round_point_cfg_;
    }

    Configure * GetAppCfg()
    {
        return app_cfg_;
    }

    const std::string & GetPidFilePath()const
    {
        return pid_file_path_;
    }

    const roomscfg_type & getRoomsCfg()const
    {
        return roomscfg_;
    }

    const runfastroomscfg_type & getRunFastRoomCfg() const
    {
      return rfroomscfg_;
    }

	const SystemRedPacketItems & GetSystemRedPacketItems()const
	{
		return system_redpacket_items_;
	}

	//百人场聚宝盘
	const AwardItems & GetAwardItems()const
	{
		return award_items_;
	}

	const RoundAwardCfgs &  GetRoundAwardCfg()const
	{
		return round_award_cfg_;
	}

	const PointsActivitiesCfg& GetMobileRoundPointCfg()const
	{
		return points_activity_cfg_;
	}

    Configure * GetGameCfg()
    {
        return game_cfg_;
    }

	void AddListener(std::function< void() >  cb)
	{
		listeners_.push_back(cb);
	}

    std::string server_version()
    {
        return server_version_;
    }

    const std::map<std::string, std::vector<FreeDate>>& game_free_date() const;
    bool IsFreeTime(std::string type);
    bool IsInFreeTime(std::string type, std::int32_t creattime);
private:
    void LoadAppCfg();

    void LoadGameCfg();

    void ReadDBCfg();

    std::int32_t ReadRoundPoint();

    void ReadRoomsCfg();

    void ReadRunFastRoomsCfg();

	void ReadSystemRedPacketCfg();

	void ReadAwardItemsCfg();

	void ReadRoundAward();

	void ReadMobileRoundPoint();

    void LoadFreeDate();
    void AddGameFreeDate(std::string type, FreeDate date);
private:
	std::int32_t sid_;

    std::string pid_file_path_;

    Configure * app_cfg_ = nullptr;

    Configure * game_cfg_ = nullptr;

	roomscfg_type roomscfg_;

  runfastroomscfg_type rfroomscfg_;

	DBConfig_type dbcfg_;

    //玩牌积分
    RoundPointCfg round_point_cfg_;

	SystemRedPacketItems system_redpacket_items_;

	//百人场聚宝盘
	AwardItems award_items_;

	//玩牌抽奖
	RoundAwardCfgs round_award_cfg_;

	//百人场积分活动时间
	PointsActivitiesCfg points_activity_cfg_;

	std::list< std::function< void() > > listeners_;

    std::string server_version_;

    std::map<std::string, std::vector<FreeDate>> game_free_date_;
};

#endif //_CONFIGMGR_H_

