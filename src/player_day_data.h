#ifndef _XNNPOKER_SRC_PLAYER_TODAY_DATA_H_
#define _XNNPOKER_SRC_PLAYER_TODAY_DATA_H_

#include "xPoker.h"

//��ҵ�ÿ������
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

	//�����ƾּ���
	int round_count_;

	//��ұҵ������ʹ���
	int gold_gift_count_;

	//��¼ʱ��
	int yday_;

	//�������Ӯȡ������
	chips_type max_win_chips_;

	//����������
	chips_type gold_incr_;

	//����ʱ��
	time_t playtime_;
};

#endif //_XNNPOKER_SRC_PLAYER_TODAY_DATA_H_




