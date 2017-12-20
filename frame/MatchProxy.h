#ifndef _MATCH_PROXY_H_
#define _MATCH_PROXY_H_

#include <memory>
#include <assistx2/tcphandler_wrapper.h>
#include <assistx2/stream2.h>
#include <functional>

class GameObj;
class MatchProxyImpl;
class PlayerInterface;
class MatchProxy:public std::enable_shared_from_this<MatchProxy>
{
public:
    explicit MatchProxy();
    virtual ~MatchProxy();
public:
    std::int32_t OnMessage(PlayerInterface * player,const assistx2::Stream& packet);
    void SetLeaveCallBack(std::function<void(PlayerInterface *)> func);
    void OnEnterMatchHall(PlayerInterface * player,const assistx2::Stream& packet);
    void OnLeaveMatchHall(PlayerInterface * player);
    void RouteMatchServer(PlayerInterface * player, assistx2::Stream& packet);
private:
    std::function<void(PlayerInterface *)> leave_func_;
    std::unique_ptr< assistx2::TcpHanlderWrapper > connect_;
    std::unique_ptr<GameObj> guard_;
};

#endif
