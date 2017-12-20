#include "game_obj.h"

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#include "ConfigMgr.h"
#include "MatchProxy.h"
#include "RunFastGameMgr.h" 

GameObj::GameObj(boost::asio::io_service service)
    :service_(service), timer_(service), event_timer_(service)
{

}

GameObj::~GameObj()
{

}

void GameObj::Initialize()
{
    DCHECK(gatewayconnector_ == nullptr);
    gatewayconnector_ = boost::make_shared(assistx2::TcpHanlderWrapper(*ios_));
    if (gatewayconnector_ == nullptr)
    {
        LOG(ERROR) << ("RunFastGameMgr::Initialize gatewayconnector_ is nullptr.");
        exit(-1);
    }

    gatewayconnector_->RegisterCloseHandler(boost::bind(&GameObj::OnClose, this, _1, _2));
    gatewayconnector_->RegisterConnectHandler(boost::bind(&GameObj::OnConnect, this, _1, _2));
    gatewayconnector_->RegisterMessageHandler(boost::bind(&GameObj::OnMessage, this, _1, _2));
    gatewayconnector_->Connect(host, static_cast<unsigned short>(assistx2::atoi_s(port)));

    {
        std::string host;
        std::string port;
        ConfigMgr::getInstance()->GetAppCfg()->getConfig("Proxy", "host", host);
        ConfigMgr::getInstance()->GetAppCfg()->getConfig("Proxy", "port", port);

        if (host.empty() == false)
        {
            DCHECK(proxy_ == nullptr);
            proxy_ = boost::make_shared(assistx2::TcpHanlderWrapper(*ios_, assistx2::ParserType::STREAMEX_PARSER));
            if (proxy_ == nullptr)
            {
                LOG(ERROR) << ("RunFastGameMgr::Initialize INIT FAILED.");
                exit(-1);
            }

            proxy_->RegisterCloseHandler(boost::bind(&GameObj::OnProxyClose, this, _1));
            proxy_->RegisterConnectHandler(boost::bind(&GameObj::OnProxyConnect, this, _1));
            proxy_->RegisterMessageHandler(boost::bind(&GameObj::OnProxyMessage, this, _1, _2));
            proxy_->Connect(host, assistx2::atoi_s(port));
        }
        LOG(INFO) << "Proxy host:=" << host << ", port:=" << port;
    }

    return 0;
}
