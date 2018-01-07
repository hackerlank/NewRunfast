#ifndef _RUNFAST4_REFEREE_LAIZI_H_
#define _RUNFAST4_REFEREE_LAIZI_H_

#include "run_referee.h"
#include "card_interface.h"
#include <boost/random.hpp>
#include <map>
class Run4RefereeLaizi
{
public:
	Run4RefereeLaizi() {}
	virtual ~Run4RefereeLaizi() {}

	virtual int Compare(const PlayedCard& left, const PlayedCard& right);
	CardType getCardType(const Cards& cards, int& count, CardInterface::Face& firstFace);
	Cards autoPlay(const PlayedCard& left, const Cards& rightHandCards, std::int32_t nseat_size);
	Cards autoPlay(const Cards& handCards);
	Cards autoPlay_AI(const Cards& rightHandCards, std::int32_t nseat_size);
	bool isMaxCard(const Cards& cards, const Cards& rightHandCards);
	Cards GetMaxCard(const Cards& HandCards);
	bool IsInHandCard(const Cards& playedCard, const Cards& handCards);
	Cards SortCard(const Cards& playedCard, CardType type, int count, CardInterface::Face firstFace);
	bool IsHaveHei3(const Cards& handCards);

	int get_laizi() const;
	std::shared_ptr<CardInterface> get_laizi_card()const;
	void set_laizi_card(const std::shared_ptr<CardInterface>& laizi);
	Cards match_Laizi(const PlayedCard& playcard);
	Cards source_Purpose_Laizi(const int& count, const Cards& card_nolaizi, const Cards& card_laizi, const PlayedCard& playcard);

public:		
	bool IsPairs(const Cards& cards, int& count_pair, CardInterface::Face& firstFace);
	bool IsShunZi(const Cards& cards, int& count_shun, CardInterface::Face& firstFace);
	bool IsFly(const Cards& cards, int& count_fly, CardInterface::Face& firstFace);
	bool IsBomb(const Cards& cards, int& count_laizi, CardInterface::Face& firstFace);
	bool IsLaizi(const Cards& cards);
	bool Is_LaiziFly(const Cards& cards, int& count_fly, CardInterface::Face& firstFace);	
	bool Is_Laizi_EndFly(const Cards& cards, CardInterface::Face& firstFace, CardType& type, int& count_fly);
	bool IsEndfly_EndThreeZone(const Cards& cards, CardInterface::Face& firstFace, CardType& type, int& count_fly);
	void generated_laizi();
 	
private:  
	std::vector<CardInfo> getCardInfo(const Cards& handCards);
    bool SelectWing(int index, int size, const std::vector<CardInfo>& handCardsInfo, Cards& cards);	
    Cards FindShunCard(const PlayedCard& left, const std::vector<CardInfo>& handCardsInfo, int count, size_t handcardscount);
    Cards FindThreeZone(const PlayedCard& left, const std::vector<CardInfo>& handCardsInfo,size_t handcardscount);
    Cards FindBomb(const PlayedCard& left, const std::vector<CardInfo>& handCardsInfo);
    Cards FindBomb(const std::vector<CardInfo>& handCardsInfo);	
    Cards GetWangCards(const std::vector<CardInfo>& handCardsInfo);
	bool IsEndFly(const Cards& cards);
	bool getLaizi(const Cards& cards, std::vector< CardInfo>& card_nolaizi, std::vector< CardInfo>& card_laizi);
	bool IsLaizi(const std::vector<CardInfo>& handCardsInfo, Cards& card_nolaizi, Cards& card_laizi);
	bool Is_laizi_Pairs(const Cards& cards, int& count_pair, CardInterface::Face& firstFace);
	bool Find_Laizi_ShunCard(const PlayedCard& left, const std::vector<CardInfo>& hand_nolaizi, const Cards& card_laizi, int count, size_t handcardscount, Cards& cards);
	bool Find_Laizi_Bomb(const PlayedCard& left, const std::vector<CardInfo>& handCardsInfo, Cards& cards, const Cards& card_laizi);
	bool Find_Laizi_Bomb(const std::vector<CardInfo>& handCardsInfo, Cards& cards, const Cards& card_laizi);
	bool Find_Laizi_ThreeZone(const PlayedCard& left, const std::vector<CardInfo>& hand_nolaizi, size_t handcardscount, Cards& cards);
	bool Select_Laiiz_Wing(int index, int left_count, const std::vector<CardInfo>& handCardsInfo, Cards& card_laizi, Cards& cards);

private:
	int laizi_;
	std::shared_ptr<CardInterface> laizi_card_;
};

#endif
