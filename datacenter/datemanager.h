#pragma once

#include <memory>
#include <assistx2/singleton.h>
#include <string>

class ThreadPool;
class PrivateRoom;
class DataManagerImpl;
class PlayerInterface;
class DataManager :public Singleton<DataManager>
{
private:
    friend class DefaultBuilder<DataManager>;
    DataManager();
    virtual ~DataManager();
public:
    void Init();
    void PlayingPlayer(PrivateRoom* room);
    void CreateRoomPlayer(PrivateRoom* room,bool isfree);
    void MingTang(const PlayerInterface *player, const std::int32_t mingtang, const PrivateRoom* play_type);
    void BombSpringStatistics(PrivateRoom* room);
    void ExecSql(const std::string& sql);
    void Test();
private:
    std::unique_ptr< DataManagerImpl > pImpl_;
};
