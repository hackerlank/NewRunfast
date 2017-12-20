#ifndef _RUNFAST_ROOM_MGR_H_
#define _RUNFAST_ROOM_MGR_H_

#include <assistx2/tcphandler_wrapper.h>
#include "xPoker.h"
#include <queue>

class RoomEventListener;
class RoomInterface;
class PlayerInterface;
class GameObj;
class RunFastRoomMgr:public boost::noncopyable
{
public:
    explicit RunFastRoomMgr();
    ~RunFastRoomMgr(void);

    std::int32_t OnMessage(boost::shared_ptr<assistx2::NativeStream > packet, PlayerInterface* player);

    boost::int32_t Initialize(std::function<std::int32_t(PlayerInterface *, std::int32_t) > kick_cb,
        assistx2::TcpHanlderWrapper * connector);
    RoomInterface * GetRoomObject(std::int32_t room);
    RoomInterface * GetPrivateRoomObject(std::int32_t room);
    std::int32_t CreateRoomByType(int32_t mid, const std::string& type, int32_t& cost);
    std::vector<RoomInterface * > GetRoomsByType(std::int32_t type_id);
    void RemovePrivateRoom(std::int32_t room);
    void AddRrivateRoom(std::int32_t owner, RoomInterface *room);
    void AddRoomListener(RoomEventListener * listener);
    RoomInterface* GetMatchRoom();
    std::vector<RoomInterface * > GetWatingGlodRoomByType(const std::string& type);
    RoomInterface * GetGoldRoomByType(const std::string& type);
    RoomInterface * GetRoomByType(const std::string& type);
    void AttachWatingGoldRoom(const std::string& type, RoomInterface * room);
    void DetachWatingGoldRoom(const std::string& type, RoomInterface * room);
    std::map<std::string, std::vector<RoomInterface * > > GetWatingGlodRoom();

    void OnChangeRoom(PlayerInterface * player);
    void OnFindRoom(PlayerInterface * player);
    void OnCreateRoom(PlayerInterface * player, assistx2::Stream * packet);
    void OnDisbandRoom(PlayerInterface * player, assistx2::Stream * packet);
    std::int32_t OnLeaveRoom(PlayerInterface * player, std::int32_t err);
    std::int32_t OnEnterRoom(PlayerInterface * player, assistx2::Stream * packet);
    void EnterNextRoom(PlayerInterface * player,std::int32_t id,std::string type);

private:
    std::map<std::int32_t, RoomInterface * > rooms_;
    std::map<std::string, std::vector<RoomInterface * > > roomgroups_;
    runfastroomscfg_type rfroomscfg_;
    std::map<std::int32_t, RoomInterface *> private_rooms_;
    std::vector<RoomEventListener * > room_listeners_;
    std::queue<RoomInterface *> match_rooms_;
    std::map<std::string, std::vector<RoomInterface * > > waiting_glod_room_groups_;
    std::unique_ptr<GameObj> obj_;
};

#endif
