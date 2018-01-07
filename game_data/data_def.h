#ifndef _XPOKER_SRC_DATADEF_H_
#define _XPOKER_SRC_DATADEF_H_

#include "helper.h"
#include <boost/date_time/posix_time/ptime.hpp>

struct PropsBase_type
{
	std::int32_t pcate = 0;
	std::int32_t pframe = 0;

	PropsBase_type()
	{

	}

	PropsBase_type(std::int32_t cate, std::int32_t frame):pcate(cate), pframe(frame)
	{
	}
};

struct Prop_type : public PropsBase_type
{
	enum { STATE_NORMAL, STATE_IN_USE };

	Prop_type(std::int32_t cate, std::int32_t frame):PropsBase_type(cate, frame) {}

	Prop_type():PropsBase_type(0, 0) {}

	bool operator == (const Prop_type & other )const
	{
		return pcate == other.pcate && pframe == other.pframe; 
	}

	bool operator == (const std::int32_t mppid )const
	{
		return pid == mppid;
	}

	std::int32_t pid;
	std::int32_t status;
	std::int32_t sptime;
	std::int32_t sltime;
	std::int32_t sendmid;
};

typedef std::vector<Prop_type > Props_type;

void ToJson(const Props_type & props, json_spirit::Value & value);

struct PropsCfg_type : public PropsBase_type
{
	PropsCfg_type(boost::int32_t cate, boost::int32_t frame):PropsBase_type(cate, frame) {}

	PropsCfg_type():PropsBase_type(0, 0) {}

	bool operator == (const PropsCfg_type & other)const
	{
		return other.pcate == pcate && other.pframe == pframe;
	}

	bool operator < (const PropsCfg_type & other)const
	{
		if (other.pcate == pcate)
		{
			return other.pframe < pframe;
		}
		else
		{
			return other.pcate < pcate;
		}
	}

	std::int32_t price;
	std::int32_t bind;
	std::int32_t validity;
	std::int32_t authority;
	std::int32_t discount;
	std::int32_t recount;
};

typedef std::vector<PropsCfg_type > PropsCfgs_type;

#if 0

struct HonorBase_type
{
	HonorBase_type(boost::int32_t cate, boost::int32_t frame): pcate(cate), pframe(frame) {}

	HonorBase_type(): pcate(0), pframe(0) {}

	virtual ~HonorBase_type() {}

	boost::int32_t pcate;
	boost::int32_t pframe;
};

struct HonorRecord_type : public HonorBase_type
{
	HonorRecord_type():update(false) { }

	HonorRecord_type(boost::int32_t cate, boost::int32_t frame):HonorBase_type(cate, frame), complete(false), update(false) { }

	virtual ~HonorRecord_type() {}

	bool operator == (const HonorBase_type & other )const
	{
		return pcate == other.pcate && pframe == other.pframe;
	}
	
	time_t timestamp;
	json_spirit::Value data;			//任务完成状态数据

	bool complete;						//任务是否完成	
	
	bool update;
};

typedef std::vector<HonorRecord_type > HonorRecords_type;

//hit->pcate == 11 牌友数达到[牌友数]
//hit->pcate == 10 金币达到 [金币数]
//hit->pcate == 9	最后一个操作，并获胜 [最后操作]
//hit->pcate == 8	一局内收到礼物 [礼物个数]
//hit->pcate == 7	和朋友一起玩
//hit->pcate == 6	收到礼物次数
//hit->pcate == 5	连胜 [连胜次数]
//hit->pcate == 4	赢取筹码 + 次数 ， 成就定义: [ 单次赢取筹码数,  次数 ]
//hit->pcate == 3	坐下成就 []
//hit->pcate == 2	成手牌类成就  [ 牌型,  次数 ]
//hit->pcate == 1	底牌类成就 [ 牌型 ]

struct Honor_type : public HonorBase_type
{
	virtual ~Honor_type() {};

	bool operator == (const HonorBase_type & other )const
	{
		return pcate == other.pcate && pframe == other.pframe;
	}
	
	boost::int32_t type;
	boost::int32_t targetlevel;
	boost::int32_t point;
	boost::int32_t award;
	json_spirit::Array data;
};

typedef std::vector<Honor_type > HonorCfg_type;

#endif

struct LevelItem
{
	std::int32_t level;
	chips_type price;

	bool operator < (const LevelItem & other)const 
	{
		return level < other.level;
	} 
};

typedef std::vector<LevelItem > LevelItems_type;

struct EventData
{
	uid_type mid;
	std::int32_t room_id;
	time_t delay;

	bool operator == (const uid_type mid)const
	{
		return this->mid == mid;
	}
};

class GiftProps
{
public:
	PropsBase_type				prop_;							//道具
	time_t									expire_;
	std::int32_t										probability_;				//概率， 1/1000
	std::int32_t										num_;							//固定掉落数

																						//当天最大掉落个数
	std::int32_t today_max_ = 0;
	//当天已掉落个数
	std::int32_t today_ = 0;
	//
	std::int32_t  yday_ = 0;
};

class GiftCfg
{
public:
	time_t begin_time_;			//活动开始时间
	time_t end_time_;			//活动结束时间

	bool enable_ = false;

	std::int32_t room_type_id_;			//房间类型ID

	bool operator==(const GiftCfg & other )const 
	{
		return other.room_type_id_ == room_type_id_;
	}

	bool operator==(const boost::int32_t room_type_id)const 
	{
		return room_type_id == room_type_id_;
	}

	std::vector<GiftProps >  props_;		 //
};

typedef std::vector<GiftCfg > RoomGiftCfg;

//大场玩牌奖励
class AwardCfg
{
public:
	std::vector<std::int32_t > room_groups_;

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
};

class RoundAwardCfg : public AwardCfg
{
public:
	std::int32_t round_ = 0;

	// 0 不循环计算， 1 循环 
	std::int32_t calc_type_ = 0;

	// 0  每日不清零， 否则清零
	std::int32_t today_ = 0;
};

typedef std::vector<RoundAwardCfg > RoundAwardCfgs;

std::string GBKToUTF8(const std::string& strGBK);

#endif //_XPOKER_SRC_DATADEF_H_

