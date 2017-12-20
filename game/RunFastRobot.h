#ifndef _RUNFAST_ROBOT_H_
#define _RUNFAST_ROBOT_H_

#include "xPoker.h"
#include "Player.h"
#include "card_interface.h"

typedef boost::function<void(PlayerInterface * player, boost::shared_ptr< assistx2::Stream > packet, RoomInterface * room, time_t delay) > DelaySendTo;

class RunFastRobotImpl;
class RunFastRobot : public Player
{
public:
    RunFastRobot(int32_t mid, DelaySendTo delay_send_helper, boost::function<void(PlayerInterface *) > & kickout_callback);
    virtual ~RunFastRobot(void);
public:
    void OnMessage(assistx2::Stream clone);
public:
    //初始化玩家数据
    virtual std::int32_t Serialize(bool loadorsave);
    virtual void SendTo(const assistx2::Stream & stream);

    virtual void Destroy();

    virtual void SetRoomObject(RoomInterface * roomobject);
private:
    friend class RunFastRobotImpl;
    std::unique_ptr<RunFastRobotImpl> pImpl_;
};
#endif
