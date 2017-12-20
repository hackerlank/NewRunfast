#ifndef _XGATEWAY_SRC_XPROXY_CMD_H_
#define _XGATEWAY_SRC_XPROXY_CMD_H_

namespace xProxy
{
	const static std::int16_t CLINET_REQUEST_REGISTER								= 10000;

	//response
	const static std::int16_t SERVER_RESPONSE_REGISTER							= 10000;

	//·�ɵ�������ڷ���
	const static std::int16_t STORE_ROUTE_TO_USE_ROOM						= 10001;

	//standard. ��׼·�ɰ�
	const static std::int16_t STANDARD_ROUTE_PACKET								= 10002;
	// PACKET { STANDARD_ROUTE_PACKET, target_type, target_uid, sub_packet_cmd, data... }

	const static std::int16_t 	GATEWAY_EVENT_GAME_SERVER_CLOSE	= 10003;

	//ϵͳ����ָ�� ��0�� �رշ������� 1�� �������������ļ���
	const static std::int16_t SYSTEM_ADMINI_CMD										= 10004;

    const static std::int16_t		CONVERGE_PACKET = 10007;

	enum
	{
		REGISTER_SUCCESS,			//��½�ɹ�
		REGISTER_FAILED,			//��½ʧ��, δ֪����.
		REGISTER_KEY_ERROR,			//�ỰKEYУ��ʧ��
		REGISTER_RELOGIN			//�ظ���½, ֻ�ᷢ��ǰһ������.
	};

	static const std::int32_t	SESSION_TYPE_BEGIN						    = 0;
	static const std::int32_t	SESSION_TYPE_AS_CLINET				    = 1;			//AS�ͻ�������
	static const std::int32_t	SESSION_TYPE_GAME_SERVER		    = 2;			//��Ϸ������	
	static const std::int32_t	SESSION_TYPE_STORE					        = 3;			//�̳�
	static const std::int32_t	SESSION_TYPE_LOGGER					    = 4;			//��־������
	static const std::int32_t	SESSION_TYPE_ADMIN_TOOL		        = 5;			//������
	static const std::int32_t	SESSION_TYPE_END							= 6;
	
	static const std::int32_t ALL_SERVER = 0;

	const std::int32_t	ADMINI_CMD_CLOSE_SERVER         = 0;
	const std::int32_t	ADMINI_CMD_RELOAD                   = 1;
	const std::int32_t	ADMINI_CMD_STOP_SERVER			= 2;

	enum 
	{
		//route
		ROUTE_SUCCESS,				//���ݳɹ���·�ɵ�Ŀ�ķ�����
		ROUTE_FAILED				//����·��ʧ��
	};
};


#endif //_XGATEWAY_SRC_XPROXY_CMD_H_

