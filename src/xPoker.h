#ifndef _X_POKER_H_
#define _X_POKER_H_

#include <string>
#include <vector>
#include <ctime>
#include <set>

typedef std::int64_t chips_type;

typedef std::uint32_t uid_type;

struct roombasecfg_type
{
	std::size_t seat;					//座位数
	std::size_t maxplayer;			//最大玩家数 (旁观的 + 坐下的)

	std::string	type;					//房间类型
};

struct roomcfg_type : public roombasecfg_type
{
	chips_type	sb;					//小盲注
	chips_type	minchips;			//最小携带量
	chips_type	maxchips;			//最大携带量

	time_t			bettime;			//下注限时
	
	chips_type	taxation;			//抽水. 筹码数

	time_t          handhog;			//抢庄
	time_t			show;				//亮牌

	//聚宝盆奖励
	std::int32_t				royalflush;			//皇家同花顺
    std::int32_t				straightflush;	//同花顺
    std::int32_t				fourofakind;		//四条

	//
    std::int32_t				robot_mode;					//机器人模式
	std::size_t		max_robot_amount;		//最大机器人投放数量

    std::int32_t				type_id;						//房间类型ID

    std::int32_t				taxes_mode;			//抽水模式

	std::int64_t min_bet = 0;
	std::int64_t max_bet = 0;
};

//房间类型
enum
{
	//novice
	ROOM_TYPE_COIN_ROOM = 0,
	ROOM_TYPE_GLOD_ROOM
};

enum TaxesMode { FIXED_TAXES_MODE,  RATIO_TAXES_MODE };

struct roomlevelcfg_type : public roomcfg_type
{
    std::int32_t begin;				//房间段起始ID
    std::int32_t end;					//房间段结束ID
};

typedef std::vector<roomlevelcfg_type > roomscfg_type;

class RedPacket
{
public:
	std::string id_;

	//为0的话，就是系统红包
	std::int32_t mid_ = 0;

	std::int32_t num_ = 0;

	time_t t_ = 0;

	chips_type gold_ = 0;

	std::string text_;

	//     //接收者列表
	//     std::vector < std::int32_t > mids_;

	//已领取过得玩家ip列表，同一个ip只能领取一次
	std::set<std::string> token_ips;
};

typedef std::vector<RedPacket > RedPackets;


class SystemRedPacketItem
{
public:
	std::int32_t room_type_id_ = 0;

	std::int32_t num_ = 0;

	chips_type gold_ = 0;

	std::vector<std::time_t > times_;
};

typedef std::vector<SystemRedPacketItem > SystemRedPacketItems;

struct runfastroomcfg
{
  std::int32_t begin;
  std::int32_t end;
  std::int32_t ju;
  std::int32_t cost;
  std::string type;
  std::string name;
  std::int32_t serverid;
  std::int64_t taxation;
  time_t bettime;
  std::int32_t taxes_mode;
  std::int32_t sb;
  std::int64_t min;
  std::int64_t max;
};

typedef std::vector< runfastroomcfg > runfastroomscfg_type;

#endif //_X_POKER_H_

