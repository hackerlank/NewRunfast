#ifndef _XNNPOKER_SRC_SEAT_IMPL_H_
#define _XNNPOKER_SRC_SEAT_IMPL_H_

#include "xPoker.h"
#include "referee.h"
class PlayedCard;
class PlayerInterface;

class Seat
{
	friend class Table;
public:
	//等待
	const static unsigned int PLAYER_STATUS_WAITING			= (1<<1);
	//掉线，掉线重连，只对在玩玩家而言
	const static unsigned int PLAYER_STATUS_NET_CLOSE		= (1<<2);
	//在玩
	const static unsigned int PLAYER_STATUS_PLAYING			= (1<<3);

	enum
	{
		emZjnStatus_Normal = 0,
		emZjnStatus_Fold = 1,
		emZjnStatus_CompareFailed = 2,
	};

public:
	Seat(std::int32_t seat)
		:no_(seat), visible_(false), status_(PLAYER_STATUS_WAITING),  bet_(0), lastop_(0),  bankroll_(0), buy_chips_amount_(0),
		bet_chips_amount_(0), win_chips_(0), betting_time_(0), buytime_(0), off_line_time_(0), timeout_count_(0), 
		maxbet_(0),  minraise_(0), gift_chips_(0), ranking_(0), show_ranking_(INVALID_RANKING)
	{
		
	}

public:
	//坐位号
    std::int32_t no_;

	//座位是否可见
	bool visible_;

public:
	std::uint32_t status_;

	//是否在游戏中
	bool ingame()const
	{
		return (status_ & PLAYER_STATUS_PLAYING) != 0;
	}

  bool isZanli() const
  {
    return is_zanli_;
  }

	bool IsZjnLive() const
	{
		return (status_ & PLAYER_STATUS_PLAYING) != 0 && zjn_status_ == emZjnStatus_Normal;
	}

    chips_type abs_win_chips()const
    {
        return win_chips_ - bet_chips_amount_;
    }

public:

	//玩家指针
	PlayerInterface * user_ = nullptr;

	//本轮用户已压筹码cex
	chips_type bet_;

	//玩家最后一个操作
    std::int32_t   lastop_;

	//玩家可用筹码数
	chips_type bankroll_;

	//买入筹码总数（包括在坐位上重新买和赠送所得）
	chips_type buy_chips_amount_;

	//本轮下注筹码总数
	chips_type bet_chips_amount_;

	//本轮赢得的筹码数（多压的筹码直接合并到 bankroll_， 包括自己压下的筹码数）
	chips_type win_chips_;

	//轮到玩家下注的时间
	time_t betting_time_;

	//重新买入筹码启始时间
	time_t buytime_;

	//掉线后的时间
	time_t off_line_time_;

	//坐下时间
	time_t sitdown_time_;

	//玩家超时次数
    std::int32_t timeout_count_;

	chips_type maxbet_;
	chips_type minraise_;

	//别的玩家赠送的筹码
	chips_type gift_chips_;

	//当前牌局排名
    std::int32_t ranking_;

	//-1， 未表决。0，不抢。1，抢庄
    std::int32_t handhog_ = -1;

    std::vector<std::int32_t > factor_list_;

    std::int32_t factor_ = 0;

    bool has_double_ = false;

	//亮
	Ranking show_ranking_;

	std::vector<std::string > show_cards_;

	//成手牌
	HandStrength handstrength_;

	Cards holecards_;
  std::shared_ptr< PlayedCard > playedCard_;

	std::shared_ptr< CardInterface > max_card_;//最大的牌

	//是否确认了结算
	bool readying_ = false;

	bool kickout_ = false;

	bool is_robot_ = false;

  bool is_tuoguan_ = false;//托管

  bool is_zanli_ = false;//暂离

	bool check_cards_ = false;//炸金牛是否已看牌

	//bool fold_down_ = false;//炸金牛是否已弃牌或比牌被刷

	std::int32_t zjn_status_ = 0;

	std::int32_t turn_count_ = 0;//自己的轮数

	bool ready_ = false;//是否已准备

	std::vector<std::int32_t> cmp_seats_;//比牌过的座位

	//看看场的每局超时的次数
	std::int32_t time_out_times_ = 0;

  int32_t score_ = 0;
  int32_t bomb_ = 0;
  int32_t bomb_score_ = 0;
  int64_t room_score_ = 0;

  //一个房间内炸弹合计数量
  std::int32_t bombs_amount_ = 0;
  //一个房间内春天合计数量
  std::int32_t spring_amount_ = 0;

  //倒计时开始时间
  time_t start_time_ = -1;

  std::int32_t bei_count_ = 1;
  std::int32_t disband_vote_ = 0; //解散投票
};

#endif //_XNNPOKER_SRC_SEAT_IMPL_H_


