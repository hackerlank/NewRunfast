#ifndef _RUNFAST_ROBOTMGR_H_
#define _RUNFAST_ROBOTMGR_H_

#include <set>

#include <assistx2/timer_wrapper.h>
#include <assistx2/stream.h>
#include <assistx2/tcphandler_wrapper.h>

#include "room_listener.h"

class RunFastRobotMgr : public RoomEventListener
{
public:
    RunFastRobotMgr(boost::asio::io_service & ios, assistx2::TcpHanlderWrapper *connector);
    virtual  ~RunFastRobotMgr();
    int Init();

public:
    virtual void OnEvent(const int event, PlayerInterface * player, RoomInterface * room, EventContext *);
    PlayerInterface* GetRobot();
    PlayerInterface* GetRobot(int64_t min,int64_t max);
    bool IsAllRobot(std::map<uid_type, PlayerInterface * > players);
private:
    PlayerInterface* MallocRobot();
    void RemoveRobot(PlayerInterface * player);
    void DelayPushMessage(PlayerInterface * player, boost::shared_ptr< assistx2::Stream > packet, RoomInterface * room, time_t delay);
    void ActivateRobot(RoomInterface * room);
    void RobotLeaveRoom(RoomInterface * room);
    void OnReady(RoomInterface* room);
private:
    boost::asio::io_service & ios_;

    assistx2::TcpHanlderWrapper * gatewayconnector_;

    assistx2::timer_wapper::TimerQueue * timer_queue_;

    boost::asio::deadline_timer robot_timer_;
    std::list< PlayerInterface *> active_robots_;
    //未分配的机器人
    std::list<PlayerInterface * > idle_robot_pool_;
};

#endif
