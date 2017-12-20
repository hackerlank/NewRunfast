#ifndef _DATA_CENTER_H_
#define _DATA_CENTER_H_

#include <assistx2/singleton.h>
#include <memory>
#include <string>
#include "GameDataMgr.h"

class Table;
class RoomInterface;
typedef struct UserInfo
{
    RoomInterface* room;
    std::int64_t score;
    GameDataMgr::GameData game_data;
}stUserInfo;

class RunFastRoomMgr;
class DataCenterImpl;
class DataCenter:public Singleton< DataCenter >
{
    friend class Singleton<DataCenter >;
public:
    DataCenter();
    virtual ~DataCenter();

    void init(RunFastRoomMgr* mgr);
    bool ReSetRoom(RoomInterface* room,std::int32_t& owner);
    void RemoveUserInfo(std::int32_t mid);
    void RemoveFullRoom(std::int32_t roomid);
    bool CheckRoomIsFull(std::int32_t mid, std::int32_t roomid);
    bool FindMyRoom(std::int32_t mid, stUserInfo& userinfo);
    std::string MakeRoomInfo(std::int32_t num, std::int32_t owner, std::int32_t playtype, std::int32_t operation,
        std::int32_t winner, std::int32_t createtime, std::int32_t players, std::int32_t proxymid, Table* table);
    void UpdateUserInfo(const std::int32_t mid);
private:
    std::unique_ptr<DataCenterImpl> pImpl_;
};

#endif
