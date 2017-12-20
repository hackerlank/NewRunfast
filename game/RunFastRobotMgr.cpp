#include "RunFastRobotMgr.h"
#include "playerinterface.h"
#include "RoomInterface.h"

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#include <assistx2/configure.h>
#include "PokerCmd.h"
#include "ConfigMgr.h"

RunFastRobotMgr::RunFastRobotMgr(boost::asio::io_service & ios, assistx2::TcpHanlderWrapper * connector) :
    ios_(ios),
    gatewayconnector_(connector),
    timer_queue_(nullptr),
    robot_timer_(ios_)
{

}

RunFastRobotMgr::~RunFastRobotMgr()
{
    for (auto it = idle_robot_pool_.begin(); it != idle_robot_pool_.end(); ++it)
    {
        delete  *it;
    }
    active_robots_.clear();
    idle_robot_pool_.clear();
}

int RunFastRobotMgr::Init()
{
    //初始化机器人池
    boost::int32_t robot_begin = 0;
    boost::int32_t robot_end = 0;
    auto cfg_reader = ConfigMgr::getInstance()->GetAppCfg();
    cfg_reader->getConfig("robot", "begin", robot_begin);
    cfg_reader->getConfig("robot", "end", robot_end);

    for (int i = robot_begin; i < robot_end; i++)
    {
        PlayerInterface * robot = PlayerInterface::CreateRobot(i, boost::bind(&RunFastRobotMgr::DelayPushMessage, this, _1, _2, _3, _4),
            boost::bind(&RunFastRobotMgr::RemoveRobot, this, _1));
        idle_robot_pool_.push_back(robot);
    }

    return 0;
}

PlayerInterface* RunFastRobotMgr::MallocRobot()
{
    if (idle_robot_pool_.empty() == false)
    {
        PlayerInterface * robot = idle_robot_pool_.front();
        idle_robot_pool_.pop_front();

        return robot;
    }

    return nullptr;
}

void RunFastRobotMgr::OnEvent(const int event, PlayerInterface * player, RoomInterface * room, EventContext *)
{

    switch (event)
    {
    case RoomEventListener::PLAYER_ENTER_EVNET:
    {
        ActivateRobot(room);
        ActivateRobot(room);
    }
    break;
    case RoomEventListener::PLAYER_LEAVE_EVNET:
    {
        RobotLeaveRoom(room);
    }
    break;
    case RoomEventListener::PLAYER_READY_EVNET:
    {
        OnReady(room);
    }
    break;
    default:
        break;
    }
}

void RunFastRobotMgr::OnReady(RoomInterface* room)
{
    assistx2::Stream stream(Texas::CLIENT_REQUEST_SITDOWN);
    stream.End();
    const std::map<uid_type, PlayerInterface *> & players = room->GetPlayers();

    for (auto iter : players)
    {
        if (PlayerInterface::IsRobot(iter.second))
        {
            room->OnMessage(iter.second, &stream);
        }
    }
}

void RunFastRobotMgr::RobotLeaveRoom(RoomInterface * room)
{
    const std::map<uid_type, PlayerInterface *> players = room->GetPlayers();
    for (auto iter : players)
    {
        if (iter.second == nullptr)
        {
            continue;
        }
        if (PlayerInterface::IsRobot(iter.second))
        {
            auto res = room->Leave(iter.second, 1077);
            if (res == 0)
            {
                iter.second->SetRoomObject(nullptr);
            }
            else
            {
                DLOG(INFO) << "RunFastRobotMgr::RobotLeaveRoom() Leave Failed";
            }
            iter.second->Destroy();
        }
    }
}

void RunFastRobotMgr::ActivateRobot(RoomInterface * room)
{
    auto robot = MallocRobot();
    if (robot == nullptr)
    {
        DLOG(INFO) << "RunFastRobotMgr::ActivateRobot() MallocRobot Failed";
        return;
    }
    if (robot->Serialize(true) != 0)
    {
        DLOG(INFO) << "RunFastRobotMgr::ActivateRobot() Serialize Failed";
        return;
    }
    robot->SetRoomObject(room);

    if (room->Enter(robot) != 0)
    {
        robot->SetRoomObject(nullptr);
        DLOG(INFO) << "RunFastRobotMgr::ActivateRobot() Enter Failed";
        idle_robot_pool_.push_back(robot);
        return;
    }

    active_robots_.push_back(robot);
    DLOG(INFO) << "RunFastRobotMgr::ActivateRobot() robot:=" << robot->GetUID();
}

PlayerInterface* RunFastRobotMgr::GetRobot()
{
    auto robot = MallocRobot();
    if (robot == nullptr)
    {
        DLOG(INFO) << "RunFastRobotMgr::ActivateRobot() MallocRobot Failed";
        return nullptr;
    }
    if (robot->Serialize(true) != 0)
    {
        DLOG(INFO) << "RunFastRobotMgr::ActivateRobot() Serialize Failed";
        return nullptr;
    }
    active_robots_.push_back(robot);

    return robot;
}

PlayerInterface* RunFastRobotMgr::GetRobot(int64_t min, int64_t max)
{
    auto robot = GetRobot();
    if (robot != nullptr)
    {
        auto now_gold = robot->GetGameBaseInfo().gold();
        if (now_gold < min)
        {
            auto gold = rand() % 100 + (min - now_gold);
            robot->GoldPay(-gold);
        }
        if (now_gold > max)
        {
            auto gold = rand() % 100 + (max - now_gold);
            robot->GoldPay(gold);
        }
    }

    return robot;
}

bool RunFastRobotMgr::IsAllRobot(std::map<uid_type, PlayerInterface *> players)
{
    for (auto iter : players)
    {
        if (PlayerInterface::IsRobot(iter.second) == false)
        {
            return false;
        }
    }
    return true;
}

void RunFastRobotMgr::RemoveRobot(PlayerInterface * player)
{
    player->Serialize(false);

    idle_robot_pool_.push_back(player);
    auto it = std::find(active_robots_.begin(), active_robots_.end(), player);
    if (it != active_robots_.end())
    {
        active_robots_.erase(it);
    }
}

void RunFastRobotMgr::DelayPushMessage(PlayerInterface * player, boost::shared_ptr< assistx2::Stream > packet, RoomInterface * room, time_t delay)
{

}
