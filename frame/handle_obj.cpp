#include "handle_obj.h"
#include "config_mgr.h"
#include "match_proxy.h"
#include "runfast_game_mgr.h"

#include <memory>
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>


HandleObj::HandleObj()
    :service_(new boost::asio::io_service()),
    timer_(*service_),
    event_timer_(*service_)
{
    gatewayconnector_ = (new assistx2::TcpHanlderWrapper(*service_, assistx2::BIN_FORMAT_PARSER));
    proxy_ = (new assistx2::TcpHanlderWrapper(*service_, assistx2::ParserType::STREAMEX_PARSER));
}

HandleObj::~HandleObj()
{

}

void HandleObj::Initialize()
{


}
