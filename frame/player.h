#ifndef _XPOKER_SRC_PLAYER_H_
#define _XPOKER_SRC_PLAYER_H_

#include <assistx2/tcphandler_wrapper.h>

#include "player_base.h"
#include "data_layer.h"

#include "player_day_data.h"

class Player : public PlayerBase
{
public:
	explicit Player(const uid_type mid);

	virtual ~Player(void);

	//初始化玩家数据
	virtual int Serialize(bool loadorsave);

	virtual bool GoldPay(const chips_type gold);

    virtual bool UpdateScore(const chips_type score);

    virtual bool PropsPay(std::int32_t pcate, std::int32_t pframe, std::int32_t num,bool isPay = true);

	virtual bool ForceGoldPay(const chips_type gold, chips_type& real_delta);

	virtual void OnGameOver(bool winner) override;

 	virtual const MemberGame & GetGameInfo()const
 	{
 		return gameinfo_;
 	}

	virtual int GetRichesRanking()const
	{
		return riches_rank_;
	}

	virtual int GetWinPointRanking()const
	{
		return win_rank_;
	}

	virtual const PropsBase_type & GetTableProp()const
	{
		return table_prop_;
	}

	virtual void SetTableProp(const PropsBase_type & prop);

	virtual void SitUp();

	virtual void SitDown(int seat);

	virtual void Destroy()
	{
		delete this;
	}

	virtual PlayerDayData & GetTodayData();

	virtual const MemberCommonGame & GetGameBaseInfo()const override
	{
		return game_base_;
	}

protected:
	assistx2::TcpHanlderWrapper * connector_ = nullptr;

	//最高赢取筹码数
	chips_type max_win_chips_ = 0;

	//获胜次数
	std::int32_t win_count_ = 0;

	//牌局记数
	std::int32_t round_count_ = 0;

	PlayerDayData today_data_;

	//桌面装饰道具
	PropsBase_type table_prop_;

	//最佳成手牌
	Cards cards_;

	MemberGame gameinfo_; 

	MemberCommonGame game_base_;

};

#endif //_XPOKER_SRC_PLAYER_H_


