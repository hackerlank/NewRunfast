#ifndef _X_POKER_CMD_H_
#define _X_POKER_CMD_H_

#include <stdint.h>

static const std::int32_t SOCKET_CLOSE_MSG = 0;

namespace Proxy
{
    namespace error_code
    {
        const static std::int32_t ERROR_TIME_OUT = 10000;
    }

    const static std::int16_t CLINET_REQUEST_REGISTER = 10000;

    //response
    const static std::int16_t SERVER_RESPONSE_REGISTER = 10000;

    const static std::int16_t 	GATEWAY_EVENT_SERVER_ACTIVE = 10001;

    const static std::int16_t 	GATEWAY_EVENT_SERVER_SHUTDOWN = 10002;

    const static std::int16_t 	GATEWAY_EVENT_SERVER_CLOSE = 10003;

    //ϵͳ����ָ�� ��0�� �رշ������� 1�� �������������ļ�, 2 �����������رգ�
    const static std::int16_t SYSTEM_ADMINI_CMD = 10004;

    //ϵͳ�㲥
    const static std::int16_t SYSTEM_BROADCAST = 10005;

    //��������
    const static std::int16_t HEARTBEAT_PACKET = 10006;

    //converge
    const static std::int16_t CONVERGE_PACKET = 10007;

    //��������֮���ͨѸ
    const static std::int16_t REQUEST_SYNC = 10008;

    const std::int32_t	ADMINI_CMD_CLOSE_SERVER = 0;
    const std::int32_t	ADMINI_CMD_RELOAD = 1;
    const std::int32_t	ADMINI_CMD_STOP_SERVER = 2;

    static const std::int32_t	SESSION_TYPE_BEGIN = 0;
    static const std::int32_t	SESSION_TYPE_CLINET = 1;			//�ͻ�������
    static const std::int32_t	SESSION_TYPE_SERVER = 2;			//������	
    static const std::int32_t	SESSION_TYPE_END = 3;

    static const std::int32_t REGISTER_SUCCESS = 0;

    static const std::int32_t REGISTER_FAILED = -1;

    //�û������½
    const static std::int32_t CLIENT_REQUEST_LOGIN = 1000;

    const static std::int32_t SERVER_RESPONSE_LOGIN = 1000;
};


namespace Texas
{
	namespace error_code
	{
        const static std::int32_t ERROR_SUCCEED = 0;
		//��������
		const static std::int32_t ERROR_ENTER_ROOM_FILLED						= -1;

		const static std::int32_t ERROR_ENTER_ROOM_NOT_FIND_ROOM	= -2;

		//blacklisk
		const static std::int32_t ERROR_ENTER_ROOM_BLACKLIST				= -3;
			
		//������Ч
		const static std::int32_t ERROR_SITDOWN_ARG_INVAILD					= -3;
		//��λ�ǿ�
		const static std::int32_t ERROR_SITDOWN_SEAT_NOT_NULL			= -4;

		//enough ��ϰ�Ҳ���
		const static std::int32_t ERROR_COIN_NOT_ENOUGH						= -5;
		//��Ҳ���
		const static std::int32_t ERROR_GOLD_NOT_ENOUGH						= -6; 

		//������Ч
		const static std::int32_t ERROR_ARG_INVAILD									= -7;

		//�����������ر�
		const static std::int32_t ERROR_SERVER_CLOSED								= -8;

		//����ʧ�ܣ�����ͬһIP
		const static std::int32_t ERROR_SITDOWN_FROM_SAME_IP				= -10;

        static const std::int32_t ERROR_WRITE_REDIS_ERROR = -11;

        static const std::int32_t ERROR_READ_REDIS_ERROR = -12;

		static const std::int32_t ERROR_UNKNOWN = -13;

		static const std::int32_t ERROR_ISNULL = -14;

		static const std::int32_t ERROR_KICK_LONG_TIME_NO_OPERATION = -15;//���ˣ���ʱ��δ����

		static const std::int32_t ERROR_KICK_GOLD_BELOW_FLOOR = -16;//���ˣ���ҵ��ڷ�������

		static const std::int32_t ERROR_BET_VALUE_ILLEGAL = -17;//��ע���Ƿ�

		static const std::int32_t ERROR_KICK_NOT_READY = -18;//���ˣ�û��׼��

		static const std::int32_t ERROR_KICK_SYS_KICK = -19;//���ˣ�ϵͳ����

		static const std::int32_t ERROR_KICK_OFF_LINE = -20;//���ˣ�����

		static const std::int32_t ERROR_STILL_IN_GAME = -21;//������Ϸ�У����ܻ���

		static const std::int32_t ERROR_CAHNGE_ROOM = -22;//����

		static const std::int32_t ERROR_SEAT_FULL = -23;//��λ����

        static const std::int32_t   REGISTER_SUCCESS = 0;			//��½�ɹ�
        static const std::int32_t   REGISTER_FAILED = 1;		//��½ʧ��, δ֪����.
        static const std::int32_t   REGISTER_KEY_ERROR = 2;			//�ỰKEYУ��ʧ��
        static const std::int32_t   REGISTER_RELOGIN = 3;			//�ظ���½, ֻ�ᷢ��ǰһ������.
	}

	//�û������½
	const static std::int16_t CLIENT_REQUEST_LOGIN							= 1000;

	//�û�������뷿��
	const static std::int16_t CLIENT_REQUEST_ENTER_ROOM				= 1001;

	//����������Ӧ���뷿�������response
	//-1, ���䲻���ڣ�0 �� �ɹ���
	const static std::int16_t SERVER_RESPONSE_ENTER_ROOM			= 1001;

	//�����������ͷ��������Ϣsnapshot
	const static std::int16_t SERVER_PUSH_PLAYERS_SNAPSHOT		= 1002;

	//������������������Ϣsnapshot
	const static std::int16_t SERVER_PUSH_TABLE_SNAPSHOT			= 1003;

	//�û������뿪����
	const static std::int16_t CLIENT_REQUEST_LEAVE_ROOM				= 1004;

	//�����������뿪���������response
	const static std::int16_t SERVER_RESPONSE_LEAVE_ROOM			= 1004;

	//�������˹㲥���뷿��������Ϣ
	const static std::int16_t SERVER_BROADCAST_PLAYER_INFO			= 1005;

	//�û���������
	const static std::int16_t CLIENT_REQUEST_SITDOWN					= 1006;

	//����������Ӧ��������response
	const static std::int16_t SERVER_RESPONSE_SITDOWN					= 1006;

	//�û�����վ��
	const static std::int16_t CLIENT_REQUEST_SITUP							= 1007;

	//����������Ӧվ������response
	const static std::int16_t SERVER_RESPONSE_SITUP						= 1007;

	//�������㲥��Ϸ��ʼ
	const static std::int16_t SERVER_BROADCAST_GAMESTART			= 1008;

	//�������㲥ת��˭��ע��
	const static std::int16_t SERVER_PUSH_NEXT_TAKER						= 1009;

	//Flop
	const static std::int16_t SERVER_BROADCAST_FLOP						= 1010;	

#if 0
	//TURN
	const static std::int16_t SERVER_BROADCAST_TRUN						= 1011;

	//RIVER
	const static std::int16_t SERVER_BROADCAST_RIVER						= 1012;
#endif //0

	//�û���ע
	const static std::int16_t CLIENT_REQUEST_BETTING						= 1013;

	//�������㲥��ע
	const static std::int16_t SERVER_BROADCAST_BETTING					= 1013;

	//����
	const static std::int16_t SERVER_BROADCAST_RESULT					= 1023;

	//��ǰ�غϳص�
	const static std::int16_t SERVER_BROADCAST_POTS						= 1024;

	//���ط���
	const static std::int16_t SERVER_BROADCAST_POTS_MALLOC		= 1025;

	//�㲥��Ϸ����
	const static std::int16_t SERVER_BROADCAST_GAMEOVER			= 1026;

	//�û��������
	const static std::int16_t CLIENT_REQUES_BUY_CHIPS					= 1027;

	//��������Ӧ�������response
	const static std::int16_t SERVER_RESPONSE_BUY_CHIPS				= 1027;

	//�������㲥�н�����Ϣ
	const static std::int16_t SERVER_BROADCAST_AWARD					= 1029;

	//���ͳ���
	const static std::int16_t CLIENT_REQUEST_GIFT_CHIPS					= 1030;

	//���ͳ���
	const static std::int16_t SERVER_RESPONSE_GIFT_CHIPS				= 1030;

	//������ϰ��
	const static std::int16_t SERVER_LEARNER_GIFT_CHIPS					= 1031;

	//ʹ�ñ���
	const static std::int16_t CLIENT_REQUEST_USE_FACE					= 1032;

	//ʹ�ñ���
	const static std::int16_t SERVER_RESPONSE_USE_FACE					= 1032;

	//�������
	const static std::int16_t CLEINT_REQUEST_ADD_FRIEND				= 1033;

	//�㲥��Ϊ����
	const static std::int16_t SERVER_BROADCAST_ADD_FRIEND			= 1033;

	//�û���Ӧ�����Ϊ����
	const static std::int16_t CLEINT_RESPONSE_ADD_FRIEND				= 1034;

	//�㲥����������
	const static std::int16_t SERVER_RESPONSE_ADD_FRIEND			= 1034;

	const static std::int16_t SERVER_PUSH_HOLD_CARDS					= 1035;

	//�����û�����ʱ�䣨���¿�ʼ��ʱ��
	const static std::int16_t SERVER_PUSH_PLAYER_PLAYING_TIME	= 1036;	

	//rob dealer�㲥��ʼ��ׯ
	const static std::int16_t SERVER_PUSH_ROB_DEALER						= 1037;

	//���������ׯ
	const static std::int16_t CLEINT_RESPONSE_ROB_DEALER				= 1038;

	//���������ׯ
	const static std::int16_t SERVER_RESPONSE_ROB_DEALER				= 1038;

	//rob dealer�㲥ׯ��
	const static std::int16_t SERVER_PUSH_DEALER								= 1039;

	//���������ׯ
	const static std::int16_t CLEINT_REQUEST_CANCEL_DEALER			= 1040;

	//���������ׯ
	const static std::int16_t SERVER_RESPONSE_CANCEL_DEALER		= 1040;

	//�����������
	const static std::int16_t CLEINT_REQUEST_SHOW_CARDS				= 1041;

	//�㲥�������
	const static std::int16_t SERVER_RESPONSE_SHOW_CARDS			= 1041;

	//�㲥��ʼѹע
	const static std::int16_t SERVER_BROADCAST_START_BET				= 1042;

	//�������ѹע
	const static std::int16_t CLEINT_REQUEST_BET								= 1043;

	//�㲥���ѹע
	const static std::int16_t SERVER_RESPONSE_BET							= 1043;

	//��Ϸ��ʼ��
	const static std::int16_t	SERVER_BROADCAST_NEW_GAME			= 1044;

	//����deal
	const static std::int16_t SERVER_BROADCAST_DEAL						= 1045;

	//SettleAccounts
	const static std::int16_t SERVER_BROADCAST_SETTLE_ACCOUNTS		= 1046;

	//������������������Ϣsnapshot
	const static std::int16_t SERVER_PUSH_TABLE_INFO						= 1047;

	const static std::int16_t CLEINT_ACK_SETTLE_ACCOUNTS					= 1048;

	const static std::int16_t SERVER_PUSH_SHOW_CARDS						= 1049;

	//�齫��ţ��ʾ�����ϵ���
	const static std::int16_t SERVER_PUSH_SHOW_TABLE_CARDS			= 1050;

	//������һ�����ƽ��
	const static std::int16_t SERVER_PUSH_LAST_ROUND_CARDS			= 1051;

	//random ������Ƶ���ʼλ��
	const static std::int16_t SERVER_PUSH_RANDOM								= 1052;

  const static std::int16_t SERVER_PLAY_MESSAGE = 1053;



    //���������뷿�䲢����
    const static std::int16_t KK_CLIENT_REQUEST_JOIN = 1101;

    //��������Ϸ��ʼ������
    const static std::int16_t KK_SERVER_PUSH_HOLD_CARDS = 1102;

    //��������ׯ
    const static std::int16_t KK_CLIENT_REQUEST_HAND_HOG = 1103;

    //�㲥ׯ��
    const static std::int16_t KK_SERVER_PUSH_DELAER = 1104;

    //ѡ����
    const static std::int16_t KK_CLIENT_REQUEST_DOUBLE = 1105;

    //��������������β��
    const static std::int16_t KK_SERVER_PUSH_CARDS = 1106;

    //����
    const static std::int16_t KK_CLIENT_REQUEST_SHOW_CARD = 1107;


    const static std::int16_t KK_SERVER_PUSH_SETTLE_ACCOUNTS = 1108;

    const static std::int16_t KK_SERVER_PUSH_TABLE_SNAPSHOT = 1109;

    //��������
    const static std::int16_t KK_SERVER_PUSH_RECONNECT_INFO = 1110;

    //��Ϸ������
    const static std::int16_t KK_SERVER_GAME_OVER = 1111;

	const static std::int16_t PHP_PEEK_BAIREN_CARDS = 1112;//͵�����˳�����

	const static std::int16_t KK_CLIENT_REQ_HINT = 1113;//��ʾ

	const static std::int16_t KK_RUN_IN_BACKGROUND = 1114;//�ƶ����е���̨����

	/************ը��ţ****BEGIN*****************/

	const static std::int16_t ZJN_GAME_START = 1125;//��Ϸ��ʼ

	const static std::int16_t ZJN_PUSH_NEXT_TAKER = 1126;//˭�Ļغ�

	const static std::int16_t ZJN_PLAYER_CHECK = 1127;//��ҿ���

	const static std::int16_t ZJN_PLAYER_FOLD = 1128;//�������

	const static std::int16_t ZJN_PLAYER_COMAPRE_CARDS = 1129;//��ұ���

	const static std::int16_t ZJN_PLAYER_BET = 1130;//�����ע

	const static std::int16_t ZJN_JOIN_GAME = 1131;//���뷿��

	const static std::int16_t ZJN_PUSH_TABLE_SNAPSHOT = 1132;//�������

	const static std::int16_t ZJN_PUSH_RECONNECT_INFO = 1133;//��������

	const static std::int16_t ZJN_PUSHPUSH_SETTLE_ACCOUNTS = 1134;//����

	const static std::int16_t ZJN_GAME_OVER = 1135;//��Ϸ����

	const static std::int16_t ZJN_SHOW_CARDS = 1136;//����

	/************ը��ţ****END*****************/


    /************����ϵͳ****BEGIN*****************/

    //����������������Ϣ
    const static std::int16_t SERVER_PUSH_TASK_RECORD = 1201;

    //��������֪ͨ�������
    const static std::int16_t SERVER_PUSH_TASK_COMPLETE = 1202;

    const static std::int16_t PHP_REQUEST_TASK_AWARD = 1203;

    const static std::int16_t PHP_REQUEST_TASK_INCR = 1204;

	/************����ϵͳ****END*****************/



	/******************��ӿ�*BEGIN******************/


	//ʥ������
	const static std::int16_t SERVER_PUSH_DROP = 1301;

	//������Ϣ
	const static std::int16_t SERVER_PUSH_ROUND_POINT = 1302;

	//�����콱
	const static std::int16_t SERVER_PUSH_ROUND_POINT_AWARD = 1303;

	const static std::int16_t SERVER_PUSH_ROUND_AWARD = 1304;


	/******************��ӿ�*END******************/
		
  //��������
  const static std::int16_t CLEINT_REQUEST_CREATE_ROOM = 1996;

  //��������
  const static std::int16_t SERVER_RESPONSE_CREATE_ROOM = 1996;

	//������Ϸ
	const static std::int16_t CLEINT_REQUEST_JOIN_ROOM				 = 1997;

	//������Ϸ
	const static std::int16_t SERVER_RESPONSE_JOIN_ROOM				 = 1997;

	//�����������ر�
	const static std::int16_t SERVER_PUSH_SERVERS_STOPPED			 = 1998;

	//���ӶϿ���
	const static std::int16_t GATEWAY_EVENT_CONNECT_CLOSE		= 1999;

	//������ҵ���ǰ���ڷ���
	const static std::int16_t SERVER_REQUEST_RECONNECT				= 1999;

	//�ͻ�����������
	const static std::int16_t CLIENT_REQUEST_CHAT_MESSAGE			= 2001;

	//������ת
	const static std::int16_t SERVER_RESPONSE_CHAT_MESSAGE		= 2001;

	//�㲥��Ϣ
	const static std::int16_t SERVER_BROADCAST_MSG						= 2002;

    //ϵͳ���
    const static std::int16_t SERVER_SYSTEM_DISABLE_PLAYER = 2003;


	//***************************************
	//red packet �����
	const static std::int16_t AS_REQUEST_GIFT_RED_PACKET = 2011;

	const static std::int16_t  SERVER_BROADCAST_GIFT_RED_PACKET = 2012;

	//����
	const static std::int16_t AS_REQUEST_GET_RED_PACKET = 2013;

	//ϵͳ����δ������Ϣ�Ϳ��õ�ϵͳ�����Ϣ
	const static std::int16_t SERVER_PUSH_RED_PACKET = 2014;

	//ϵͳ�㲥�����ȡ
	const static std::int16_t SERVER_BROADCAST_GET_RED_PACKET = 2015;

	//ϵͳ�㲥��ȡϵͳ���
	const static std::int16_t SERVER_BROADCAST_GET_SYSTEM_RED_PACKET = 2018;

    //������
    const static std::int16_t CLIENT_REQUEST_CHANGE_NAME = 2019;


	//**************************************************************************

	//�ͻ��������������� 
	const static std::int16_t CLIENT_REQUEST_PRESENT_GIFT				= 3001;

	//����������Ӧ��������
	const static std::int16_t SERVER_RESPONSE_PRESENT_GIFT			= 3001;

	//�ͻ����������ͽ��
	const static std::int16_t CLIENT_REQUEST_PRESENT_GOLD			= 3002;

	//����������Ӧ���ͽ��
	const static std::int16_t SERVER_RESPONSE_PRESENT_GOLD		= 3002;

	//����property
	const static std::int16_t CLEINT_REQUEST_BUG_PROPERTY			= 3003;

	//��Ӧ��������
	const static std::int16_t SERVER_RESPONSE_BUG_PROPERTY		= 3003;

	//�û�ʹ�õ���
	const static std::int16_t CLEINT_REQUEST_USE_PROPERTY			= 3006;

	//������Ӧʹ�õ���
	const static std::int16_t SERVER_RESPONSE_USE_PROPERTY			= 3006;

	//�㲥�۱��轱�ؽ��
	const static std::int16_t SERVER_BROADCAST_AWARD_POOL		= 3007;

	//notify
	const static std::int16_t SERVER_NOTIFY_HONOR_COMPLETE		= 3008;

	//����ˢ�����а�RefreshRanking
	const static std::int16_t CLIENT_REQUES_REFRESH_RANKING		= 3009;

	//����ˢ�����а�RefreshRanking
	const static std::int16_t SERVER_RESPONSE_REFRESH_RANKING	= 3009;

	//ȡ������װ������
	const static std::int16_t CLIENT_REQUES_CANCEL_TABLE_PROP		= 3015;

	const static std::int16_t SERVER_RESPONSE_CANCEL_TABLE_PROP	= 3015;

    //��ȡ������ڷ���
    const static short CLEINT_REQUEST_GET_PLAYER_ROOM = 3016;

    const static short SERVER_RESPONSE_GET_PLAYER_ROOM = 3016;


	//����ˢ��Ӯ�����а�
	const static std::int16_t SERVER_RESPONSE_REFRESH_WIN_POINT_RANKING	= 3020;

	const static std::int16_t SERVER_PUSH_GIFT_PROPS						= 3028;

	//�󳡽���֪ͨ
	const static std::int16_t SERVER_PUSH_ENTER_NOTICE					= 3031;

	const static std::int16_t SERVER_SEND_EMAIL = 3033;//�������ʼ�

	
	//*******************************Begin.�̳�����Ϸ������֮���·�ɰ�*************************//

	const static std::int16_t STORE_ROUTE_USE_PROPS						= 4001;

	//��Ϸ����������ͬ�����ء� ���ӿ�������
	const static std::int16_t CLIENT_REQUEST_SYNC_AWARD_POOL			= 4002;

	//���ӿ�������
	const static std::int16_t SERVER_RESPONSE_SYNC_AWARD_POOL		= 4002;

	//*******************************End. �̳�����Ϸ������֮���·�ɰ�*************************//
	
	namespace Log
	{
		const static std::int16_t XLOGGER_GAME_TRACE						= 9001;

		const static std::int16_t XLOGGER_WRITE_AWARD_RECORD		= 9002;

		const static std::int16_t XLOGGER_WRITE_PLAYER_RECORD		= 9003;

		const static std::int16_t XLOGGER_WRITE_ROOM_RECORD		= 9004;

		const static std::int16_t XLOGGER_WRITE_BANK_RECORD			= 9005;

		//�ϱ���������
		const static std::int16_t XLOGGER_UPDATE_ONLINE_DATA			= 9006;

		//�ϱ���������
		const static std::int16_t XLOGGER_UPDATE_PLAYING_DATA			= 9007;

		//�ϱ���ˮ����
		const static std::int16_t XLOGGER_UPDATE_TAXATION_DATA		= 9008;

		//�ϱ�ͳ������statistics
		const static std::int16_t XLOGGER_UPDATE_STATISTICS_DATA		= 9009;

		const static std::int16_t XLOGGER_WRITE_GOLD_LOG								= 9011;

		const static std::int16_t XLOGGER_WRITE_ROTOT_GOLD_INCR_LOG		= 9012;

		const static std::int16_t XLOGGER_WRITE_RED_PACKET_LOG = 9020;

		const static std::int16_t XLOGGER_WRITE_GET_RED_PACKET_LOG = 9021;

		const static std::int16_t XLOGGER_WRITE_TAXES_RECORD = 9022;			//ÿ��ÿ�³�ˮ��Ϣ
	}
}

namespace BaiRen
{  
	//���������ׯ
	const static std::int16_t CLIENT_REQUEST_HAND_HOG		= 1053;

	//�㲥�����ׯ
	const static std::int16_t SERVER_RESPONSE_HAND_HOG	= 1053;

	//���������ׯ
	const static std::int16_t CLIENT_REQUEST_CANCEL_HAND_HOG		= 1054;

	//�㲥�����ׯ
	const static std::int16_t SERVER_RESPONSE_CANCEL_HAND_HOG	= 1054;

	//�㲥��ע������
	const static std::int16_t SERVER_PULL_BET_LIMITED								= 1055;

	//�㲥��ʼ��ע
	const static std::int16_t SERVER_PULL_START_BET								= 1056;

	//���������ע
	const static std::int16_t CLIENT_REQUEST_BET										= 1057;

	//�㲥�����ע
	const static std::int16_t SERVER_RESPONSE_BET									= 1057;

	//deal �㲥��ʼ����
	const static std::int16_t SERVER_PULL_START_DEAL							= 1058;

	//���Ӽ�¼
	const static std::int16_t SERVER_PULL_ROUND_RECOREDS				= 1059;

	const static std::int16_t SERVER_PULL_HANDHOG_RECORDS			= 1060;

	//�㲥����
	const static std::int16_t SERVER_PULL_SETTLE_ACCOUNTS				= 1061;

	//
	const static std::int16_t CLIENT_REQUEST_RAND								= 1062;

	const static std::int16_t SERVER_RESPONSE_RAND								= 1062;

	//�������vip����
	const static std::int16_t CLIENT_REQUEST_VIP_SITDOWN = 1901;

	//�㲥vip����
	const static std::int16_t CLIENT_REQUEST_VIP_SITUP = 1902;

	//�㲥vip �����б�
	const static std::int16_t SERVER_PULL_VIP_SEATS_LIST = 1903;

};

namespace RunFast
{
	//�������㲥�����
	const static std::int16_t SERVER_BROADCAST_LAIZI_CARD = 7001;

	//�������㲥��Ӵ������
	const static std::int16_t SERVER_BROADCAST_PLAYED_CARD_LAIZI = 7002;
	 
	//�ͻ����������ƥ�����ͳ���
	const static std::int16_t CLIENT_REQUEST_PLAYED_LAIZI_MATCH_TYPE = 7003;
	//�������Ӧ���ƥ�����ͳ�������
	const static std::int16_t SERVER_RESPONSE_PLAYED_LAIZI_MATCH_TYPE = 7003;

	//��������֪ͨ�ͻ��������
	const static std::int16_t SERVER_NET_RECONNECT_NOTIFY_CLIENT_LAIZI = 7004;

    //ǿ�ƽ�����Ϸ
    const static std::int16_t SERVER_BROADCAST_FORCE_GAMEOVER = 1066;
    //ͶƱ��ɢ����
    const static std::int16_t CLIENT_REQUES_DISBAND_VOTE = 1067;
    const static std::int16_t SERVER_BROADCAST_DISBAND_VOTE = 1067;
    //������ʼ
    const static std::int16_t SERVER_BROAD_MATCH_START = 1068;
    //֪ͨ�׳���
    const static std::int16_t SERVER_NOTIFY_FIRST_PLAY = 1069;
   //֪ͨ�����ɢ
    const static std::int16_t SERVER_NOTIFY_DISBAND = 1070;
  //�㲥��һ��������
  const static std::int16_t SERVER_BROADCAST_NEXT_PLAYER = 1071;

  //����
  const static std::int16_t CLIENT_REQUEST_PLAY = 1072;

  const static std::int16_t SERVER_RESPONSE_PLAY = 1072;

  //�㲥�������
  const static std::int16_t SERVER_BROADCAST_PLAYED_CARD = 1073;

  const static std::int16_t SERVER_BROADCAST_GAMEOVER = 1074;

  //�й�
  const static std::int16_t CLIEN_REQUEST_TUOGUAN = 1075;
  const static std::int16_t SERVER_RESPONSE_TUOGUAN = 1075;

  //����
  const static std::int16_t CLIENT_REQUEST_ZANLI = 1076;
  const static std::int16_t SERVER_RESPONSE_ZANLI = 1076;

  //��ɢ����
  const static std::int16_t CLIENT_REQUEST_DISBAND = 1077;
  const static std::int16_t SERVER_RESPONSE_DISBAND = 1077;

  //�㲥�������
  const static std::int16_t SERVER_BROADCAST_TABLE_SNAPSHOT = 1078;

  //ը������
  const static std::int16_t SERVER_BROADCAST_BOMB_ACCOUNTS = 1079;

  //�������
  const static std::int16_t SERVER_BROADCAST_ROOM_ACCOUNTS = 1080;
  //�·������
  const static std::int16_t SERVER_BROADCAST_ROOM_ACCOUNTS_EX = 8080;

  //�ӽ��
  const static std::int16_t SERVER_UPDATE_GLOP = 1081;

  //���ֽ���
  const static std::int16_t SERVER_BROADCAST_ACCOUNTS = 1082;
  //�µ��ֽ���
  const static std::int16_t SERVER_BROADCAST_ACCOUNTS_EX = 8082;

  //���ߺ��ط�����
  const static std::int16_t SERVER_NET_RECONNECT_HANDCARDS = 1083;

  //������Ϣ
  const static std::int16_t CLIENT_SEND_MESSAGE = 1084;
  //�㲥��Ϣ
  const static std::int16_t SERVER_BROADCAST_MESSAGE = 1084;

  const static std::int16_t CLINET_ADD_ROBOT = 1085;
  const static std::int16_t SERVER_ADD_ROBOT = 1085;

  const static std::int16_t SERVER__UPDATE_GOLD = 1086;

  //�����������
  const static std::int16_t CLIENT_ENTER_MATCH_HALL = 1087;
  const static std::int16_t SERVER_ENTER_MATCH_HALL = 1087;

  //�뿪��������
  const static std::int16_t CLIENT_LEVAE_MATCH_HALL = 1088;
  const static std::int16_t SERVER_LEVAE_MATCH_HALL = 1088;

  //����
  const static std::int16_t CLIENT_CHECK_IN = 1089;
  const static std::int16_t SERVER_CHECK_IN = 1089;

  //ȡ������
  const static std::int16_t CLIENT_CHECK_OUT = 1090;
  const static std::int16_t SERVER_CHECK_OUT = 1090;

  //�ȴ�����������ɱ���
  const static std::int16_t SERVER_MATCH_WAITING = 1091;

  //��������
  const static std::int16_t SERVER_MATCH_RANKING = 1092;

  //������̭
  const static std::int16_t SERVER_MATCH_ELIMINATE_PAYER = 1093;

  //�����佱
  const static std::int16_t SERVER_MATCH_PRIZE = 1094;

  //��������
  const static std::int16_t SERVER_MATCH_RANK = 1095;

  //��������
  const static std::int16_t SERVER_MATCH_PUSH_MESSAGE= 1096;

  //�㲥����״̬
  const static std::int16_t SERVER_BROAD_PLAYGROUND_STATUS = 1097;

  const static std::int16_t SERVER_BROAD_NOW_RANK = 1098;

  

  //������Ϸ
  const static std::int16_t CLIENT_FIND_Z_ROOM = 1099;
  const static std::int16_t SERVER_FIND_Z_ROOM = 1099;

  //������Ϸ
  const static std::int16_t CLIENT_CHANGE_Z_ROOM = 1100;
  const static std::int16_t SERVER_CHANGE_Z_ROOM = 1100;

  const static std::int16_t SERVER_NOTIFY_GLOD_LIMIT = 1101;

  const static std::int16_t SERVER_NOTIFY_MSG_LIST = 1102;

  const static std::int16_t SERVER_VERSION_MESSAGE = 9999;

  namespace ErrorCode
  {
    static const std::int32_t ERROR_DISBAND_MUST_BE_OWNNER = -1001; //ֻ�з������ܽ�ɢ
    static const std::int32_t ERROR_DISBAND_ANOTHER_PLAYER = -1002;//ֻʣ����ʱ���ܽ�ɢ


    static const std::int32_t ERROR_ROOM_COUNT_NULL = -1020;//��Ϸ��ʼʧ�ܣ�����ʣ�����Ϊ0

    static const std::int32_t ERROR_ROOM_ID_ERROR= -3;//����Ŵ���

    static const std::int32_t ERROR_MATCH_CLOSED = -1010; //�����ѹر�
    static const std::int32_t ERROR_MATCH_PLAYING = -1011; //�����ѿ�ʼ
    static const std::int32_t ERROR_MATCH_FULL = -1012; //������������
    static const std::int32_t ERROR_MATCH_NOT_FIND = -1013; //����δ�ҵ�
    static const std::int32_t ERROR_MATCH_READY_CHECK = -1014; //�����ظ�����
    static const std::int32_t ERROR_MATCH_CHECK_COST_FAILED = -1015; //�������ò���

    static const std::int32_t ERROR_ENTER_MATCH_HAS_ROOM = -1016; //����������е���Ϸ

    static const std::int32_t ERROR_CHECK_HAS_NO_MUCH_PLAYGROUND = -1017; //��������

    static const std::int32_t ERROR_MATCH_SERVER_CLOSED = -1018; //����δ����

    static const std::int32_t ERROR_GLOD_ENTER_LIMIT = -1020; //�������������
    static const std::int32_t ERROR_Z_ROOM_PLAYING = -1021; //������Ϸ��
  };

  namespace PHP
  {
    const static std::int16_t PHP_INCR_GLOP = 1001;
  }

  namespace Log
  {
    static const std::int16_t XLOGGER_GAME_RECORD = 9031;
    static const std::int16_t XLOGGER_GOLD_RECORD = 9032;
    const static std::int16_t XLOGGER_ROOM_RECORD = 9033;
    const static std::int16_t XLOGGER_PLAYING_RECORD = 9034;
    const static std::int16_t XLOGGER_PLAYER_RECORD = 9035;
    const static std::int16_t XLOGGER_GAME_RECORD_SUB = 9037;
    const static std::int16_t XLOGGER_PLAYER_ONLINE = 9040;
  }
};

#endif

