#ifndef _PLAY_LOGIC_H_
#define _PLAY_LOGIC_H_

#include "run_referee.h"

class PlayLogic
{
public:
    PlayLogic() {}
    virtual ~PlayLogic() {}

    virtual int Compare(const PlayedCard& left, const PlayedCard& right);
    CardType getCardType(const Cards& cards, int& count, CardInterface::Face& firstFace);
    Cards autoPlay(const PlayedCard& left, const Cards& rightHandCards, std::int32_t nseat_size);
    Cards autoPlay(const Cards& handCards);
    Cards autoPlay_AI(const Cards& rightHandCards, std::int32_t nseat_size);
    bool isMaxCard(const Cards& cards, const Cards& rightHandCards);
    Cards GetMaxCard(const std::vector<CardInfo>& handCardsInfo);
    bool IsInHandCard(const Cards& playedCard, const Cards& handCards);
    Cards SortCard(const Cards& playedCard, CardType type, int count, CardInterface::Face firstFace);
private:
    std::vector<CardInfo> getCardInfo(const Cards& handCards);
    bool IsPairs(const Cards& cards, int& count_pair, CardInterface::Face& firstFace);
    bool IsShunZi(const Cards& cards, int& count_shun, CardInterface::Face& firstFace);
    bool IsFly(const Cards& cards, int& count_fly, CardInterface::Face& firstFace);
    bool SelectWing(int index, int size, const std::vector<CardInfo>& handCardsInfo, Cards& cards);
    Cards FindShunCard(const PlayedCard& left, const std::vector<CardInfo>& handCardsInfo, int count, size_t handcardscount);
    Cards FindThreeZone(const PlayedCard& left, const std::vector<CardInfo>& handCardsInfo, size_t handcardscount);
    Cards FindBomb(const PlayedCard& left, const std::vector<CardInfo>& handCardsInfo);
    Cards FindBomb(const std::vector<CardInfo>& handCardsInfo);
    Cards FindOne(const Cards& cards, const std::vector<CardInfo>& handCardsInfo);
    Cards FindShunCard(const std::vector<CardInfo>& handCardsInfo, int count);
    bool CheckShunCard(const Cards& cards, const std::vector<CardInfo>& handCardsInfo); 
    Cards FindThreeZone(const std::vector<CardInfo>& handCardsInfo);
    Cards FindPairs(const std::vector<CardInfo>& handCardsInfo);
    bool IsEndFly(const Cards& cards);
    bool FindCard(const CardInterface::Face face,const Cards& cards,const std::int32_t num);
    Cards GetMinCard(const std::vector<CardInfo>& handCardsInfo);
};

#endif
