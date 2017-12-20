#include "playerinterface.h"

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

#include <Player.h>
#include <RunFastRobot.h>

PlayerInterface * PlayerInterface::CreateRealPlayer( uid_type mid )
{
	return new Player(mid);
}

PlayerInterface * PlayerInterface::CreateRobot( uid_type mid,
											 boost::function<void (PlayerInterface * player, boost::shared_ptr< assistx2::Stream > packet, RoomInterface * room, time_t delay) > send_callback, 
											 boost::function<void (PlayerInterface * ) >  kickout )
{
	return new RunFastRobot(mid, send_callback, kickout);
}

bool PlayerInterface::IsRobot( const PlayerInterface * player )
{
	return player->GetType() == PlayerInterface::ROBOT_TYPE;
}

void PlayerInterface::SendTo(const assistx2::Stream &)
{
    DCHECK(false);
}
