#include "handle_obj.h"

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#include "config_mgr.h"
#include "match_proxy.h"
#include "runfast_game_mgr.h" 
#include <memory>

HandleObj::HandleObj()
    :service_(boost::asio::io_service()),
    timer_(service_),
    event_timer_(service_),
    gatewayconnector_(assistx2::TcpHanlderWrapper(service_)),
    proxy_(assistx2::TcpHanlderWrapper(service_, assistx2::ParserType::STREAMEX_PARSER))
{

}

HandleObj::~HandleObj()
{

}

void HandleObj::Initialize()
{


    return 0;
}
