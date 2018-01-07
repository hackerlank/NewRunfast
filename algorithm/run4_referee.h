#ifndef _RUNFAST4_REFEREE_H_
#define _RUNFAST4_REFEREE_H_

#include "run_referee.h"

enum class PLAY_TYPE
{
    TYPE_DEFAULT,
    TYPE_TWO,
};

class Run4Referee
{
public:
    Run4Referee() {}
    virtual ~Run4Referee() {}

    virtual int Compare(const PlayedCard& left, const PlayedCard& right);
    CardType getCardType(const Cards& cards, int& count, CardInterface::Face& firstFace);
    Cards autoPlay(const PlayedCard& left, const Cards& rightHandCards, std::int32_t nseat_size);
    Cards autoPlay(const Cards& handCards);
    Cards autoPlay_AI(const Cards& rightHandCards, std::int32_t nseat_size);
    bool isMaxCard(const Cards& cards, const Cards& rightHandCards);
    Cards GetMaxCard(const Cards& HandCards);
    bool IsInHandCard(const Cards& playedCard,const Cards& handCards);
    Cards SortCard(const Cards& playedCard, CardType type, int count, CardInterface::Face firstFace);
    bool IsHaveHei3(const Cards& handCards);
    Cards FindTHShunCard(const PlayedCard& left, const Cards cards, int count, size_t handcardscount);
    bool IsHaveCard(std::shared_ptr<CardInterface> card, const Cards cards);
    bool IsHaveCard(CardInterface::Face face, const std::vector<CardInfo>& handCardsInfo);
    void SetType(PLAY_TYPE type);
	bool IsEndfly_EndThreeZone(const Cards& cards, CardInterface::Face& firstFace, CardType& type, int& count_fly);
private:
    std::vector<CardInfo> getCardInfo(const Cards& handCards);
    bool IsPairs(const Cards& cards, int& count_pair, CardInterface::Face& firstFace);
    bool IsPairs_Ex(const Cards& cards, int& count_pair, CardInterface::Face& firstFace);
    bool IsShunZi(const Cards& cards, int& count_shun, CardInterface::Face& firstFace);
    bool IsShunZi_Ex(const Cards& cards, int& count_shun, CardInterface::Face& firstFace);
    bool IsFly(const Cards& cards, int& count_fly, CardInterface::Face& firstFace);
    bool IsBomb(const Cards& cards);
    bool SelectWing(int index, int size, const std::vector<CardInfo>& handCardsInfo, Cards& cards);
    Cards FindShunCard(const PlayedCard& left, const std::vector<CardInfo>& handCardsInfo, int count,size_t handcardscount);
    Cards FindShunCard_Ex(const PlayedCard& left, const std::vector<CardInfo>& handCardsInfo, int count, size_t handcardscount);
    Cards FindThreeZone(const PlayedCard& left, const std::vector<CardInfo>& handCardsInfo,size_t handcardscount);
    Cards FindThree(const PlayedCard& left, const std::vector<CardInfo>& handCardsInfo, size_t handcardscount);
    Cards FindBomb(const PlayedCard& left, const std::vector<CardInfo>& handCardsInfo);
    Cards FindBomb(const std::vector<CardInfo>& handCardsInfo);
    Cards GetWangCards(const std::vector<CardInfo>& handCardsInfo);
    bool IsEndFly(const Cards& cards);
    Cards GetTHShunCards(const Cards cards, std::shared_ptr<CardInterface> firstcard, const std::int32_t count);
    Cards GetTHShunCards_Ex(const Cards cards, std::shared_ptr<CardInterface> firstcard, std::int32_t count);
    bool IsSameType(const Cards& cards);

private:
    PLAY_TYPE type_ = PLAY_TYPE::TYPE_DEFAULT;
};

#endif
