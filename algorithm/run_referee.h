#ifndef _RUNFAST_REFEREE_H_
#define _RUNFAST_REFEREE_H_

#include "card_interface.h"
#include "seat_impl.h"

enum CardType
{
    TYPE_INVALID = -1,
    TYPE_ONE,
    TYPE_ONEPAIR,
    TYPE_PAIRS,
    TYPE_THREE,
    TYPE_THREE_ZONE,
    TYPE_SHUNZI,
    TYPE_FLY,
	TYPE_LAIZI_BOMB, //癞子炸弹
    TYPE_BOMB,	
	TYPE_ALL_LAIZI_BOMB, //纯癞子炸弹
    TYPE_FOUR_TWO, //四带二
    TYPE_FOUR_TWO_FLY, //四带二飞机
    TYPE_FOUR_THREE, //四带三
    TYPE_FOUR_THREE_FLY //四带三飞机
};

struct CardInfo
{
    CardInterface::Face face;
    std::vector<CardInterface::Suit> suits;
};

class PlayedCard
{
public:
    explicit PlayedCard(int id, const Cards& cards, CardType type, int count, CardInterface::Face firstFace) :
        id_(id),
        cards_(cards),
        type_(type),
        count_(count),
        firstFace_(firstFace)
    {

    }
    virtual ~PlayedCard() {}

    int getId() const { return id_; }
    CardType getType() const { return type_; }
    int getCount() const { return count_; }
    CardInterface::Face getFirstFace() const { return firstFace_; }
    Cards getCards() const { return cards_; }
private:
    int id_;
    Cards cards_;
    CardType type_;

    //连续量
    int count_;

    //连续牌的起始牌面
    CardInterface::Face firstFace_;
};

class PDKPokerReferee
{
public:
    PDKPokerReferee() {}
    virtual ~PDKPokerReferee() {}

    virtual int Compare(const PlayedCard& left, const PlayedCard& right);
    CardType getCardType(const Cards& cards, int& count, CardInterface::Face& firstFace, const std::size_t holecards_size);
    Cards autoPlay(const PlayedCard& left,  const Cards& rightHandCards, std::int32_t nseat_size);
    Cards autoPlay(const Cards& handCards);
    Cards autoPlay_AI(const Cards& rightHandCards, std::int32_t nseat_size);
    bool isMaxCard(const Cards& cards, const Cards& rightHandCards);
    Cards GetMaxCard(const Cards& HandCards);
    bool IsInHandCard(const Cards& playedCard,const Cards& handCards);
    Cards SortCard(const Cards& playedCard, CardType type, int count, CardInterface::Face firstFace);
    bool IsHaveHei3(const Cards& handCards);
	bool IsEndFly_EndThreeZone(const Cards& cards, CardInterface::Face& firstFace, CardType& type, int& count_fly);
    std::size_t FindBomb(const Cards& cards);

public:
    bool FOUR_WITH_TWO_ = false;//四带二玩法
    bool FOUR_WITH_THREE_ = false;//四带三玩法

private:
     std::vector<CardInfo> getCardInfo(const Cards& handCards);
    bool IsPairs(const Cards& cards, int& count_pair, CardInterface::Face& firstFace);
    bool IsShunZi(const Cards& cards, int& count_shun, CardInterface::Face& firstFace);
    bool IsFly(const Cards& cards, int& count_fly, CardInterface::Face& firstFace);
    bool IsFourTwo(const Cards& cards, int& count_fly, CardInterface::Face& firstFace, const std::size_t holecards_size);
    bool IsEndFourTwo(const Cards& cards, int& count_fly, CardInterface::Face& firstFace);
    bool IsFourThree(const Cards& cards, int& count_fly, CardInterface::Face& firstFace);
    bool SelectWing(int index, int size, const std::vector<CardInfo>& handCardsInfo, Cards& cards);
    bool SelectFourThreeWing(int index, int size, const std::vector<CardInfo>& handCardsInfo, Cards& cards);
    Cards FindShunCard(const PlayedCard& left, const std::vector<CardInfo>& handCardsInfo, int count,size_t handcardscount);
    Cards FindThreeZone(const PlayedCard& left, const std::vector<CardInfo>& handCardsInfo,size_t handcardscount);
    Cards FindFourTwo(const PlayedCard& left, const std::vector<CardInfo>& handCardsInfo, size_t handcardscount);
    Cards FindFourTwoFly(const PlayedCard& left, const std::vector<CardInfo>& handCardsInfo, size_t handcardscount);
    Cards FindFourThree(const PlayedCard& left, const std::vector<CardInfo>& handCardsInfo);
    Cards FindFourThreeFly(const PlayedCard& left, const std::vector<CardInfo>& handCardsInfo);
    Cards FindBomb(const PlayedCard& left, const std::vector<CardInfo>& handCardsInfo);
    Cards FindBomb(const std::vector<CardInfo>& handCardsInfo);
    bool IsEndFly(const Cards& cards);
};

#endif
