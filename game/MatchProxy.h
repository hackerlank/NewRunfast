#ifndef _MATCH_PROXY_H_
#define _MATCH_PROXY_H_

#include <memory>
#include <assistx2/tcphandler_wrapper.h>
#include <assistx2/stream2.h>
#include <functional>

class MatchProxyImpl;
class PlayerInterface;
class MatchProxy:public std::enable_shared_from_this<MatchProxy>
{
public:
    explicit MatchProxy(boost::asio::io_service & ios, assistx2::TcpHanlderWrapper * connector);
    virtual ~MatchProxy();
public:
    std::int32_t OnMessage(PlayerInterface * player,const assistx2::Stream& packet);
    void SetLeaveCallBack(std::function<void(PlayerInterface *)> func);
private:
    std::unique_ptr< MatchProxyImpl > pImpl_;
};

#endif
