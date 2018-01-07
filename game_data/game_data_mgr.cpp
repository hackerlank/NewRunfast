#include "GameDataMgr.h"
#include <map>
#include <sstream>

class GameDataMgrImpl
{
public:
    GameDataMgrImpl();
    virtual ~GameDataMgrImpl();
public:
    std::map<int32_t, GameDataMgr::GameData> game_data_;
};

GameDataMgr::GameDataMgr() :
    pImpl_(new GameDataMgrImpl())
{

}

GameDataMgr::~GameDataMgr()
{

}

void GameDataMgr::UpdateGameData(int32_t seatno, int32_t score, int32_t bomb_count, bool isWin)
{
    auto iter = pImpl_->game_data_.find(seatno);
    if (iter != pImpl_->game_data_.end())
    {
        iter->second.game_count += 1;
        iter->second.bomb_count += bomb_count;
        iter->second.sum_scroe += score;
        if (isWin)
        {
            iter->second.win_count += 1;
        }
        else
        {
            iter->second.lost_count += 1;
        }
        if (iter->second.max_score < score)
        {
            iter->second.max_score = score;
        }
    }
    else
    {
        GameDataMgr::GameData gameData;
        gameData.bomb_count = bomb_count;
        gameData.game_count = 1;
        gameData.max_score = score;
        gameData.sum_scroe = score;
        if (isWin)
        {
            gameData.win_count = 1;
            gameData.lost_count = 0;
        }
        else
        {
            gameData.lost_count = 1;
            gameData.win_count = 0;
        }
        pImpl_->game_data_.insert(std::make_pair(seatno, gameData));
    }
}

void GameDataMgr::ResetGameData(int32_t seatno,const GameData& game_data)
{
    GameData gameData;
    gameData.bomb_count = game_data.bomb_count;
    gameData.game_count = game_data.game_count;
    gameData.max_score = game_data.max_score;
    gameData.sum_scroe = game_data.sum_scroe;
    gameData.win_count = game_data.win_count;
    gameData.lost_count = game_data.lost_count;
   
    pImpl_->game_data_.insert(std::make_pair(seatno, gameData));
}

GameDataMgr::GameData GameDataMgr::GetGameData(int32_t seatno)
{
    auto iter = pImpl_->game_data_.find(seatno);
    if (iter != pImpl_->game_data_.end())
    {
        return iter->second;
    }

    return GameDataMgr::GameData();
}

std::string GameDataMgr::MakeGameData(std::int32_t mid,int32_t score, int32_t sum_scroe, int32_t bomb_scroe)
{
    std::stringstream ss;
    ss << mid;
    ss << ",";
    ss << score;
    if (bomb_scroe > 0)
    {
        ss << "+" << bomb_scroe;
    }
    else if (bomb_scroe < 0)
    {
        ss << "-" << -bomb_scroe;
    }
    ss << ",";
    ss << sum_scroe;
    ss << ";";

    return ss.str();
}

std::string GameDataMgr::MakeRoomData(std::int32_t mid, const GameData& game_data)
{
    std::stringstream ss;
    ss << mid << ":";
    ss << game_data.game_count << ",";
    ss << game_data.bomb_count << ",";
    ss << game_data.max_score << ",";
    ss << game_data.win_count << ",";
    ss << game_data.lost_count << ",";
    ss << game_data.sum_scroe;
    ss << ";";

    return ss.str();
}

void GameDataMgr::ClearAll()
{
    pImpl_->game_data_.clear();
}

GameDataMgrImpl::GameDataMgrImpl()
{
}

GameDataMgrImpl::~GameDataMgrImpl()
{
}