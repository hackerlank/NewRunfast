#ifndef _XNNPOKER_SRC_PLAYER_TODAY_DATA_H_
#define _XNNPOKER_SRC_PLAYER_TODAY_DATA_H_

#include "xpoker.h"

//玩家的每日数据
class PlayerDayData
{
public:
	PlayerDayData()
		:round_count_(0), gold_gift_count_(0), 
		yday_(0), max_win_chips_(0), gold_incr_(0), playtime_(0)
	{

	}

	std::string ToString()const;

	static  void FromString(const std::string & str, PlayerDayData & data);

	int IncrRoundCount();

	int GetRoundCount();

	void IncrGoldGiftCount();

	int  GetGoldGiftCount();

	void SetMaxWin(const chips_type win_gold);

	time_t GetPlayTime();

	time_t IncrPlayTime(const time_t incr);

	void SetGoldIncr(const chips_type incr);

private:
	bool IsToday();

	void Clear();

	//当天牌局记数
	int round_count_;

	//金币币当天赠送次数
	int gold_gift_count_;

	//记录时间
	int yday_;

	//当天最高赢取筹码数
	chips_type max_win_chips_;

	//当天金币增量
	chips_type gold_incr_;

	//在玩时间
	time_t playtime_;
};

#endif //_XNNPOKER_SRC_PLAYER_TODAY_DATA_H_




