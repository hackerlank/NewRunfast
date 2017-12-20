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
	json_spirit::Value data;			//�������״̬����

	bool complete;						//�����Ƿ����	
	
	bool update;
};

typedef std::vector<HonorRecord_type > HonorRecords_type;

//hit->pcate == 11 �������ﵽ[������]
//hit->pcate == 10 ��Ҵﵽ [�����]
//hit->pcate == 9	���һ������������ʤ [������]
//hit->pcate == 8	һ�����յ����� [�������]
//hit->pcate == 7	������һ����
//hit->pcate == 6	�յ��������
//hit->pcate == 5	��ʤ [��ʤ����]
//hit->pcate == 4	Ӯȡ���� + ���� �� �ɾͶ���: [ ����Ӯȡ������,  ���� ]
//hit->pcate == 3	���³ɾ� []
//hit->pcate == 2	��������ɾ�  [ ����,  ���� ]
//hit->pcate == 1	������ɾ� [ ���� ]

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
	PropsBase_type				prop_;							//����
	time_t									expire_;
	std::int32_t										probability_;				//���ʣ� 1/1000
	std::int32_t										num_;							//�̶�������

																						//�������������
	std::int32_t today_max_ = 0;
	//�����ѵ������
	std::int32_t today_ = 0;
	//
	std::int32_t  yday_ = 0;
};

class GiftCfg
{
public:
	time_t begin_time_;			//���ʼʱ��
	time_t end_time_;			//�����ʱ��

	bool enable_ = false;

	std::int32_t room_type_id_;			//��������ID

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

//�����ƽ���
class AwardCfg
{
public:
	std::vector<std::int32_t > room_groups_;

	boost::posix_time::ptime begin_time_;

	std::time_t begin_each_day_;

	boost::posix_time::ptime end_time_;

	std::time_t end_each_day_;

	//�ر����ʱ��
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

	// 0 ��ѭ�����㣬 1 ѭ�� 
	std::int32_t calc_type_ = 0;

	// 0  ÿ�ղ����㣬 ��������
	std::int32_t today_ = 0;
};

typedef std::vector<RoundAwardCfg > RoundAwardCfgs;

std::string GBKToUTF8(const std::string& strGBK);

#endif //_XPOKER_SRC_DATADEF_H_

