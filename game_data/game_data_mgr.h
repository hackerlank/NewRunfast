#ifndef _GAMEDATA_MGR_H_
#define _GAMEDATA_MGR_H_

#include <memory>
#include <string>

class GameDataMgrImpl;
class GameDataMgr
{
public:
    struct GameData
    {
        int32_t game_count;  //��Ϸ����
        int32_t bomb_count; //ը������
        int32_t max_score;    //������ߵ÷�
        int32_t win_count;    //ʤ����
        int32_t lost_count;    //�ܾ���
        int32_t sum_scroe;    //�ۼƻ���
    };
public:
    GameDataMgr();
    virtual ~GameDataMgr();
    void UpdateGameData(int32_t seatno, int32_t score, int32_t bomb_count, bool isWin);
    GameData GetGameData(int32_t seatno);
    std::string MakeGameData(std::int32_t mid,std::int32_t score, int32_t sum_scroe, int32_t bomb_scroe);
    std::string MakeRoomData(std::int32_t mid, const GameData& game_data);
    void ResetGameData(int32_t seatno,const GameData& game_data);
    void ClearAll();
private:
    std::unique_ptr< GameDataMgrImpl > pImpl_;
};

#endif
