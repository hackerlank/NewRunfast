#ifndef _XGATEWAY_SRC_XPROXY_CMD_H_
#define _XGATEWAY_SRC_XPROXY_CMD_H_

namespace xProxy
{
	const static std::int16_t CLINET_REQUEST_REGISTER								= 10000;

	//response
	const static std::int16_t SERVER_RESPONSE_REGISTER							= 10000;

	//路由到玩家所在房间
	const static std::int16_t STORE_ROUTE_TO_USE_ROOM						= 10001;

	//standard. 标准路由包
	const static std::int16_t STANDARD_ROUTE_PACKET								= 10002;
	// PACKET { STANDARD_ROUTE_PACKET, target_type, target_uid, sub_packet_cmd, data... }

	const static std::int16_t 	GATEWAY_EVENT_GAME_SERVER_CLOSE	= 10003;

	//系统管理指令 ｛0， 关闭服务器， 1， 重启加载配置文件｝
	const static std::int16_t SYSTEM_ADMINI_CMD										= 10004;

    const static std::int16_t		CONVERGE_PACKET = 10007;

	enum
	{
		REGISTER_SUCCESS,			//登陆成功
		REGISTER_FAILED,			//登陆失败, 未知错误.
		REGISTER_KEY_ERROR,			//会话KEY校验失败
		REGISTER_RELOGIN			//重复登陆, 只会发给前一个连接.
	};

	static const std::int32_t	SESSION_TYPE_BEGIN						    = 0;
	static const std::int32_t	SESSION_TYPE_AS_CLINET				    = 1;			//AS客户端连接
	static const std::int32_t	SESSION_TYPE_GAME_SERVER		    = 2;			//游戏服务器	
	static const std::int32_t	SESSION_TYPE_STORE					        = 3;			//商城
	static const std::int32_t	SESSION_TYPE_LOGGER					    = 4;			//日志服务器
	static const std::int32_t	SESSION_TYPE_ADMIN_TOOL		        = 5;			//管理工具
	static const std::int32_t	SESSION_TYPE_END							= 6;
	
	static const std::int32_t ALL_SERVER = 0;

	const std::int32_t	ADMINI_CMD_CLOSE_SERVER         = 0;
	const std::int32_t	ADMINI_CMD_RELOAD                   = 1;
	const std::int32_t	ADMINI_CMD_STOP_SERVER			= 2;

	enum 
	{
		//route
		ROUTE_SUCCESS,				//数据成功地路由到目的服务器
		ROUTE_FAILED				//数据路由失败
	};
};


#endif //_XGATEWAY_SRC_XPROXY_CMD_H_

