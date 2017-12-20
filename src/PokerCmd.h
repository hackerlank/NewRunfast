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

    //系统管理指令 ｛0， 关闭服务器， 1， 重启加载配置文件, 2 服务器即将关闭｝
    const static std::int16_t SYSTEM_ADMINI_CMD = 10004;

    //系统广播
    const static std::int16_t SYSTEM_BROADCAST = 10005;

    //心跳请求
    const static std::int16_t HEARTBEAT_PACKET = 10006;

    //converge
    const static std::int16_t CONVERGE_PACKET = 10007;

    //用于主从之间的通迅
    const static std::int16_t REQUEST_SYNC = 10008;

    const std::int32_t	ADMINI_CMD_CLOSE_SERVER = 0;
    const std::int32_t	ADMINI_CMD_RELOAD = 1;
    const std::int32_t	ADMINI_CMD_STOP_SERVER = 2;

    static const std::int32_t	SESSION_TYPE_BEGIN = 0;
    static const std::int32_t	SESSION_TYPE_CLINET = 1;			//客户端连接
    static const std::int32_t	SESSION_TYPE_SERVER = 2;			//服务器	
    static const std::int32_t	SESSION_TYPE_END = 3;

    static const std::int32_t REGISTER_SUCCESS = 0;

    static const std::int32_t REGISTER_FAILED = -1;

    //用户请求登陆
    const static std::int32_t CLIENT_REQUEST_LOGIN = 1000;

    const static std::int32_t SERVER_RESPONSE_LOGIN = 1000;
};


namespace Texas
{
	namespace error_code
	{
        const static std::int32_t ERROR_SUCCEED = 0;
		//房间已满
		const static std::int32_t ERROR_ENTER_ROOM_FILLED						= -1;

		const static std::int32_t ERROR_ENTER_ROOM_NOT_FIND_ROOM	= -2;

		//blacklisk
		const static std::int32_t ERROR_ENTER_ROOM_BLACKLIST				= -3;
			
		//参数无效
		const static std::int32_t ERROR_SITDOWN_ARG_INVAILD					= -3;
		//座位非空
		const static std::int32_t ERROR_SITDOWN_SEAT_NOT_NULL			= -4;

		//enough 练习币不足
		const static std::int32_t ERROR_COIN_NOT_ENOUGH						= -5;
		//金币不足
		const static std::int32_t ERROR_GOLD_NOT_ENOUGH						= -6; 

		//参数无效
		const static std::int32_t ERROR_ARG_INVAILD									= -7;

		//服务器即将关闭
		const static std::int32_t ERROR_SERVER_CLOSED								= -8;

		//坐下失败，来自同一IP
		const static std::int32_t ERROR_SITDOWN_FROM_SAME_IP				= -10;

        static const std::int32_t ERROR_WRITE_REDIS_ERROR = -11;

        static const std::int32_t ERROR_READ_REDIS_ERROR = -12;

		static const std::int32_t ERROR_UNKNOWN = -13;

		static const std::int32_t ERROR_ISNULL = -14;

		static const std::int32_t ERROR_KICK_LONG_TIME_NO_OPERATION = -15;//踢人：长时间未操作

		static const std::int32_t ERROR_KICK_GOLD_BELOW_FLOOR = -16;//踢人：金币低于房间下限

		static const std::int32_t ERROR_BET_VALUE_ILLEGAL = -17;//下注金额非法

		static const std::int32_t ERROR_KICK_NOT_READY = -18;//踢人：没点准备

		static const std::int32_t ERROR_KICK_SYS_KICK = -19;//踢人：系统踢人

		static const std::int32_t ERROR_KICK_OFF_LINE = -20;//踢人：离线

		static const std::int32_t ERROR_STILL_IN_GAME = -21;//还在游戏中，不能换房

		static const std::int32_t ERROR_CAHNGE_ROOM = -22;//换房

		static const std::int32_t ERROR_SEAT_FULL = -23;//座位已满

        static const std::int32_t   REGISTER_SUCCESS = 0;			//登陆成功
        static const std::int32_t   REGISTER_FAILED = 1;		//登陆失败, 未知错误.
        static const std::int32_t   REGISTER_KEY_ERROR = 2;			//会话KEY校验失败
        static const std::int32_t   REGISTER_RELOGIN = 3;			//重复登陆, 只会发给前一个连接.
	}

	//用户请求登陆
	const static std::int16_t CLIENT_REQUEST_LOGIN							= 1000;

	//用户请求进入房间
	const static std::int16_t CLIENT_REQUEST_ENTER_ROOM				= 1001;

	//服务器端响应进入房间的请求response
	//-1, 房间不存在，0 ， 成功了
	const static std::int16_t SERVER_RESPONSE_ENTER_ROOM			= 1001;

	//服务器端推送房间玩家信息snapshot
	const static std::int16_t SERVER_PUSH_PLAYERS_SNAPSHOT		= 1002;

	//服务器端推送桌面信息snapshot
	const static std::int16_t SERVER_PUSH_TABLE_SNAPSHOT			= 1003;

	//用户请求离开房间
	const static std::int16_t CLIENT_REQUEST_LEAVE_ROOM				= 1004;

	//服务器端响离开房间的请求response
	const static std::int16_t SERVER_RESPONSE_LEAVE_ROOM			= 1004;

	//服务器端广播进入房间的玩家信息
	const static std::int16_t SERVER_BROADCAST_PLAYER_INFO			= 1005;

	//用户请求坐下
	const static std::int16_t CLIENT_REQUEST_SITDOWN					= 1006;

	//服务器端响应坐下请求response
	const static std::int16_t SERVER_RESPONSE_SITDOWN					= 1006;

	//用户请求站起
	const static std::int16_t CLIENT_REQUEST_SITUP							= 1007;

	//服务器端响应站起请求response
	const static std::int16_t SERVER_RESPONSE_SITUP						= 1007;

	//服务器广播游戏开始
	const static std::int16_t SERVER_BROADCAST_GAMESTART			= 1008;

	//服务器广播转到谁下注了
	const static std::int16_t SERVER_PUSH_NEXT_TAKER						= 1009;

	//Flop
	const static std::int16_t SERVER_BROADCAST_FLOP						= 1010;	

#if 0
	//TURN
	const static std::int16_t SERVER_BROADCAST_TRUN						= 1011;

	//RIVER
	const static std::int16_t SERVER_BROADCAST_RIVER						= 1012;
#endif //0

	//用户下注
	const static std::int16_t CLIENT_REQUEST_BETTING						= 1013;

	//服务器广播下注
	const static std::int16_t SERVER_BROADCAST_BETTING					= 1013;

	//亮牌
	const static std::int16_t SERVER_BROADCAST_RESULT					= 1023;

	//当前回合池底
	const static std::int16_t SERVER_BROADCAST_POTS						= 1024;

	//奖池分配
	const static std::int16_t SERVER_BROADCAST_POTS_MALLOC		= 1025;

	//广播游戏结束
	const static std::int16_t SERVER_BROADCAST_GAMEOVER			= 1026;

	//用户购买筹码
	const static std::int16_t CLIENT_REQUES_BUY_CHIPS					= 1027;

	//服务器响应购买筹码response
	const static std::int16_t SERVER_RESPONSE_BUY_CHIPS				= 1027;

	//服务器广播中奖的消息
	const static std::int16_t SERVER_BROADCAST_AWARD					= 1029;

	//赠送筹码
	const static std::int16_t CLIENT_REQUEST_GIFT_CHIPS					= 1030;

	//赠送筹码
	const static std::int16_t SERVER_RESPONSE_GIFT_CHIPS				= 1030;

	//赠送练习币
	const static std::int16_t SERVER_LEARNER_GIFT_CHIPS					= 1031;

	//使用表情
	const static std::int16_t CLIENT_REQUEST_USE_FACE					= 1032;

	//使用表情
	const static std::int16_t SERVER_RESPONSE_USE_FACE					= 1032;

	//添加朋友
	const static std::int16_t CLEINT_REQUEST_ADD_FRIEND				= 1033;

	//广播加为好友
	const static std::int16_t SERVER_BROADCAST_ADD_FRIEND			= 1033;

	//用户回应申请加为好友
	const static std::int16_t CLEINT_RESPONSE_ADD_FRIEND				= 1034;

	//广播好友申请结果
	const static std::int16_t SERVER_RESPONSE_ADD_FRIEND			= 1034;

	const static std::int16_t SERVER_PUSH_HOLD_CARDS					= 1035;

	//返回用户在玩时间（坐下开始计时）
	const static std::int16_t SERVER_PUSH_PLAYER_PLAYING_TIME	= 1036;	

	//rob dealer广播开始抢庄
	const static std::int16_t SERVER_PUSH_ROB_DEALER						= 1037;

	//玩家请求抢庄
	const static std::int16_t CLEINT_RESPONSE_ROB_DEALER				= 1038;

	//玩家请求抢庄
	const static std::int16_t SERVER_RESPONSE_ROB_DEALER				= 1038;

	//rob dealer广播庄家
	const static std::int16_t SERVER_PUSH_DEALER								= 1039;

	//玩家请求弃庄
	const static std::int16_t CLEINT_REQUEST_CANCEL_DEALER			= 1040;

	//玩家请求弃庄
	const static std::int16_t SERVER_RESPONSE_CANCEL_DEALER		= 1040;

	//玩家请求亮牌
	const static std::int16_t CLEINT_REQUEST_SHOW_CARDS				= 1041;

	//广播玩家亮牌
	const static std::int16_t SERVER_RESPONSE_SHOW_CARDS			= 1041;

	//广播开始压注
	const static std::int16_t SERVER_BROADCAST_START_BET				= 1042;

	//玩家请求压注
	const static std::int16_t CLEINT_REQUEST_BET								= 1043;

	//广播玩家压注
	const static std::int16_t SERVER_RESPONSE_BET							= 1043;

	//游戏开始了
	const static std::int16_t	SERVER_BROADCAST_NEW_GAME			= 1044;

	//发牌deal
	const static std::int16_t SERVER_BROADCAST_DEAL						= 1045;

	//SettleAccounts
	const static std::int16_t SERVER_BROADCAST_SETTLE_ACCOUNTS		= 1046;

	//服务器端推送桌面信息snapshot
	const static std::int16_t SERVER_PUSH_TABLE_INFO						= 1047;

	const static std::int16_t CLEINT_ACK_SETTLE_ACCOUNTS					= 1048;

	const static std::int16_t SERVER_PUSH_SHOW_CARDS						= 1049;

	//麻将斗牛显示桌面上的牌
	const static std::int16_t SERVER_PUSH_SHOW_TABLE_CARDS			= 1050;

	//推送上一局亮牌结果
	const static std::int16_t SERVER_PUSH_LAST_ROUND_CARDS			= 1051;

	//random 随机发牌的启始位置
	const static std::int16_t SERVER_PUSH_RANDOM								= 1052;

  const static std::int16_t SERVER_PLAY_MESSAGE = 1053;



    //看看场进入房间并坐下
    const static std::int16_t KK_CLIENT_REQUEST_JOIN = 1101;

    //看看场游戏开始并发牌
    const static std::int16_t KK_SERVER_PUSH_HOLD_CARDS = 1102;

    //看看场抢庄
    const static std::int16_t KK_CLIENT_REQUEST_HAND_HOG = 1103;

    //广播庄家
    const static std::int16_t KK_SERVER_PUSH_DELAER = 1104;

    //选择倍数
    const static std::int16_t KK_CLIENT_REQUEST_DOUBLE = 1105;

    //服务器端推送推尾牌
    const static std::int16_t KK_SERVER_PUSH_CARDS = 1106;

    //亮牌
    const static std::int16_t KK_CLIENT_REQUEST_SHOW_CARD = 1107;


    const static std::int16_t KK_SERVER_PUSH_SETTLE_ACCOUNTS = 1108;

    const static std::int16_t KK_SERVER_PUSH_TABLE_SNAPSHOT = 1109;

    //断线重连
    const static std::int16_t KK_SERVER_PUSH_RECONNECT_INFO = 1110;

    //游戏结束了
    const static std::int16_t KK_SERVER_GAME_OVER = 1111;

	const static std::int16_t PHP_PEEK_BAIREN_CARDS = 1112;//偷窥百人场牌型

	const static std::int16_t KK_CLIENT_REQ_HINT = 1113;//提示

	const static std::int16_t KK_RUN_IN_BACKGROUND = 1114;//移动端切到后台运行

	/************炸金牛****BEGIN*****************/

	const static std::int16_t ZJN_GAME_START = 1125;//游戏开始

	const static std::int16_t ZJN_PUSH_NEXT_TAKER = 1126;//谁的回合

	const static std::int16_t ZJN_PLAYER_CHECK = 1127;//玩家看牌

	const static std::int16_t ZJN_PLAYER_FOLD = 1128;//玩家弃牌

	const static std::int16_t ZJN_PLAYER_COMAPRE_CARDS = 1129;//玩家比牌

	const static std::int16_t ZJN_PLAYER_BET = 1130;//玩家下注

	const static std::int16_t ZJN_JOIN_GAME = 1131;//进入房间

	const static std::int16_t ZJN_PUSH_TABLE_SNAPSHOT = 1132;//桌面快照

	const static std::int16_t ZJN_PUSH_RECONNECT_INFO = 1133;//断线重连

	const static std::int16_t ZJN_PUSHPUSH_SETTLE_ACCOUNTS = 1134;//结算

	const static std::int16_t ZJN_GAME_OVER = 1135;//游戏结束

	const static std::int16_t ZJN_SHOW_CARDS = 1136;//亮牌

	/************炸金牛****END*****************/


    /************任务系统****BEGIN*****************/

    //服务器推送任务信息
    const static std::int16_t SERVER_PUSH_TASK_RECORD = 1201;

    //服务器端通知任务完成
    const static std::int16_t SERVER_PUSH_TASK_COMPLETE = 1202;

    const static std::int16_t PHP_REQUEST_TASK_AWARD = 1203;

    const static std::int16_t PHP_REQUEST_TASK_INCR = 1204;

	/************任务系统****END*****************/



	/******************活动接口*BEGIN******************/


	//圣诞掉落活动
	const static std::int16_t SERVER_PUSH_DROP = 1301;

	//积分消息
	const static std::int16_t SERVER_PUSH_ROUND_POINT = 1302;

	//积分领奖
	const static std::int16_t SERVER_PUSH_ROUND_POINT_AWARD = 1303;

	const static std::int16_t SERVER_PUSH_ROUND_AWARD = 1304;


	/******************活动接口*END******************/
		
  //创建房间
  const static std::int16_t CLEINT_REQUEST_CREATE_ROOM = 1996;

  //创建房间
  const static std::int16_t SERVER_RESPONSE_CREATE_ROOM = 1996;

	//快速游戏
	const static std::int16_t CLEINT_REQUEST_JOIN_ROOM				 = 1997;

	//快速游戏
	const static std::int16_t SERVER_RESPONSE_JOIN_ROOM				 = 1997;

	//服务器即将关闭
	const static std::int16_t SERVER_PUSH_SERVERS_STOPPED			 = 1998;

	//连接断开了
	const static std::int16_t GATEWAY_EVENT_CONNECT_CLOSE		= 1999;

	//返回玩家掉线前所在房间
	const static std::int16_t SERVER_REQUEST_RECONNECT				= 1999;

	//客户端请求聊天
	const static std::int16_t CLIENT_REQUEST_CHAT_MESSAGE			= 2001;

	//服务器转
	const static std::int16_t SERVER_RESPONSE_CHAT_MESSAGE		= 2001;

	//广播消息
	const static std::int16_t SERVER_BROADCAST_MSG						= 2002;

    //系统封号
    const static std::int16_t SERVER_SYSTEM_DISABLE_PLAYER = 2003;


	//***************************************
	//red packet 发红包
	const static std::int16_t AS_REQUEST_GIFT_RED_PACKET = 2011;

	const static std::int16_t  SERVER_BROADCAST_GIFT_RED_PACKET = 2012;

	//领红包
	const static std::int16_t AS_REQUEST_GET_RED_PACKET = 2013;

	//系统推送未领红包信息和可用的系统红包信息
	const static std::int16_t SERVER_PUSH_RED_PACKET = 2014;

	//系统广播红包领取
	const static std::int16_t SERVER_BROADCAST_GET_RED_PACKET = 2015;

	//系统广播领取系统红包
	const static std::int16_t SERVER_BROADCAST_GET_SYSTEM_RED_PACKET = 2018;

    //改名卡
    const static std::int16_t CLIENT_REQUEST_CHANGE_NAME = 2019;


	//**************************************************************************

	//客户端请求赠送礼物 
	const static std::int16_t CLIENT_REQUEST_PRESENT_GIFT				= 3001;

	//服务器端响应赠送礼物
	const static std::int16_t SERVER_RESPONSE_PRESENT_GIFT			= 3001;

	//客户端请求赠送金币
	const static std::int16_t CLIENT_REQUEST_PRESENT_GOLD			= 3002;

	//服务器端响应赠送金币
	const static std::int16_t SERVER_RESPONSE_PRESENT_GOLD		= 3002;

	//购买property
	const static std::int16_t CLEINT_REQUEST_BUG_PROPERTY			= 3003;

	//响应购买请求
	const static std::int16_t SERVER_RESPONSE_BUG_PROPERTY		= 3003;

	//用户使用道具
	const static std::int16_t CLEINT_REQUEST_USE_PROPERTY			= 3006;

	//服务响应使用道具
	const static std::int16_t SERVER_RESPONSE_USE_PROPERTY			= 3006;

	//广播聚宝盆奖池金额
	const static std::int16_t SERVER_BROADCAST_AWARD_POOL		= 3007;

	//notify
	const static std::int16_t SERVER_NOTIFY_HONOR_COMPLETE		= 3008;

	//请求刷新排列榜RefreshRanking
	const static std::int16_t CLIENT_REQUES_REFRESH_RANKING		= 3009;

	//请求刷新排列榜RefreshRanking
	const static std::int16_t SERVER_RESPONSE_REFRESH_RANKING	= 3009;

	//取消桌面装饰礼物
	const static std::int16_t CLIENT_REQUES_CANCEL_TABLE_PROP		= 3015;

	const static std::int16_t SERVER_RESPONSE_CANCEL_TABLE_PROP	= 3015;

    //获取玩家所在房间
    const static short CLEINT_REQUEST_GET_PLAYER_ROOM = 3016;

    const static short SERVER_RESPONSE_GET_PLAYER_ROOM = 3016;


	//请求刷新赢点排行榜
	const static std::int16_t SERVER_RESPONSE_REFRESH_WIN_POINT_RANKING	= 3020;

	const static std::int16_t SERVER_PUSH_GIFT_PROPS						= 3028;

	//大场进入通知
	const static std::int16_t SERVER_PUSH_ENTER_NOTICE					= 3031;

	const static std::int16_t SERVER_SEND_EMAIL = 3033;//发送新邮件

	
	//*******************************Begin.商城与游戏服务器之间的路由包*************************//

	const static std::int16_t STORE_ROUTE_USE_PROPS						= 4001;

	//游戏服务器请求同步奖池。 本接口已作废
	const static std::int16_t CLIENT_REQUEST_SYNC_AWARD_POOL			= 4002;

	//本接口已作废
	const static std::int16_t SERVER_RESPONSE_SYNC_AWARD_POOL		= 4002;

	//*******************************End. 商城与游戏服务器之间的路由包*************************//
	
	namespace Log
	{
		const static std::int16_t XLOGGER_GAME_TRACE						= 9001;

		const static std::int16_t XLOGGER_WRITE_AWARD_RECORD		= 9002;

		const static std::int16_t XLOGGER_WRITE_PLAYER_RECORD		= 9003;

		const static std::int16_t XLOGGER_WRITE_ROOM_RECORD		= 9004;

		const static std::int16_t XLOGGER_WRITE_BANK_RECORD			= 9005;

		//上报在线数据
		const static std::int16_t XLOGGER_UPDATE_ONLINE_DATA			= 9006;

		//上报在玩数据
		const static std::int16_t XLOGGER_UPDATE_PLAYING_DATA			= 9007;

		//上报抽水数据
		const static std::int16_t XLOGGER_UPDATE_TAXATION_DATA		= 9008;

		//上报统计数据statistics
		const static std::int16_t XLOGGER_UPDATE_STATISTICS_DATA		= 9009;

		const static std::int16_t XLOGGER_WRITE_GOLD_LOG								= 9011;

		const static std::int16_t XLOGGER_WRITE_ROTOT_GOLD_INCR_LOG		= 9012;

		const static std::int16_t XLOGGER_WRITE_RED_PACKET_LOG = 9020;

		const static std::int16_t XLOGGER_WRITE_GET_RED_PACKET_LOG = 9021;

		const static std::int16_t XLOGGER_WRITE_TAXES_RECORD = 9022;			//每人每月抽水信息
	}
}

namespace BaiRen
{  
	//玩家请求抢庄
	const static std::int16_t CLIENT_REQUEST_HAND_HOG		= 1053;

	//广播玩家抢庄
	const static std::int16_t SERVER_RESPONSE_HAND_HOG	= 1053;

	//玩家请求下庄
	const static std::int16_t CLIENT_REQUEST_CANCEL_HAND_HOG		= 1054;

	//广播玩家下庄
	const static std::int16_t SERVER_RESPONSE_CANCEL_HAND_HOG	= 1054;

	//广播下注的限制
	const static std::int16_t SERVER_PULL_BET_LIMITED								= 1055;

	//广播开始下注
	const static std::int16_t SERVER_PULL_START_BET								= 1056;

	//玩家请求下注
	const static std::int16_t CLIENT_REQUEST_BET										= 1057;

	//广播玩家下注
	const static std::int16_t SERVER_RESPONSE_BET									= 1057;

	//deal 广播开始发牌
	const static std::int16_t SERVER_PULL_START_DEAL							= 1058;

	//桌子记录
	const static std::int16_t SERVER_PULL_ROUND_RECOREDS				= 1059;

	const static std::int16_t SERVER_PULL_HANDHOG_RECORDS			= 1060;

	//广播结算
	const static std::int16_t SERVER_PULL_SETTLE_ACCOUNTS				= 1061;

	//
	const static std::int16_t CLIENT_REQUEST_RAND								= 1062;

	const static std::int16_t SERVER_RESPONSE_RAND								= 1062;

	//玩家请求vip坐下
	const static std::int16_t CLIENT_REQUEST_VIP_SITDOWN = 1901;

	//广播vip起来
	const static std::int16_t CLIENT_REQUEST_VIP_SITUP = 1902;

	//广播vip 椅子列表
	const static std::int16_t SERVER_PULL_VIP_SEATS_LIST = 1903;

};

namespace RunFast
{
	//服务器广播癞子牌
	const static std::int16_t SERVER_BROADCAST_LAIZI_CARD = 7001;

	//服务器广播癞子打出牌型
	const static std::int16_t SERVER_BROADCAST_PLAYED_CARD_LAIZI = 7002;
	 
	//客户端请求癞子匹配类型出牌
	const static std::int16_t CLIENT_REQUEST_PLAYED_LAIZI_MATCH_TYPE = 7003;
	//服务端响应癞子匹配类型出牌请求
	const static std::int16_t SERVER_RESPONSE_PLAYED_LAIZI_MATCH_TYPE = 7003;

	//断线重连通知客户端癞子牌
	const static std::int16_t SERVER_NET_RECONNECT_NOTIFY_CLIENT_LAIZI = 7004;

    //强制结束游戏
    const static std::int16_t SERVER_BROADCAST_FORCE_GAMEOVER = 1066;
    //投票解散房间
    const static std::int16_t CLIENT_REQUES_DISBAND_VOTE = 1067;
    const static std::int16_t SERVER_BROADCAST_DISBAND_VOTE = 1067;
    //比赛开始
    const static std::int16_t SERVER_BROAD_MATCH_START = 1068;
    //通知首出人
    const static std::int16_t SERVER_NOTIFY_FIRST_PLAY = 1069;
   //通知房间解散
    const static std::int16_t SERVER_NOTIFY_DISBAND = 1070;
  //广播下一个出牌人
  const static std::int16_t SERVER_BROADCAST_NEXT_PLAYER = 1071;

  //出牌
  const static std::int16_t CLIENT_REQUEST_PLAY = 1072;

  const static std::int16_t SERVER_RESPONSE_PLAY = 1072;

  //广播打出的牌
  const static std::int16_t SERVER_BROADCAST_PLAYED_CARD = 1073;

  const static std::int16_t SERVER_BROADCAST_GAMEOVER = 1074;

  //托管
  const static std::int16_t CLIEN_REQUEST_TUOGUAN = 1075;
  const static std::int16_t SERVER_RESPONSE_TUOGUAN = 1075;

  //暂离
  const static std::int16_t CLIENT_REQUEST_ZANLI = 1076;
  const static std::int16_t SERVER_RESPONSE_ZANLI = 1076;

  //解散房间
  const static std::int16_t CLIENT_REQUEST_DISBAND = 1077;
  const static std::int16_t SERVER_RESPONSE_DISBAND = 1077;

  //广播桌面快照
  const static std::int16_t SERVER_BROADCAST_TABLE_SNAPSHOT = 1078;

  //炸弹结算
  const static std::int16_t SERVER_BROADCAST_BOMB_ACCOUNTS = 1079;

  //房间结算
  const static std::int16_t SERVER_BROADCAST_ROOM_ACCOUNTS = 1080;
  //新房间结算
  const static std::int16_t SERVER_BROADCAST_ROOM_ACCOUNTS_EX = 8080;

  //加金币
  const static std::int16_t SERVER_UPDATE_GLOP = 1081;

  //单局结算
  const static std::int16_t SERVER_BROADCAST_ACCOUNTS = 1082;
  //新单局结算
  const static std::int16_t SERVER_BROADCAST_ACCOUNTS_EX = 8082;

  //断线后重发手牌
  const static std::int16_t SERVER_NET_RECONNECT_HANDCARDS = 1083;

  //发送消息
  const static std::int16_t CLIENT_SEND_MESSAGE = 1084;
  //广播消息
  const static std::int16_t SERVER_BROADCAST_MESSAGE = 1084;

  const static std::int16_t CLINET_ADD_ROBOT = 1085;
  const static std::int16_t SERVER_ADD_ROBOT = 1085;

  const static std::int16_t SERVER__UPDATE_GOLD = 1086;

  //进入比赛大厅
  const static std::int16_t CLIENT_ENTER_MATCH_HALL = 1087;
  const static std::int16_t SERVER_ENTER_MATCH_HALL = 1087;

  //离开比赛大厅
  const static std::int16_t CLIENT_LEVAE_MATCH_HALL = 1088;
  const static std::int16_t SERVER_LEVAE_MATCH_HALL = 1088;

  //报名
  const static std::int16_t CLIENT_CHECK_IN = 1089;
  const static std::int16_t SERVER_CHECK_IN = 1089;

  //取消报名
  const static std::int16_t CLIENT_CHECK_OUT = 1090;
  const static std::int16_t SERVER_CHECK_OUT = 1090;

  //等待其它房间完成比赛
  const static std::int16_t SERVER_MATCH_WAITING = 1091;

  //比赛排名
  const static std::int16_t SERVER_MATCH_RANKING = 1092;

  //比赛淘汰
  const static std::int16_t SERVER_MATCH_ELIMINATE_PAYER = 1093;

  //比赛颁奖
  const static std::int16_t SERVER_MATCH_PRIZE = 1094;

  //比赛排名
  const static std::int16_t SERVER_MATCH_RANK = 1095;

  //比赛推送
  const static std::int16_t SERVER_MATCH_PUSH_MESSAGE= 1096;

  //广播赛场状态
  const static std::int16_t SERVER_BROAD_PLAYGROUND_STATUS = 1097;

  const static std::int16_t SERVER_BROAD_NOW_RANK = 1098;

  

  //快速游戏
  const static std::int16_t CLIENT_FIND_Z_ROOM = 1099;
  const static std::int16_t SERVER_FIND_Z_ROOM = 1099;

  //快速游戏
  const static std::int16_t CLIENT_CHANGE_Z_ROOM = 1100;
  const static std::int16_t SERVER_CHANGE_Z_ROOM = 1100;

  const static std::int16_t SERVER_NOTIFY_GLOD_LIMIT = 1101;

  const static std::int16_t SERVER_NOTIFY_MSG_LIST = 1102;

  const static std::int16_t SERVER_VERSION_MESSAGE = 9999;

  namespace ErrorCode
  {
    static const std::int32_t ERROR_DISBAND_MUST_BE_OWNNER = -1001; //只有房主才能解散
    static const std::int32_t ERROR_DISBAND_ANOTHER_PLAYER = -1002;//只剩房主时才能解散


    static const std::int32_t ERROR_ROOM_COUNT_NULL = -1020;//游戏开始失败，房间剩余局数为0

    static const std::int32_t ERROR_ROOM_ID_ERROR= -3;//房间号错误

    static const std::int32_t ERROR_MATCH_CLOSED = -1010; //比赛已关闭
    static const std::int32_t ERROR_MATCH_PLAYING = -1011; //比赛已开始
    static const std::int32_t ERROR_MATCH_FULL = -1012; //比赛人数已满
    static const std::int32_t ERROR_MATCH_NOT_FIND = -1013; //赛场未找到
    static const std::int32_t ERROR_MATCH_READY_CHECK = -1014; //不能重复报名
    static const std::int32_t ERROR_MATCH_CHECK_COST_FAILED = -1015; //报名费用不够

    static const std::int32_t ERROR_ENTER_MATCH_HAS_ROOM = -1016; //请先完成已有的游戏

    static const std::int32_t ERROR_CHECK_HAS_NO_MUCH_PLAYGROUND = -1017; //赛场不足

    static const std::int32_t ERROR_MATCH_SERVER_CLOSED = -1018; //赛场未开启

    static const std::int32_t ERROR_GLOD_ENTER_LIMIT = -1020; //金币数量不符合
    static const std::int32_t ERROR_Z_ROOM_PLAYING = -1021; //正在游戏中
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

