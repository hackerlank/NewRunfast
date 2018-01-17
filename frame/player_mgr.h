#ifndef PLAYERMGR_H
#define PLAYERMGR_H
#include <memory>
#include "xpoker.h"
#include "handle_obj.h"

class PlayerInterface;
class GameObj;
class PlayerMgr: public HandleObj
{
public:
    explicit PlayerMgr();
    virtual ~PlayerMgr();
    int32_t OnMessage(assistx2::Stream& packet, PlayerInterface* player);
    void RemovePlayer(PlayerInterface * player);
    PlayerInterface * GetPlayerInterface(const uid_type mid);
    std::int32_t OnConnectClose(PlayerInterface * player, assistx2::Stream * stream);
    std::int32_t OnLogin(const uid_type uid, PlayerInterface *& player, const std::int32_t login_source);
    void SendReConnect(const std::int32_t & mid, const std::int32_t & roomid, const std::string & roomtype);
    void OnHeartBeat(PlayerInterface * player, assistx2::Stream * packet);
    int32_t GetPlayerRoom(assistx2::Stream * packet);
    void OnRouteMatchServerResult(PlayerInterface * player, assistx2::Stream *packet);
//private:
    std::map<uid_type, PlayerInterface * > players_;
};

#endif // PLAYERMGR_H
