#ifndef _X_RUNFAST_GAME_MGR_
#define _X_RUNFAST_GAME_MGR_

#include <assistx2/tcphandler_wrapper.h>
#include <assistx2/stream2.h>

#include "xPoker.h"
#include "DataLayer.h"
#include <asio.hpp>

class MatchProxy;
class GameObj;
class PlayerMgr;
class RunFastRoomMgr;
class PlayerInterface;
class RunFastRobotMgr;
class RunFastGameMgr:public GameObj
{
public:
    explicit RunFastGameMgr(const boost::asio::io_service & ios);
    virtual ~RunFastGameMgr(void);
    RunFastGameMgr(const RunFastGameMgr& copy);

    std::int32_t Initialize();
    void ShutDown();

    std::int32_t OnMessage(assistx2::TcpHandler * socket, boost::shared_ptr<assistx2::NativeStream > native_stream);
    std::int32_t OnConnect(assistx2::TcpHandler * handler, assistx2::ErrorCode err);
    std::int32_t OnClose(assistx2::TcpHandler * handler, assistx2::ErrorCode err);

    int OnProxyMessage(assistx2::TcpHandler * socket, boost::shared_ptr< assistx2::NativeStream >  packet);
    int OnProxyConnect(assistx2::TcpHandler * handler);
    int OnProxyClose(assistx2::TcpHandler * handler);

private:    
    void OnRegister(assistx2::Stream * packet);
    void OnRouteToRoom(assistx2::Stream * packet);
    void OnLoginEvent(assistx2::Stream * packet);
    void OnProxyTimer();
    void OnRobotTimer();


    void OnRouteMessage(assistx2::Stream * packet);
    void OnCloseServer();
    void OnNotifyUpdate();
    void OnWriteOnlineLog();
    void OnTimerReSendMsg(std::int32_t mid);
    void SendServerMessage(std::int32_t mid);
private:  
    std::string run_id_;
    RunFastRoomMgr * roommgr_;
    RunFastRobotMgr *robot_mgr_;
    PlayerMgr *playermgr_;
    MatchProxy *match_proxy_;
};

#endif //_X_POKER_SRC_GAME_MGR_
