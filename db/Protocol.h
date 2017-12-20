#ifndef _X_CLOUD_INCLUDE_PROTOCOL_
#define _X_CLOUD_INCLUDE_PROTOCOL_

namespace cloud
{
	//获取membergame对像
	static const short GET_MEMBER_GAME_OBJECT				= 1000;

	//返回membergame对像. [cmd， err,  bin]
	static const short PUSH_MEMBER_GAME_OBJECT			= 1000;

	//Increment   field [cmd, key, incr]
	static const short INCREMENT_OBJECT_FIELD_VALUE		= 1001;

	//更新 membergame [cmd, uid_, coin_increase_, win_count_, round_count_,  str, max_win_chips_]
	static const short UPDATE_MEMBER_GAME						= 1002;

	//同步奖池
	static const short UPDATE_AWARD_POOL						= 1006;

	//领取奖金
	static const short GET_AWARD											= 1007;

	//发起金币统计
	static const short START_GOLD_STAT = 1011;
}

#endif