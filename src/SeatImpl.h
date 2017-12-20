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
	//�ȴ�
	const static unsigned int PLAYER_STATUS_WAITING			= (1<<1);
	//���ߣ�����������ֻ��������Ҷ���
	const static unsigned int PLAYER_STATUS_NET_CLOSE		= (1<<2);
	//����
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
	//��λ��
    std::int32_t no_;

	//��λ�Ƿ�ɼ�
	bool visible_;

public:
	std::uint32_t status_;

	//�Ƿ�����Ϸ��
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

	//���ָ��
	PlayerInterface * user_ = nullptr;

	//�����û���ѹ����cex
	chips_type bet_;

	//������һ������
    std::int32_t   lastop_;

	//��ҿ��ó�����
	chips_type bankroll_;

	//���������������������λ����������������ã�
	chips_type buy_chips_amount_;

	//������ע��������
	chips_type bet_chips_amount_;

	//����Ӯ�õĳ���������ѹ�ĳ���ֱ�Ӻϲ��� bankroll_�� �����Լ�ѹ�µĳ�������
	chips_type win_chips_;

	//�ֵ������ע��ʱ��
	time_t betting_time_;

	//�������������ʼʱ��
	time_t buytime_;

	//���ߺ��ʱ��
	time_t off_line_time_;

	//����ʱ��
	time_t sitdown_time_;

	//��ҳ�ʱ����
    std::int32_t timeout_count_;

	chips_type maxbet_;
	chips_type minraise_;

	//���������͵ĳ���
	chips_type gift_chips_;

	//��ǰ�ƾ�����
    std::int32_t ranking_;

	//-1�� δ�����0��������1����ׯ
    std::int32_t handhog_ = -1;

    std::vector<std::int32_t > factor_list_;

    std::int32_t factor_ = 0;

    bool has_double_ = false;

	//��
	Ranking show_ranking_;

	std::vector<std::string > show_cards_;

	//������
	HandStrength handstrength_;

	Cards holecards_;
  std::shared_ptr< PlayedCard > playedCard_;

	std::shared_ptr< CardInterface > max_card_;//������

	//�Ƿ�ȷ���˽���
	bool readying_ = false;

	bool kickout_ = false;

	bool is_robot_ = false;

  bool is_tuoguan_ = false;//�й�

  bool is_zanli_ = false;//����

	bool check_cards_ = false;//ը��ţ�Ƿ��ѿ���

	//bool fold_down_ = false;//ը��ţ�Ƿ������ƻ���Ʊ�ˢ

	std::int32_t zjn_status_ = 0;

	std::int32_t turn_count_ = 0;//�Լ�������

	bool ready_ = false;//�Ƿ���׼��

	std::vector<std::int32_t> cmp_seats_;//���ƹ�����λ

	//��������ÿ�ֳ�ʱ�Ĵ���
	std::int32_t time_out_times_ = 0;

  int32_t score_ = 0;
  int32_t bomb_ = 0;
  int32_t bomb_score_ = 0;
  int64_t room_score_ = 0;

  //һ��������ը���ϼ�����
  std::int32_t bombs_amount_ = 0;
  //һ�������ڴ���ϼ�����
  std::int32_t spring_amount_ = 0;

  //����ʱ��ʼʱ��
  time_t start_time_ = -1;

  std::int32_t bei_count_ = 1;
  std::int32_t disband_vote_ = 0; //��ɢͶƱ
};

#endif //_XNNPOKER_SRC_SEAT_IMPL_H_


