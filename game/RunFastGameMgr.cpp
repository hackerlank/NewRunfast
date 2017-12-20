#include "RunFastGameMgr.h"
#include <vector>

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <assistx2/platform_wrapper.h>
#include <assistx2/string_wrapper.h>
#include <assistx2/time_tracer.h>
#include <assistx2/configure.h>

#include "PokerCmd.h"
#include "RunFastRoomMgr.h"
#include "RoomInterface.h"
#include "ConfigMgr.h"
#include "ProxyCmd.h"
#include "TimerHelper.h"
#include "playerinterface.h"
#include "version.h"
#include "RunFastRobotMgr.h"
#include "RunFastTracer.h"
#include "DataCenter.h"
#include "PrivateRoom.h"
#include "MatchProxy.h"
#include "PlayerRoomManager.h"
#include "playermgr.h"
#include "game_obj.h"

std::int32_t g_server_id = 0;
std::int32_t g_enable_ip_check = 0;
bool g_server_closed = false;
bool g_server_stopped = false;
std::int32_t g_game_session = 0;
std::vector<std::int32_t > g_ip_white_list;

#define KICK_DELAY_TIME (8)

const static std::int16_t ENTER_TO_MATCH_SERVER = 9997;
const static std::int16_t GAMESERVER_ROUTE_PACKET = 10010;

RunFastGameMgr::RunFastGameMgr(const boost::asio::io_service & ios)
    : GameObj(ios),
    roommgr_(nullptr), robot_mgr_(nullptr), match_hall_(nullptr), match_proxy_(nullptr)
{

}

RunFastGameMgr::~RunFastGameMgr(void)
{
}

std::int32_t RunFastGameMgr::Initialize()
{
    LOG(INFO) << "--------------------Initialize Start-----------------------";
    boost::uuids::random_generator  generator;

    std::stringstream ss;
    ss << generator();

    run_id_ = ss.str();
    LOG(INFO) << "RunFastGameMgr::Initialize()-> run_id_:=" << run_id_;

    GlobalTimerProxy::getInstance()->Init(ios_);
    g_server_id = ConfigMgr::getInstance()->GetSID();

    auto cfg_reader = ConfigMgr::getInstance()->GetAppCfg();
    cfg_reader->getConfig("white_list", "enable_check", g_enable_ip_check);

    std::string host;
    std::string port;
    cfg_reader->getConfig("xGateWay", "host", host);
    cfg_reader->getConfig("xGateWay", "port", port);

    std::string white_list;
    if (g_enable_ip_check != 0)
    {
        if (cfg_reader->getConfig("white_list", "host", white_list) == true)
        {
            std::vector<std::string> white_list_string;
            assistx2::SplitString(white_list, ",", white_list_string);
            for (auto it : white_list_string)
            {
                int temp;
                temp = std::stoi(it);
                g_ip_white_list.push_back(temp);
            }
        }
    }
    LOG(INFO) << "g_enable_ip_check:=" << g_enable_ip_check << ", white_list:=" << white_list;


    if (RunFastTracer::getInstance()->Init(gatewayconnector_) != 0)
    {
        LOG(ERROR) << ("Tracer INIT FAILED.");
        exit(-1);
    }

    robot_mgr_ = new RunFastRobotMgr(ios_, gatewayconnector_);
    if (robot_mgr_ == nullptr)
    {
        LOG(ERROR) << ("RobotMgr INIT FAILED.");
        exit(-1);
    }

    //PlayerRoomManager::getInstance()->Init();

    robot_mgr_->Init();

    roommgr_ = new RunFastRoomMgr(ios_);
    if (roommgr_ == nullptr)
    {
        LOG(ERROR) << ("RunFastRoomMgr INIT FAILED.");
        exit(-1);
    }

    roommgr_->AddRoomListener(robot_mgr_);

    roommgr_->Initialize(boost::bind(&RunFastGameMgr::OnLeaveRoom, this, _1, _2), gatewayconnector_);

    match_proxy_ = std::make_shared<MatchProxy>(ios_, gatewayconnector_);
    if (match_proxy_ != nullptr)
    {
        match_proxy_->SetLeaveCallBack(std::bind(&RunFastGameMgr::OnLeaveMatchHall, this, std::placeholders::_1));
    }

    GlobalTimerProxy::getInstance()->NewTimer(std::bind(&RunFastGameMgr::OnRobotTimer, this), 3);

    OnWriteOnlineLog();

    LOG(INFO) << "--------------------Initialize End-----------------------";

    return 0;
}

void RunFastGameMgr::ShutDown()
{
    if (roommgr_ != nullptr)
    {
        delete roommgr_;
        roommgr_ = nullptr;
    }

    if (gatewayconnector_ != nullptr)
    {
        delete gatewayconnector_;
        gatewayconnector_ = nullptr;
    }
    RunFastTracer::Destroy();
}

std::int32_t RunFastGameMgr::OnMessage(assistx2::TcpHandler * socket, boost::shared_ptr<assistx2::NativeStream > native_stream)
{
    if (g_server_stopped == true)
    {
        assistx2::Stream result(Texas::SERVER_PUSH_SERVERS_STOPPED);
        result.Write(player->GetUID());
        result.Write(std::int32_t(0));
        result.End();
        gatewayconnector_->SendTo(result.GetNativeStream());

        DLOG(INFO) << "RunFastGameMgr::OnEnterMatchHall() g_server_stopped is true";
        return 0;
    }

    assistx2::Stream packet(native_stream);
    const std::int32_t cmd = packet.GetCmd();
    DLOG(INFO) << "RunFastGameMgr::OnMessage()->cmd:" << cmd;

    switch (cmd)
    {
    case xProxy::SERVER_RESPONSE_REGISTER:
        OnRegister(&packet);
        return 0;
    case xProxy::STORE_ROUTE_TO_USE_ROOM:
        OnRouteToRoom(&packet);
        return 0;
    case Texas::CLIENT_REQUEST_LOGIN:
        OnLoginEvent(&packet);
        return 0;
    case xProxy::STANDARD_ROUTE_PACKET:
        OnRouteMessage(&packet);
        return 0;
    case GAMESERVER_ROUTE_PACKET:
        OnRouteMatchServerResult(& packet);
        return 0;
    default:
        break;
    }

    const uid_type uid = packet.Read<std::int32_t>();
    DLOG(INFO) << "RunFastGameMgr::OnMessage() mid:=" << uid << " cmd:=" << cmd;

    PlayerInterface * player;
    if ((player = playermgr_->GetPlayerInterface(uid)) == nullptr)
    {
        DLOG(INFO) << "RunFastGameMgr::OnMessage() mid:=" << uid << " cmd:=" << cmd;
        return 0;
    }
    else if (player->GetMatchProxy())
    {
        return player->GetMatchProxy()->OnMessage(player, packet);
    }

    switch(cmd)
    {
    case ENTER_TO_MATCH_SERVER:
        player->SetMatchProxy(match_proxy_);
        break;
    case RunFast::CLIENT_ENTER_MATCH_HALL:
        OnEnterMatchHall(player, packet);
        break;
    default:
        roommgr_->OnMessage(&packet, player);
        playermgr_->OnMessage(uid, packet);
    }
    std::stringstream ss;
    ss << "RunFastGameMgr::OnMessage:-->" << cmd;
    assistx2::TimeTracer tracerline(new assistx2::SimpleDumpHelper((ss.str()), assistx2::TimeTracer::SECOND));
    return 0;
}

std::int32_t RunFastGameMgr::OnConnect(assistx2::TcpHandler * handler, assistx2::ErrorCode /*err*/)
{
    DLOG(INFO) << "RunFastGameMgr::OnConnect()";

    std::int32_t err = handler->SetReceiveBufferSize(1024 * 128);
    if (err != 0)
    {
        LOG(ERROR) << "OnConnect, SET RECV BUFFER FAILED, err:=" << err;
    }

    err = handler->SetSendBufferSize(1024 * 128);
    if (err != 0)
    {
        LOG(ERROR) << "OnConnect, SET SEND BUFFER FAILED, err:=" << err;
    }

    assistx2::Stream stream(xProxy::CLINET_REQUEST_REGISTER);
    stream.Write(static_cast<std::int32_t>(xProxy::SESSION_TYPE_GAME_SERVER));
    stream.Write(g_server_id);
    stream.Write(run_id_);
    stream.End();

    handler->Send(stream.GetNativeStream());

    return 0;
}

std::int32_t RunFastGameMgr::OnClose(assistx2::TcpHandler * handler, assistx2::ErrorCode err)
{
    LOG(INFO) << "RunFastGameMgr::OnClose(). ";

    auto clone(players_);
    for (auto it = clone.begin(); it != clone.end(); ++it)
    {
        if (it->second->GetRoomObject() != nullptr)
        {
            OnLeaveRoom(it->second, 0);
        }
    }

    if (g_server_closed == true)
    {
        ios_.stop();
    }

    return 0;
}

int RunFastGameMgr::OnProxyMessage(assistx2::TcpHandler * socket, boost::shared_ptr< assistx2::NativeStream >  packet)
{
    assistx2::StreamEx stream(packet);

    const std::int16_t cmd = stream.GetCmd();

    switch (cmd)
    {
    case Proxy::CLINET_REQUEST_REGISTER:
    {
        auto err = stream.Read<std::int32_t>();
        LOG(INFO) << "Proxy::CLINET_REQUEST_REGISTER err:=" << err;

        OnProxyTimer();
    }
    break;
    case RunFast::PHP::PHP_INCR_GLOP:
    {
        auto mid = stream.Read<std::int32_t>();
        auto incr = stream.Read<std::int64_t>();

        auto player = GetPlayerInterface(mid);
        int64_t amount = 0;
        auto err = 0;
        if (player != nullptr)
        {
            auto res = player->GoldPay(-incr);
            if (res == true)
            {
                err = 0;
                amount = player->GetGameBaseInfo().gold();
            }
            else
            {
                err = -1;
            }
        }
        else
        {
            int64_t real_pay = 0;
            err = DataLayer::getInstance()->Pay(mid, -incr, amount, real_pay);
        }
       
        stream.Flush();
        stream.Write(err);
        stream.Write(amount);
        stream.End();
        proxy_->SendTo(stream.GetNativeStream());
    }
    break;
    default:
        break;
    }

    return 0;
}

void RunFastGameMgr::OnProxyTimer()
{
    assistx2::StreamEx  heartbeat_stream(Proxy::HEARTBEAT_PACKET);
    heartbeat_stream.End();

    if (proxy_ != nullptr && g_server_stopped == false)
    {
        proxy_->SendTo(heartbeat_stream.GetNativeStream());
    }

    GlobalTimerProxy::getInstance()->NewTimer(std::bind(&RunFastGameMgr::OnProxyTimer, this), 9);
}

int RunFastGameMgr::OnProxyConnect(assistx2::TcpHandler * handler)
{
    //LOG(INFO) << "RunFastGameMgr::OnProxyConnect() ";

    const static std::int32_t MASTER = 0;

    assistx2::StreamEx stream(Proxy::CLINET_REQUEST_REGISTER);
    stream.Write(MASTER);
    stream.Write(g_server_id);
    stream.End();

    proxy_->SendTo(stream.GetNativeStream());

    return 0;
}

int RunFastGameMgr::OnProxyClose(assistx2::TcpHandler * handler)
{
    //LOG(ERROR) << "RunFastGameMgr::OnProxyClose() ";
    return 0;
}

void RunFastGameMgr::OnRegister(assistx2::Stream * packet)
{
    const boost::int32_t err = packet->Read<boost::int32_t>();
    if (err != xProxy::REGISTER_SUCCESS)
    {
        LOG(ERROR) << "RunFastGameMgr::OnMessage.  REGISTER FAILED. err:=" << err;
        ios_.stop();
    }
}

void RunFastGameMgr::OnRouteToRoom(assistx2::Stream * packet)
{
    //const boost::int32_t target = packet->Read<boost::int32_t>();
    std::string buffer;

    const std::size_t size = packet->ReadBinary(buffer);
    if (size >= assistx2::Stream::PACKET_HEADER_SIZE)
    {
        assistx2::Stream route(buffer.c_str(), size);
        //OnStoreMessage(target, route);
    }
}

void RunFastGameMgr::OnLoginEvent(assistx2::Stream * packet)
{
    const uid_type mid = packet->Read<boost::int32_t>();
    const std::int32_t login_source = packet->Read<std::int32_t>();
    g_game_session = packet->Read<std::int32_t>();
    /*const std::string ip_addr =*/ packet->Read<std::string>();

    DLOG(INFO) << "RunFastGameMgr::OnLoginEvent mid:=" << mid;

    auto it = players_.find(mid);
    if (it != players_.end())
    {
        //it->second->SetLoginAddr(ip_addr);
        it->second->SetLoginStatus(true);
        auto room = it->second->GetRoomObject();
        if (room != nullptr)
        {
            playermgr_->SendReConnect(mid, room->GetID(), room->getRunFastRoomCfg().type);
        }
        //else if (playGround.size() != 0)
        else if (it->second->GetMatchProxy() != nullptr)
        {
            playermgr_->SendReConnect(mid, 0, std::string("M0"));
        }
        else
        {
            playermgr_->SendReConnect(mid, 0, std::string(""));
            RunFastTracer::getInstance()->SendDisbandMessage(mid);
        }
        RunFastTracer::getInstance()->SendNotifyMsg(mid);
        GlobalTimerProxy::getInstance()->NewTimer(std::bind(&
            RunFastGameMgr::OnTimerReSendMsg, this,mid), 1);
        RunFastTracer::getInstance()->UpdateLoginPlayer(login_source, mid);
        SendServerMessage(mid);
        RunFastTracer::getInstance()->SendRoomMessage(mid);
    }
    else
    {
        PlayerInterface * player = nullptr;
        const boost::int32_t err = playermgr_->OnLogin(mid, player, login_source);
        if (err == 0)
        {
            DataCenter::getInstance()->UpdateUserInfo(mid);
            stUserInfo userinfo;
            auto res = DataCenter::getInstance()->FindMyRoom(mid, userinfo);
            if (res == true)
            {
                SendReConnect(mid, userinfo.room->GetID(), userinfo.room->getRunFastRoomCfg().type);
            }
            else
            {
                SendReConnect(mid, 0, std::string(""));
            }
            //player->SetLoginAddr(ip_addr);
            RunFastTracer::getInstance()->SendDisbandMessage(mid);
            RunFastTracer::getInstance()->SendNotifyMsg(mid);
            GlobalTimerProxy::getInstance()->NewTimer(std::bind(&
                RunFastGameMgr::OnTimerReSendMsg, this, mid), 1);
            RunFastTracer::getInstance()->UpdateLoginPlayer(login_source, mid);
            SendServerMessage(mid);
            RunFastTracer::getInstance()->SendRoomMessage(mid);
        }

        DLOG(INFO) << "Login mid:=" << mid << ", err:=" << err << ", login_source:" << login_source;
    }
}

void RunFastGameMgr::OnRobotTimer()
{
    auto rooms = roommgr_->GetWatingGlodRoom();
    for (auto iter : rooms)
    {
        for (auto it : iter.second )
        {
            if (it->GetState() == RoomInterface::PLAYING)
            {
                continue;
            }
            auto players = it->GetPlayers();
            if (players.size() == 3)
            {
                continue;
            }
            if (robot_mgr_->IsAllRobot(players) == false)
            {
                auto robot = robot_mgr_->GetRobot(it->getRunFastRoomCfg().min,
                    it->getRunFastRoomCfg().max);
                DCHECK(robot != nullptr);

                assistx2::Stream stream(Texas::CLIENT_REQUEST_ENTER_ROOM);
                stream.Write(-1);
                stream.Write(iter.first);
                stream.End();
                roommgr_->OnEnterRoom(robot, &stream);
            }
            else
            {
                for (auto player : players)
                {
                    roommgr_->OnLeaveRoom(player.second,0);
                }
            }
        }
    }

    GlobalTimerProxy::getInstance()->NewTimer(std::bind(&RunFastGameMgr::OnRobotTimer, this), 3);
}

void RunFastGameMgr::OnEnterMatchHall(PlayerInterface *player, const assistx2::Stream &packet)
{
    if (player->GetRoomObject() != nullptr)
    {
        assistx2::Stream stream(RunFast::SERVER_ENTER_MATCH_HALL);
        stream.Write(player->GetUID());
        stream.Write(RunFast::ErrorCode::ERROR_ENTER_MATCH_HAS_ROOM);
        stream.End();

        obj_->gatewayconnector()->SendTo(stream.GetNativeStream());
        return;
    }

    match_proxy_->OnMessage(player, packet);
}

void RunFastGameMgr::OnLeaveMatchHall(PlayerInterface *player)
{
    if (player->GetMatchProxy() == nullptr &&
        player->GetLoginStatus() == false)
    {
        playermgr_->RemovePlayer(player);
    }
}

void RunFastGameMgr::OnRouteMessage(assistx2::Stream * packet)
{
    const short subcmd = packet->Read<boost::int16_t>();

    switch (subcmd)
    {
        case xProxy::SYSTEM_ADMINI_CMD:
        {
            const boost::int32_t op = packet->Read<boost::int32_t>();

            LOG(ERROR) << "StoreEngine::OnRouteMessage. op:=" << op;

            //执行服务器立即关闭
            if (op == xProxy::ADMINI_CMD_CLOSE_SERVER)
            {
                LOG(ERROR) << ("xPoker Shutdown...");
                g_server_closed = true;
                g_server_stopped = true;

                gatewayconnector_->Close();
            }
            else if (op == xProxy::ADMINI_CMD_RELOAD)
            {
                //重新加载配置文件
                //ConfigMgr::getInstance()->Update();
            }
            //服务器即将关闭
            else if (op == xProxy::ADMINI_CMD_STOP_SERVER)
            {
                LOG(ERROR) << ("xPoker Stopped...");
                g_server_stopped = true;
               
                OnNotifyUpdate();
                GlobalTimerProxy::getInstance()->NewTimer(std::bind(&RunFastGameMgr::OnCloseServer, this), 180);
            }
        }
        break;
        default:
            break;
    }
}

void RunFastGameMgr::OnCloseServer()
{
    ios_.stop();
}

void RunFastGameMgr::OnNotifyUpdate()
{
    //系统通知:服务器已更新，请所有玩家在完成一局游戏或比赛后刷新游戏重新进入以免造成数据丢失,谢谢合作！
    assistx2::Stream stream(Proxy::SYSTEM_BROADCAST);
    stream.Write(0);
    stream.Write(std::string("\u7cfb\u7edf\u901a\u77e5\u003a\u670d\u52a1\u5668\u5df2\u66f4\u65b0\uff0c\u8bf7\u6240"
        "\u6709\u73a9\u5bb6\u5728\u5b8c\u6210\u4e00\u5c40\u6e38\u620f"
        "\u6216\u6bd4\u8d5b\u540e\u5237\u65b0\u6e38\u620f\u91cd\u65b0"
        "\u8fdb\u5165\u4ee5\u514d\u9020\u6210\u6570\u636e\u4e22\u5931\u002c\u8c22\u8c22\u5408\u4f5c\uff01"));
    stream.Write(1);
    stream.End();

    gatewayconnector_->SendTo(stream.GetNativeStream());

    GlobalTimerProxy::getInstance()->NewTimer(std::bind(&RunFastGameMgr::OnNotifyUpdate, this), 30);
}

void RunFastGameMgr::OnWriteOnlineLog()
{
    RunFastTracer::getInstance()->WriteOnlineRecord();

    GlobalTimerProxy::getInstance()->NewTimer(std::bind(&RunFastGameMgr::OnWriteOnlineLog, this), 60);
}

void RunFastGameMgr::OnTimerReSendMsg(std::int32_t mid)
{
    RunFastTracer::getInstance()->SendAccount(mid);
    RunFastTracer::getInstance()->SendRoomAccount(mid);
    RunFastTracer::getInstance()->SendMatchMessage(mid);
}

void RunFastGameMgr::SendServerMessage(std::int32_t mid)
{
    //发送服务器版本号
    assistx2::Stream stream(RunFast::SERVER_VERSION_MESSAGE);
    stream.Write(mid);
    stream.Write(ConfigMgr::getInstance()->server_version());
    stream.End();
    gatewayconnector_->SendTo(stream.GetNativeStream());
}

