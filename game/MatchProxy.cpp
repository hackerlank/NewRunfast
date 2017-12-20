#include "MatchProxy.h"
#include "playerinterface.h"
#include "PokerCmd.h"

const static std::int16_t GAMESERVER_ROUTE_PACKET = 10010;
const static std::int16_t LEAVE_FROM_MATCH_SERVER = 9998;

class MatchProxyImpl
{
public:
    MatchProxyImpl(boost::asio::io_service & ios);
    ~MatchProxyImpl();
    void RouteMatchServer(PlayerInterface * player, assistx2::Stream& packet);
public:
    boost::asio::io_service & ios_;
    assistx2::TcpHanlderWrapper * connector_;
    std::function<void(PlayerInterface *)> leave_func_;
};

MatchProxy::MatchProxy(boost::asio::io_service & ios,assistx2::TcpHanlderWrapper * connector):
pImpl_(new MatchProxyImpl(ios))
{
    pImpl_->connector_ = connector;
}

MatchProxy::~MatchProxy()
{

}

std::int32_t MatchProxy::OnMessage(PlayerInterface * player, const assistx2::Stream& packet)
{
    assistx2::Stream clone(packet);
    const auto cmd = clone.GetCmd();
    switch (cmd)
    {
    case LEAVE_FROM_MATCH_SERVER:
    {
        player->SetMatchProxy(nullptr);
        pImpl_->leave_func_(player);
    }
        break;
    case Texas::GATEWAY_EVENT_CONNECT_CLOSE:
        player->SetLoginStatus(false);
        break;
    default:
        pImpl_->RouteMatchServer(player, clone);
    	break;
    }

    return 0;
}

void MatchProxy::SetLeaveCallBack(std::function<void(PlayerInterface *)> func)
{
    pImpl_->leave_func_ = func;
}

MatchProxyImpl::MatchProxyImpl(boost::asio::io_service & ios):
 ios_(ios)
{
}

MatchProxyImpl::~MatchProxyImpl()
{
}

void MatchProxyImpl::RouteMatchServer(PlayerInterface * player, assistx2::Stream& packet)
{
    //packet.Insert(player->GetUID());

    assistx2::Stream stream(GAMESERVER_ROUTE_PACKET);
    stream.Write(1301);
    stream.WriteBinary(std::string(packet.GetNativeStream().GetData(), packet.GetNativeStream().GetSize()));
    stream.End();

    connector_->SendTo(stream.GetNativeStream());
}
