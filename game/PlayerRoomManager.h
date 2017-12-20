#ifndef PLAYER_ROOM_MANAGER_H_
#define PLAYER_ROOM_MANAGER_H_

#include <memory>
#include <assistx2/singleton.h>

class PlayerRoomManagerImpl;
class PlayerRoomManager :public Singleton<PlayerRoomManager>
{
public:
    PlayerRoomManager();
    virtual ~PlayerRoomManager();

    void Init();

    void UpdateConfig();
    void UpdateCreatorRoomList(const std::int32_t mid);
    bool CanCreateRoom(const std::int32_t mid);
    void AddRoom(const std::int32_t mid,const std::int32_t roomid);
    void DeleteRoom(const std::int32_t mid, const std::int32_t roomid);
    bool IsOwnerRoom(const std::int32_t mid, const std::int32_t roomid);
private:
    std::unique_ptr< PlayerRoomManagerImpl > pImpl_;
};



#endif // !PLAYER_ROOM_MANAGER_H_

