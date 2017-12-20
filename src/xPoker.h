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
	std::size_t seat;					//��λ��
	std::size_t maxplayer;			//�������� (�Թ۵� + ���µ�)

	std::string	type;					//��������
};

struct roomcfg_type : public roombasecfg_type
{
	chips_type	sb;					//Сäע
	chips_type	minchips;			//��СЯ����
	chips_type	maxchips;			//���Я����

	time_t			bettime;			//��ע��ʱ
	
	chips_type	taxation;			//��ˮ. ������

	time_t          handhog;			//��ׯ
	time_t			show;				//����

	//�۱��轱��
	std::int32_t				royalflush;			//�ʼ�ͬ��˳
    std::int32_t				straightflush;	//ͬ��˳
    std::int32_t				fourofakind;		//����

	//
    std::int32_t				robot_mode;					//������ģʽ
	std::size_t		max_robot_amount;		//��������Ͷ������

    std::int32_t				type_id;						//��������ID

    std::int32_t				taxes_mode;			//��ˮģʽ

	std::int64_t min_bet = 0;
	std::int64_t max_bet = 0;
};

//��������
enum
{
	//novice
	ROOM_TYPE_COIN_ROOM = 0,
	ROOM_TYPE_GLOD_ROOM
};

enum TaxesMode { FIXED_TAXES_MODE,  RATIO_TAXES_MODE };

struct roomlevelcfg_type : public roomcfg_type
{
    std::int32_t begin;				//�������ʼID
    std::int32_t end;					//����ν���ID
};

typedef std::vector<roomlevelcfg_type > roomscfg_type;

class RedPacket
{
public:
	std::string id_;

	//Ϊ0�Ļ�������ϵͳ���
	std::int32_t mid_ = 0;

	std::int32_t num_ = 0;

	time_t t_ = 0;

	chips_type gold_ = 0;

	std::string text_;

	//     //�������б�
	//     std::vector < std::int32_t > mids_;

	//����ȡ�������ip�б�ͬһ��ipֻ����ȡһ��
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

