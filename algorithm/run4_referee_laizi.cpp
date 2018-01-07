#include "run4_referee_laizi.h"
#include <algorithm>

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

#define _RUNFAST_DEBUG_

int Run4RefereeLaizi::Compare(const PlayedCard& left, const PlayedCard& right)
{
	auto leftType = left.getType();
	auto rightType = right.getType();
	if (leftType == TYPE_INVALID || rightType == TYPE_INVALID)
	{
		return 0;
	}

	if (leftType == TYPE_BOMB && rightType == TYPE_BOMB)
	{
		if (left.getCards()[0]->getSuit() == CardInterface::NONESUIT)
		{
			return 1;
		}
		else if (right.getCards()[0]->getSuit() == CardInterface::NONESUIT)
		{
			return -1;
		}
		else
		{
			if (left.getCards().size() == right.getCards().size())
			{
				return left.getCards()[0]->getFace() < right.getCards()[0]->getFace() ? -1 : 1;
			}
			else
			{
				return left.getCards().size() < right.getCards().size() ? -1 : 1;
			}
		}
	}
	else if (leftType == TYPE_LAIZI_BOMB && rightType == TYPE_LAIZI_BOMB)
	{
		return left.getFirstFace() > right.getFirstFace() ? 1 : -1;
		/*Cards left_nolaizi;
		Cards left_laizi;
		auto left_cardsInfo = getCardInfo(left.getCards());
		Cards right_nolaizi;
		Cards right_laizi;
		auto right_cardsInfo = getCardInfo(right.getCards());
		if (IsLaizi(left_cardsInfo, left_nolaizi, left_laizi) && IsLaizi(right_cardsInfo, right_nolaizi, right_laizi))
		{
			return left_nolaizi.front()->getFace() > right_nolaizi.front()->getFace() ? 1 : -1;
		}		*/
	}
	else if (leftType == TYPE_ALL_LAIZI_BOMB)
	{
		return 1;
	}
	else if (rightType == TYPE_ALL_LAIZI_BOMB)
	{
		return -1;
	}
	else if (TYPE_LAIZI_BOMB<= leftType && leftType <= TYPE_ALL_LAIZI_BOMB && 
		rightType != TYPE_ALL_LAIZI_BOMB &&
		rightType != TYPE_BOMB &&
		rightType != TYPE_LAIZI_BOMB)
	{
		return 1;
	}
	else if (leftType != TYPE_LAIZI_BOMB  && leftType != TYPE_BOMB && leftType != TYPE_ALL_LAIZI_BOMB &&
		TYPE_ALL_LAIZI_BOMB >= rightType && rightType >= TYPE_LAIZI_BOMB)
	{
		return -1;
	}
	else if (leftType == TYPE_LAIZI_BOMB && rightType == TYPE_BOMB)
	{
		return -1;
	}
	else if (leftType == TYPE_BOMB && rightType == TYPE_LAIZI_BOMB)
	{
		return 1;
	}
	else
	{
		switch (leftType)
		{
		case TYPE_ONE:
		case TYPE_ONEPAIR:
		case TYPE_THREE:
		case TYPE_PAIRS:
		case TYPE_SHUNZI:
		case TYPE_THREE_ZONE:
			return left.getFirstFace() > right.getFirstFace() ? 1 : -1;
		case TYPE_FLY:
			return left.getFirstFace() > right.getFirstFace() ? 1 : -1;
		default:
			return 0;
		}
	}

	return 0;
}

CardType Run4RefereeLaizi::getCardType(const Cards& cards, int& count, CardInterface::Face& firstFace)
{
	if (0 == cards.size())
	{
		count = 0;
		return TYPE_INVALID;
	}
	Cards clone(cards);
	std::vector<CardInfo> card_laizi;
	int count_laizi;
	std::sort(clone.begin(), clone.end());

	DLOG(INFO) << "Run4Referee::getCardType() cards:=" << clone;

	count = 0;

	auto size = clone.size();
	if (size == 1)
	{
		firstFace = cards.back()->getFace();
		return TYPE_ONE;
	}
	else if (size == 2)
	{
		if (clone[0]->getFace() == clone[1]->getFace())
		{
			firstFace = cards[0]->getFace();
			return TYPE_ONEPAIR;
		}
		if (clone[0]->getFace() == laizi_ ||
			clone[1]->getFace() == laizi_)
		{
			firstFace = cards[0]->getFace() != laizi_ ? cards[0]->getFace() : cards[1]->getFace();
			return TYPE_ONEPAIR;
		}
	}
	else if (IsBomb(clone, count_laizi, firstFace) && size == 4)
	{
		if (4 == count_laizi)//static_cast<int>(cards.size()))
		{
			return TYPE_ALL_LAIZI_BOMB;
		}
		else if (count_laizi > 0 && count_laizi < 4)
		{
			return TYPE_LAIZI_BOMB;
		}
		else if (0 == count_laizi)
		{
			return TYPE_BOMB;
		}
	}
	else if (IsShunZi(clone, count, firstFace))
	{
		return TYPE_SHUNZI;
	}
	else if (IsPairs(clone, count, firstFace))
	{
		return TYPE_PAIRS;
	}
	else if (IsFly(clone, count, firstFace))
	{
		if (count == 1)
		{
			return TYPE_THREE_ZONE;
		}
		else if (count > 1)
		{
			return TYPE_FLY;
		}
		else
		{
			return TYPE_INVALID;
		}
	}
	else
	{
		return TYPE_INVALID;
	}

	return TYPE_INVALID;
}

bool Run4RefereeLaizi::isMaxCard(const Cards& cards, const Cards& rightHandCards)
{
	Cards clone(rightHandCards);
	std::sort(clone.begin(), clone.end(), ComparePoker);
	if (cards[0]->getFace() != clone[clone.size() - 1]->getFace())
	{
		return false;
	}

	return true;
}

Cards Run4RefereeLaizi::GetMaxCard(const Cards& HandCards)
{
	Cards clone(HandCards);
	std::sort(clone.begin(), clone.end(), ComparePoker);

	Cards cards;
	auto card = CardFactory::MakePokerCard(clone[clone.size() - 1]->getFace(), clone[clone.size() - 1]->getSuit());
	cards.push_back(card);

	return cards;
}

Cards  Run4RefereeLaizi::autoPlay(const PlayedCard& left, const Cards& rightHandCards, std::int32_t nseat_size)
{
	if (0 == rightHandCards.size())
	{
		return Cards();
	}
	Cards clone(rightHandCards);
	std::sort(clone.begin(), clone.end(), ComparePoker);

	DLOG(INFO) << "Run4Referee::autoPlay() PlayedCards:=" << left.getCards()
		<< " type:=" << left.getType() << " count:= " << left.getCount() << " fristface:= " << left.getFirstFace() <<" LAIZI:= " << laizi_card_->getName();
	DLOG(INFO) << "Run4Referee::autoPlay()   HandCards:=" << clone;

	Cards cards;
	auto handCardsInfo = getCardInfo(rightHandCards);
	std::sort(handCardsInfo.begin(), handCardsInfo.end(), [](const CardInfo& left, const CardInfo& right) {
		return left.face < right.face;
	});

	Cards bombCards = FindBomb(handCardsInfo);

	if (left.getType() == TYPE_ALL_LAIZI_BOMB)
	{
		return cards;
	}
	if (bombCards.size() == 0 &&
		left.getCards().size() > rightHandCards.size() &&
		left.getType() != TYPE_THREE_ZONE &&
		left.getType() != TYPE_FLY 
		)
	{
		return cards;
	}

	if (left.getType() != TYPE_LAIZI_BOMB &&
		left.getType() != TYPE_BOMB &&
		left.getType() != TYPE_ALL_LAIZI_BOMB &&
		rightHandCards.size() == bombCards.size())
	{
		return bombCards;
	}

	switch (left.getType())
	{
	case TYPE_ONE:
	{
		if (nseat_size == 1)
		{
			Cards max_cards = GetMaxCard(rightHandCards);
			if (max_cards[0]->getFace() > left.getCards()[0]->getFace())
			{
				return max_cards;
			}
		}
		for (auto iter : handCardsInfo)
		{
			if (iter.face > left.getCards()[0]->getFace())
			{
				std::sort(iter.suits.begin(), iter.suits.end());
				auto card = CardFactory::MakePokerCard(iter.face, iter.suits[0]);
				cards.push_back(card);
				return cards;
			}
		}
	}
	break;
	case TYPE_ONEPAIR:
	{
		Cards card_nolaizi;
		Cards card_laizi;
		bool flag = IsLaizi(handCardsInfo, card_nolaizi, card_laizi);
		for (auto iter : handCardsInfo)
		{
			if (iter.suits.size() >= 2 &&
				iter.face > left.getCards()[0]->getFace()
				)
			{
				std::sort(iter.suits.begin(), iter.suits.end());
				auto card = CardFactory::MakePokerCard(iter.face, iter.suits[0]);
				cards.push_back(card);
				card = CardFactory::MakePokerCard(iter.face, iter.suits[1]);
				cards.push_back(card);
				return cards;
			}
			if (iter.face > left.getFirstFace() && iter.face != laizi_ && flag == true)
			{
				std::sort(iter.suits.begin(), iter.suits.end());
				auto card = CardFactory::MakePokerCard(iter.face, iter.suits[0]);
				cards.push_back(card);
				card = card_laizi.front();
				cards.push_back(card);
				return cards;
			}
		}
	}
	break;
	case TYPE_THREE_ZONE:
		cards = FindThreeZone(left, handCardsInfo, rightHandCards.size());
		break;
	case TYPE_PAIRS:
		cards = FindShunCard(left, handCardsInfo, 2, rightHandCards.size());
		break;
	case TYPE_SHUNZI:
		cards = FindShunCard(left, handCardsInfo, 1, rightHandCards.size());
		break;
	case TYPE_FLY:
		cards = FindShunCard(left, handCardsInfo, 3, rightHandCards.size());
		break;
	case TYPE_LAIZI_BOMB:
		cards = FindBomb(left, handCardsInfo);
		if (cards.size() > 0)
		{
			return cards;
		}
		else
		{
			for (auto iter : handCardsInfo)
			{
				if (iter.suits.size() >= 4)
				{
					for (auto suit_iter : iter.suits)
					{
						auto card = CardFactory::MakePokerCard(iter.face, suit_iter);
						cards.push_back(card);
					}
					return cards;
				}
			}
		}		
		return cards;
	case TYPE_BOMB:
		for (auto iter : handCardsInfo)
		{
			if (iter.suits.size() >= 4 && iter.face > left.getFirstFace())
			{
				for (auto suit_iter : iter.suits)
				{
					auto card = CardFactory::MakePokerCard(iter.face, suit_iter);
					cards.push_back(card);
				}
				return cards;
			}
			else if (iter.suits.size() >= 4 && iter.face == laizi_)
			{
				for (auto suit_iter : iter.suits)
				{
					auto card = CardFactory::MakePokerCard(iter.face, suit_iter);
					cards.push_back(card);
				}
				return cards;
			}
		}
		return cards;
	default:
		break;
	}

	if (cards.size() == 0 && 
		bombCards.size() == 4)
	{
		DLOG(INFO) << "bombCards.size() = " << bombCards.size() << ", bombCards = " << bombCards;
		return bombCards;
	}
	if (cards.size() == rightHandCards.size() &&
		cards.size() % 5 == 0 && 
		4 == bombCards.size())
	{
		DLOG(INFO) << "bombCards.size() = " << bombCards.size() << ", bombCards = " << bombCards;
		return bombCards;
	}

	DLOG(INFO) << "Run4Referee::autoPlay()   ReturnCards:=" << cards.size();
	return cards;
}

Cards Run4RefereeLaizi::autoPlay(const Cards& handCards)
{
	if (0 == handCards.size())
	{
		return Cards();
	}
	int count = 0;
	CardInterface::Face firstface;
	CardType type;
	Cards clone(handCards);
	Cards cards;
	auto handCardsInfo = getCardInfo(handCards);

	auto cards_type = getCardType(clone, count, firstface);
	DLOG(INFO) << "getCardType = " << cards_type;

	/*if (IsBomb(handCards, size, firstface))
	{
		DLOG(INFO) << "handCards = " << clone;
		return clone;
	}*/
	if (cards_type != TYPE_INVALID)
	{
		if (FindBomb(handCardsInfo).size() == 0 ||
			FindBomb(handCardsInfo).size() == handCards.size()
			)
		{
			return clone;
		}
		else
		{
			Cards cards;
			cards.push_back(clone[0]);
			return cards;
		}
	}
	else
	{
		std::sort(clone.begin(), clone.end(), ComparePoker);
		if (clone.size() == 3 &&
			clone[0]->getFace() == clone[2]->getFace()
			)
		{
			return clone;
		}
		else if (clone.size() == 4 &&
			(clone[0]->getFace() == clone[2]->getFace() ||
				clone[1]->getFace() == clone[3]->getFace()
				)
			)
		{
			return clone;
		}
		else if (IsEndFly(clone) == true)
		{
			return clone;
		}
		else if (Is_Laizi_EndFly(clone, firstface, type, count))
		{
			return clone;
			/*if (FindBomb(handCardsInfo).size() == 0 )
			{
				return clone;
			}
			else
			{
				Cards cards;
				cards.push_back(clone[0]);
				return cards;
			}*/
		}
		else if (cards_type == TYPE_LAIZI_BOMB && handCards.size() == 4)
		{
			return clone;
		}
		else if (clone.size() >= 1)
		{
			cards.push_back(clone[0]);
		}
	}

	return cards;
}

Cards Run4RefereeLaizi::autoPlay_AI(const Cards& rightHandCards, std::int32_t nseat_size)
{
	Cards cards;
	if (rightHandCards.empty())
	{
		return cards;
	}
	if (nseat_size == 1)
	{
		return GetMaxCard(rightHandCards);
	}
	Cards clone(rightHandCards);
	auto handCardsInfo = getCardInfo(clone);
	std::sort(handCardsInfo.begin(), handCardsInfo.end(), [](const CardInfo& left, const CardInfo& right) {
		return left.face < right.face;
	});
	for (auto suit_iter : handCardsInfo[0].suits)
	{
		auto card = CardFactory::MakePokerCard(handCardsInfo[0].face, suit_iter);
		cards.push_back(card);
	}
	if (cards.size() == 3)
	{
		std::sort(clone.begin(), clone.end(), ComparePoker);
		for (auto iter : clone)
		{
			if (iter->getFace() == handCardsInfo[0].face)
			{
				continue;
			}
			cards.push_back(iter);
			if (cards.size() == 5u)
			{
				break;
			}
		}
	}

	return cards;
}

bool Run4RefereeLaizi::IsShunZi(const Cards& cards, int& count_shun, CardInterface::Face& firstFace)
{
	count_shun = 1;
	size_t count_laizi = 0;
	std::vector< CardInfo> card_nolaizi;
	std::vector< CardInfo> card_laizi;
	auto size = cards.size();
	Cards allcards(cards);
	std::sort(allcards.begin(), allcards.end(), ComparePoker);

	if (size < 5)
	{
		return false;
	}

	if (allcards[size - 1]->getFace() == CardInterface::Two && laizi_ != CardInterface::Two)
	{
		return false;
	}

	getLaizi(allcards, card_nolaizi, card_laizi);
	std::sort(card_nolaizi.begin(), card_nolaizi.end(), [](const CardInfo& left, const CardInfo& right) {
		return left.face < right.face;
	});
	count_laizi = card_laizi.size();
	if (0 == card_nolaizi.size())
	{
		DLOG(ERROR) << "Run4Referee_laizi::IsLaiZi()";
		return false;
	}

	for (size_t i = 1; i < card_nolaizi.size(); ++i)
	{
		if ((card_nolaizi[i - 1].face + 1) == card_nolaizi[i].face)
		{
			count_shun += 1;
		}
		else if (count_laizi > 0)
		{
			if ((card_nolaizi[i - 1].face + 2) == card_nolaizi[i].face)
			{
				count_shun += 2;
				count_laizi -= 1;
				continue;
			}
			switch (count_laizi)
			{
			case 2:
				if ((card_nolaizi[i - 1].face + 3) == card_nolaizi[i].face)
				{
					count_shun += 3;
					count_laizi -= 2;
				}
				else
				{
					return false;
				}break;
			case 3:
				if ((card_nolaizi[i - 1].face + 4) == card_nolaizi[i].face)
				{
					count_shun += 4;
					count_laizi -= 3;
				}
				else if ((card_nolaizi[i - 1].face + 3) == card_nolaizi[i].face)
				{
					count_shun += 3;
					count_laizi -= 2;
				}
				else
				{
					return false;
				}break;
			case 4:
				if ((card_nolaizi[i - 1].face + 5) == card_nolaizi[i].face)
				{
					count_shun += 5;
					count_laizi -= 4;
				}
				else if ((card_nolaizi[i - 1].face + 4) == card_nolaizi[i].face)
				{
					count_shun += 4;
					count_laizi -= 3;
				}
				else if ((card_nolaizi[i - 1].face + 3) == card_nolaizi[i].face)
				{
					count_shun += 3;
					count_laizi -= 2;
				}
				else
				{
					return false;
				}break;
			default:
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	if (card_nolaizi.back().face + count_laizi > CardInterface::Ace)
	{
		count_shun += count_laizi;
		firstFace = static_cast<CardInterface::Face>(CardInterface::Two - count_shun);		
	}
	else
	{  
		count_shun += count_laizi;
		firstFace = card_nolaizi[0].face;		
	}
		
	return true;
}

bool Run4RefereeLaizi::IsPairs(const Cards& cards, int& count_pair, CardInterface::Face& firstFace)
{
	count_pair = 0;
	auto size = cards.size();
	Cards allcards(cards);
	auto handCardsInfo = getCardInfo(cards);
	std::vector< CardInterface::Face > pairFaces;
	std::vector< CardInterface::Face > oneFaces;
	
	std::sort(allcards.begin(), allcards.end(), ComparePoker);

	for (auto it = handCardsInfo.begin(); it != handCardsInfo.end(); ++it)
	{
		if (it->face == CardInterface::Two && it->face != laizi_)
		{
			return false;
		}
	}
	if (size < 4)
	{
		return false;
	}

	if ((size % 2) != 0)
	{
		return false;
	}

	//癞子连对
	if (IsLaizi(allcards))
	{
		if (Is_laizi_Pairs(cards, count_pair, firstFace))
		{
			return true;
		}
	}

	for (size_t i = 1; i < size - 2; i += 2)
	{
		if (allcards[i - 1]->getFace() == allcards[i]->getFace() &&
			(allcards[i]->getFace() + 1) == allcards[i + 1]->getFace()
			)
		{
			count_pair += 1;
		}
		else
		{
			return false;
		}
	}
	if (allcards[size - 1]->getFace() == allcards[size - 2]->getFace())
	{
		count_pair += 1;
	}
	else
	{
		return false;
	}
	firstFace = allcards[0]->getFace();
	return true;
}

bool Run4RefereeLaizi::Is_laizi_Pairs(const Cards& cards, int& count_pair, CardInterface::Face& firstFace)
{
	count_pair = 0;
	std::size_t count_laizi = 0;
	std::vector<CardInfo> card_laizi;
	std::vector<CardInfo> card_nolaizi;
	auto size = cards.size();
	Cards allcards(cards);
	auto handCardsInfo = getCardInfo(cards);
	std::vector< CardInterface::Face > pairFaces;
	std::vector< CardInterface::Face > oneFaces;

	std::sort(allcards.begin(), allcards.end(), ComparePoker);

	if (getLaizi(allcards, card_nolaizi, card_laizi) == false)
	{
		return false;
	}
	for (auto iter : handCardsInfo)
	{
		if (iter.face == CardInterface::Two && laizi_ != CardInterface::Two)
		{
			return false;
		}
		if (iter.suits.size() > 2u && iter.face != laizi_)
		{
			return false;
		}
		if (iter.suits.size() == 2u && iter.face != laizi_)
		{
			pairFaces.push_back(iter.face);
		}
		else if (iter.face != laizi_)
		{
			oneFaces.push_back(iter.face);
		}
	}
	if (oneFaces.size() > card_laizi.size())
	{
		return false;
	}
	std::vector< CardInterface::Face > one_pairFaces(oneFaces);
	for (auto iter : pairFaces)
	{
		one_pairFaces.push_back(iter);
	}
	std::sort(one_pairFaces.begin(), one_pairFaces.end());
	for(auto iter : one_pairFaces)
		DLOG(INFO) << "one_pairFaces = " << iter;

	count_laizi = card_laizi.size();
	count_laizi -= oneFaces.size();

	for (std::size_t i = 1; i < one_pairFaces.size(); ++i)
	{
		if ((one_pairFaces[i - 1] + 1) == one_pairFaces[i])
		{
			count_pair += 2;
		}
		else if (count_laizi >= 2 && count_laizi % 2 == 0)
		{
			switch (count_laizi)
			{
			case 2:
				if ((one_pairFaces[i - 1] + 2) == one_pairFaces[i])
				{
					count_pair += 2;
					count_laizi -= 2;
				}
				else { return false; }
				break;

			case 4:
				if (count_laizi == 4 && (one_pairFaces[i - 1] + 3) == one_pairFaces[i])
				{
					count_pair += 3;
					count_laizi -= 4;
				}
				else if ((one_pairFaces[i - 1] + 2) == one_pairFaces[i])
				{
					count_pair += 2;
					count_laizi -= 2;
				}
				else { return false; }
				break;
			default:
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	
	if (oneFaces.size() == count_laizi ||
		one_pairFaces.size() == card_laizi.size() ||
		(count_laizi + 2) == card_laizi.size() ||	
		(count_laizi + 4) == card_laizi.size() ||
		count_laizi % 2 == 0
		)
	{
		if ( (one_pairFaces.back() + count_laizi / 2) > CardInterface::Ace && count_laizi % 2 == 0)
		{
			firstFace = static_cast<CardInterface::Face>(CardInterface::Two - size / 2);
		}
		else
		{
			firstFace = one_pairFaces[0];
		}
		count_pair = cards.size() / 2;
		return true;
	}

	firstFace = (CardInterface::Face)0;
	return false;
}

bool Run4RefereeLaizi::IsFly(const Cards& cards, int& count_fly, CardInterface::Face& firstFace)
{
	count_fly = 0;
	auto size = cards.size();

	auto handCardsInfo = getCardInfo(cards);
	std::sort(handCardsInfo.begin(), handCardsInfo.end(), [](const CardInfo& left, const CardInfo& right) {
		return left.face < right.face;
	});

	std::vector< CardInterface::Face > threeFaces;

	if (size < 5)
	{
		return false;
	}

	if ((size % 5) != 0)
	{
		return false;
	}

	//癞子飞机
	if (IsLaizi(cards))
	{
		if (Is_LaiziFly(cards, count_fly, firstFace))
		{
			return true;
		}
	}

	for (auto iter : handCardsInfo)
	{
		if (iter.suits.size() >= 3u)
		{
			threeFaces.push_back(iter.face);
		}
	}

	if (threeFaces.size() == 0)
	{
		return false;
	}

	std::sort(threeFaces.begin(), threeFaces.end());
	auto count = 1;
	auto index = 0;
	for (size_t f = 1; f < threeFaces.size(); ++f)
	{
		if ((threeFaces[f - 1] + 1) == threeFaces[f])
		{
			index = f;
			count += 1;
			if (static_cast<std::int32_t>(size) == (count * 5))
			{
				break;
			}
			continue;
		}
		else
		{
			if (static_cast<std::int32_t>(size) == (count * 5))
			{
				break;
			}
			else
			{
				index = 0;
				count = 1;
			}
		}
	}
	count_fly = count;
	if (static_cast<std::int32_t>(size) != (count_fly * 5))
	{
		return false;
	}
	firstFace = threeFaces[index - count + 1];
	return true;
}

bool Run4RefereeLaizi::Is_LaiziFly(const Cards& cards, int& count_fly, CardInterface::Face& firstFace)
{
    firstFace = CardInterface::Face::nAce;
	std::map<CardInterface::Face, std::size_t> card_slot;	
	std::vector<int> need_laizi;
	need_laizi.resize(16);
	auto handCardsInfo = getCardInfo(cards);	
	Cards card_laizi;
	Cards card_nolaizi;
	if (!IsLaizi(handCardsInfo, card_nolaizi, card_laizi))
	{				
		DLOG(INFO) << "The number of non-Laizi: " << card_nolaizi.size() << "The number of Laizi: " << card_laizi.size();
		return false;
	}
	auto laizi_size = card_laizi.size();
	auto nolaizi_info = getCardInfo(card_nolaizi);
    //auto laizi_info = getCardInfo(card_laizi);

	std::sort(nolaizi_info.begin(), nolaizi_info.end(), [](const CardInfo& left, const CardInfo& right) {
		return left.face < right.face;
	});
	std::sort(handCardsInfo.begin(), handCardsInfo.end(), [](const CardInfo& left, const CardInfo& right) {
		return left.face < right.face;
	});

	for (int i = 1; i <= 12; i++)
	{
		card_slot.insert(std::pair<CardInterface::Face, std::size_t>(static_cast<CardInterface::Face>(i), 0));
	}
	for (std::size_t t = 0; t < nolaizi_info.size(); ++t)
	{
		card_slot[nolaizi_info[t].face] = nolaizi_info[t].suits.size();
	}
	if (cards.size() == 5u)
	{
		count_fly = 1;
		if (laizi_size >= 3u)
		{
			firstFace = handCardsInfo.back().face;
			return true;
		}
		else if (laizi_size >= 2u)
		{			
			firstFace = nolaizi_info.back().face;
			return true;	
		}
	}
	else if (cards.size() == 10u)
	{
		count_fly = 2;
	}
	else if (cards.size() == 15u)
	{
		count_fly = 3;
	}
	else
	{
		count_fly = 0;
		return false;
	}
	for (int i = static_cast<int>(nolaizi_info.front().face); i <= static_cast<int>(nolaizi_info.back().face) - count_fly + 1; i++)
	{
		for (int k = i; k < count_fly + i; k++)
		{
			if (card_slot[(CardInterface::Face)k] < 3)
			{
				need_laizi[i] += 3 - card_slot[(CardInterface::Face)k];
			}
		}
	}

	std::vector<int> tmp;
	for (std::size_t i = 1; i < need_laizi.size(); i++)
	{
		if (need_laizi[i] != 0u)
		{
			tmp.push_back(need_laizi[i]);
		}
	}	
	if (tmp.size() == 0u)
	{
		return false;
	}

	//min最少需要的癞子张数
	std::size_t min = *std::min_element(tmp.begin(), tmp.end());
	DLOG(INFO) << "The minimum need of Laizi: " << min << " The number of Laizi : " << laizi_size;
	if (laizi_size < min)
	{
		return false;
	}	
	for (std::size_t i = 0; i < need_laizi.size(); i++)
	{
		if ( (min == static_cast<std::size_t>(need_laizi[i]) || laizi_size == static_cast<std::size_t>(need_laizi[i]) ) && 
			(i + count_fly) <= CardInterface::Face::Two)
		{
			firstFace = static_cast<CardInterface::Face>(i);
			DLOG(INFO) << " firstFace = " << i << " The minimum need of Laizi: " << need_laizi[i] << " The number of Laizi: " << laizi_size;
		}
		DLOG(INFO) << " need laizi:= " << need_laizi[i] << " laizi size:= " << laizi_size << " min " << min;
	}
	
	if (firstFace + count_fly > CardInterface::Face::Two)
	{
		DLOG(INFO) << " firstFace = " << firstFace; 
		count_fly = 0;
		return false;
	}

    if(CardInterface::nAce == firstFace)
    {
        firstFace = CardInterface::Face::Two;
    }

	DLOG(INFO) << "TYPE_FLY = " << TYPE_FLY << "count_fly = " << count_fly << " laizi count = " << laizi_size 
		<< " LAIZI = " << get_laizi_card()->getName() << " need LAIZI = " << laizi_size << " firstFace = " << firstFace;
	return true;
}

bool Run4RefereeLaizi::IsBomb(const Cards& cards, int& count_laizi, CardInterface::Face& firstFace)
{
	if (cards.size() != 4)
	{
		return false;
	}
	count_laizi = 0;
	//int count = 1;
	Cards card_nolaizi;
	Cards card_laizi;
	auto handCardsInfo = getCardInfo(cards);

	if (IsLaizi(handCardsInfo, card_nolaizi, card_laizi))
	{
		if (card_nolaizi.size() + card_laizi.size() != cards.size())
		{
			DLOG(INFO) << "IsLaizi() error: card_nolaizi.size()= " << card_nolaizi.size() << "card_laizi.size()= " << card_laizi.size();
			return false;
		}
		switch (card_laizi.size())
		{
		case 1:
			if (card_nolaizi.at(0)->getFace() == card_nolaizi.at(1)->getFace() && card_nolaizi.at(1)->getFace() == card_nolaizi.at(2)->getFace())
			{
				count_laizi = 1;
				firstFace = card_nolaizi.front()->getFace();
				return true;
			}
			break;
		case 2:
			if (card_nolaizi.at(0)->getFace() == card_nolaizi.at(1)->getFace())
			{
				count_laizi = 2;
				firstFace = card_nolaizi.front()->getFace();
				return true;
			}
			break;
		case 3:
			count_laizi = 3;
			firstFace = card_nolaizi.front()->getFace();
			return true;
		case 4:
			count_laizi = 4;
			firstFace = handCardsInfo.front().face;
			return true;
		default:
			return false;
		}
	}

	if (1 == handCardsInfo.size() && handCardsInfo[0].suits.size() >= 4)
	{
		firstFace = handCardsInfo.front().face;
		return true;
	}
	for (auto iter : cards)
	{
		if (iter->getSuit() != CardInterface::NONESUIT)
		{
			return false;
		}
	}

	//firstFace = handCardsInfo.front().face;
	return true;
}

bool Run4RefereeLaizi::getLaizi(const Cards& cards, std::vector< CardInfo>& card_nolaizi, std::vector< CardInfo>& card_laizi)
{
	auto size = cards.size();
	auto handCardsInfo = getCardInfo(cards);

	for (auto iter : cards)
	{
		CardInfo card;
		if (iter->getFace() != laizi_)
		{
			card.face = iter->getFace();
			card.suits.push_back(iter->getSuit());
			card_nolaizi.push_back(card);
		}
		else
		{
			card.face = iter->getFace();
			card.suits.push_back(iter->getSuit());
			card_laizi.push_back(card);
		}
	}
	if (0 == card_laizi.size())
	{
		return false;
	}
	if ((card_laizi.size() + card_nolaizi.size()) != size)
	{
		return false;
	}
	return true;
}

bool Run4RefereeLaizi::IsLaizi(const Cards& cards)
{
	for (auto iter : cards)
	{		
		if (iter->getFace() == laizi_)
		{
			return true;
		}	
	}
	return false;
}

bool Run4RefereeLaizi::IsLaizi(const std::vector<CardInfo>& handCardsInfo, Cards& card_nolaizi, Cards& card_laizi)
{
	for (auto iter : handCardsInfo)
	{
		if (iter.face == laizi_)
		{
			for (auto suit_iter1 : iter.suits)
			{
				auto card = CardFactory::MakePokerCard(iter.face, suit_iter1);
				card_laizi.push_back(card);
			}
		}
		else
		{
			for (auto suit_iter2 : iter.suits)
			{
				auto card = CardFactory::MakePokerCard(iter.face, suit_iter2);
				card_nolaizi.push_back(card);
			}
		}
	}
	if (0 == card_laizi.size())
	{
		return false;
	}
	/*if ((card_laizi.size() + card_nolaizi.size()) != size)
	{
		return false;
	}*/
	return true;
}
std::vector<CardInfo> Run4RefereeLaizi::getCardInfo(const Cards& handCards)
{
	auto size = handCards.size();
	Cards allcards(handCards);
	std::sort(allcards.begin(), allcards.end(), ComparePoker);
	std::vector<CardInfo> handCardsInfo;
	for (size_t i = 0; i < size; i++)
	{
		auto iter = std::find_if(handCardsInfo.begin(), handCardsInfo.end(), [i, &allcards](const CardInfo& info) {
			return allcards[i]->getFace() == info.face;
		});
		if (iter == handCardsInfo.end())
		{
			CardInfo cardInfo;
			cardInfo.face = allcards[i]->getFace();
			cardInfo.suits.push_back(allcards[i]->getSuit());
			handCardsInfo.push_back(cardInfo);
		}
		else
		{
			iter->suits.push_back(allcards[i]->getSuit());
		}
	}

	return handCardsInfo;
}

Cards Run4RefereeLaizi::FindShunCard(const PlayedCard& left, const std::vector<CardInfo>& handCardsInfo, int count, size_t handcardscount)
{
	Cards cards;
	auto size = handCardsInfo.size();
	int cout_pairs = 1;
	int index = 0;
	for (size_t i = 0; i < size - 1; ++i)
	{
		if (handCardsInfo[i].face <= left.getFirstFace() ||
			handCardsInfo[i + 1].face == CardInterface::Two 
			)
		{
			index = 0;
			cout_pairs = 1;
			continue;
		}
		//         if (handCardsInfo[i].suits.size() == 4 ||
		//             handCardsInfo[i + 1].suits.size() == 4)
		//         {
		//             index = 0;
		//             cout_pairs = 1;
		//             continue;
		//         }
		if (handCardsInfo[i].suits.size() < static_cast<std::uint32_t>(count) ||
			handCardsInfo[i + 1].suits.size() < static_cast<std::uint32_t>(count))
		{
			index = 0;
			cout_pairs = 1;
			continue;
		}
		if ((handCardsInfo[i].face + 1) == handCardsInfo[i + 1].face)
		{
			index = i + 1;
			cout_pairs += 1;
		}
		else
		{
			index = 0;
			cout_pairs = 1;
		}
		if (left.getCount() == cout_pairs)
		{
			break;
		}
	}

	if (cout_pairs < left.getCount())
	{
		Cards card_nolaizi;
		Cards card_laizi;	
		if (IsLaizi(handCardsInfo, card_nolaizi, card_laizi))
		{
			DLOG(INFO) << "card_nolaizi: " << card_nolaizi;
			DLOG(INFO) << "card_laizi: " << card_laizi;
		
			auto hand_nolaizi = getCardInfo(card_nolaizi);
			
			if (Find_Laizi_ShunCard(left, hand_nolaizi, card_laizi, count, handcardscount, cards))
			{
				return cards;
			}
		}
		return cards;
	}

	for (int i = index - cout_pairs + 1; i <= index; ++i)
	{
		for (int s = 0; s < count; ++s)
		{
			auto card = CardFactory::MakePokerCard(handCardsInfo[i].face, handCardsInfo[i].suits[s]);
			cards.push_back(card);
		}
	}

	//飞机加上翅膀
	if (count == 3)
	{
		auto res = SelectWing(index - cout_pairs + 1, left.getCount(), handCardsInfo, cards);
		if (!res && cards.size() != handcardscount)
		{
			cards.clear();
		}
	}

	return cards;
}

bool Run4RefereeLaizi::Find_Laizi_ShunCard(const PlayedCard& left, const std::vector<CardInfo>& hand_nolaizi,
	const Cards& card_laizi, int count, size_t handcardscount, Cards& cards)
{
	auto handCardsInfo = hand_nolaizi;
	auto hand_laizi = getCardInfo(card_laizi);
	auto size = handCardsInfo.size();
	auto laizi_size = card_laizi.size();
	int index = 0;

	if (laizi_size <= 0 || left.getCount() == 0)
	{
		return false;
	}
	std::sort(handCardsInfo.begin(), handCardsInfo.end(), [](const CardInfo& left, const CardInfo& right) {
		return left.face < right.face;
	});

	std::vector<int> need_laizi(16); //匹配牌型需要的癞子张数
	std::map<CardInterface::Face, size_t> card_slot;
	need_laizi.resize(16);

	for (int i = 1; i <= 12; i++)
	{
		card_slot.insert(std::pair<CardInterface::Face, int>(static_cast<CardInterface::Face>(i), 0));
	}
	for (size_t t = 0; t < size; ++t)
	{
		card_slot[handCardsInfo[t].face] = handCardsInfo[t].suits.size();
	}

	for (int i = static_cast<int>(left.getFirstFace() + 1); i <= static_cast<int>(CardInterface::Two - left.getCount()); i++)
	{
		//取left.getCount()张牌分析需要多少张癞子 
		for (int k = i; k < (i + left.getCount()); k++)
		{
			if (card_slot[(CardInterface::Face)k] < static_cast<std::size_t>(count))
			{
				//记录每次需要的癞子张数
				need_laizi[i] += count - card_slot[(CardInterface::Face)k];
			}
		}
		DLOG(INFO) << "need_laizi[" << i << "] = " << need_laizi[i] << " laizi_size = " << laizi_size;
	}
	
	std::vector<int> tmp;
	for (std::size_t i = 1; i < need_laizi.size(); i++)
	{
		if (need_laizi[i] != 0)
		{
			tmp.push_back(need_laizi[i]);
		}
	}
	if (tmp.size() <= 0u)
	{
		return false;
	}

	//min最少需要的癞子张数
	int min = *std::min_element(tmp.begin(), tmp.end());
	if (laizi_size < static_cast<std::size_t>(min))
	{
		DLOG(INFO) << "min = " << min << " laizi_size = " << laizi_size;
		cards.clear();
		return false;
	}

	for (std::size_t i = 0; i < need_laizi.size(); i++)
	{
		if (min == need_laizi[i])
		{
			index = i;
			//break;
		}
	}	
		
	if (index > CardInterface::Two - left.getCount())
	{
		DLOG(INFO) << "index = " << index << " left.getCount() = " << left.getCount();
		cards.clear();
		return false;
	}

	/*for (size_t i = 0; i < size; i++)
	{
		if (static_cast<int32_t>(handCardsInfo[i].face) == index)
		{
			handcards_index = i;
			for (size_t j = i; j < (left.getCount() + i); j++)
			{
				auto suit_size = (int)handCardsInfo[j].suits.size();
				if (suit_size > count)
				{
					suit_size = count;
				}
				for (int k = 0; k < suit_size; ++k)
				{
					DLOG(ERROR) << "K = " << k << "J = " << j;
					auto card = CardFactory::MakePokerCard(handCardsInfo[j].face, handCardsInfo[j].suits[k]);
					cards.push_back(card);
				}
			}			
			break;
		}
	}	
	if (static_cast<int>(cards.size() + min) > left.getCount() * count)
	{
		for (size_t i = 0; i < (cards.size() + min) - left.getCount() * count; i++)
		{
			cards.pop_back();
		}
	}*/
	
	//取出要得起的牌
	for (int j = index; j < left.getCount() + index; ++j)
	{
		for (int i = 1; i <= count; i++)
		{
			cards.push_back(CardFactory::MakePokerCard(static_cast<CardInterface::Face>(j), static_cast<CardInterface::Suit>(i)));
		}
	}

	std::sort(cards.begin(), cards.end());
	DLOG(INFO) << "cards = " << cards;

	//如果是飞机加上翅膀
	if (count == 3)
	{		
		Cards clone_laizi(card_laizi);
		for (int s = 0; s < min; s++)
		{
			clone_laizi.pop_back();
		}
		auto res = Select_Laiiz_Wing(index, left.getCount(), handCardsInfo, clone_laizi, cards);
		if (!res && cards.size() != handcardscount)
		{
			cards.clear();
			return false;
		}
	}

	return true;
}

bool Run4RefereeLaizi::SelectWing(int index, int size, const std::vector<CardInfo>& handCardsInfo, Cards& cards)
{
	int wingSize = 0;
	for (size_t i = 0; i < handCardsInfo.size(); ++i)
	{
		if (handCardsInfo[index].face == handCardsInfo[i].face)
		{
			i += size - 1;
			continue;
		}

		for (auto suit_iter : handCardsInfo[i].suits)
		{
			wingSize += 1;
			auto card = CardFactory::MakePokerCard(handCardsInfo[i].face, suit_iter);
			cards.push_back(card);
			if (wingSize == size * 2)
			{
				return true;
			}
		}
	}

	return false;
}

bool Run4RefereeLaizi::Select_Laiiz_Wing(int index, int left_count, const std::vector<CardInfo>& handCardsInfo, Cards& card_laizi, Cards& cards)
{
	int wingSize = 0;
	for (auto iter : handCardsInfo)
	{
		if (index <= iter.face && iter.face < index + left_count)
		{
			continue;
		}
		std::sort(iter.suits.begin(), iter.suits.end());
		for (auto suit_iter : iter.suits)
		{
			wingSize += 1;
			auto card = CardFactory::MakePokerCard(iter.face, suit_iter);
			cards.push_back(card);
			if (wingSize == 2 * left_count)
			{
				return true;
			}
		}
	}

	for (auto iter_laizi : card_laizi)
	{
		cards.push_back(iter_laizi);
		wingSize += 1;
		if (wingSize == 2 * left_count)
		{
			return true;
		}
	}

	return false;
}

Cards Run4RefereeLaizi::FindThreeZone(const PlayedCard& left, const std::vector<CardInfo>& handCardsInfo, size_t handcardscount)
{
	Cards cards;
	CardInterface::Face face;

	for (auto iter : handCardsInfo)
	{
		if (iter.suits.size() == 3 &&
			iter.face > left.getFirstFace())
		{
			face = iter.face;
			std::sort(iter.suits.begin(), iter.suits.end());
			for (int s = 0; s < 3; ++s)
			{
				auto card = CardFactory::MakePokerCard(iter.face, iter.suits[s]);
				cards.push_back(card);
			}
			break;
		}
	}
	if (cards.size() < 3)
	{			
		if (Find_Laizi_ThreeZone(left, handCardsInfo, handcardscount, cards))
		{
			return cards;
		}

		return cards;
	}
	int wingSize = 0;
	for (auto iter : handCardsInfo)
	{
		if (iter.face == face)
		{
			continue;
		}
		std::sort(iter.suits.begin(), iter.suits.end());
		for (auto suit_iter : iter.suits)
		{
			wingSize += 1;
			auto card = CardFactory::MakePokerCard(iter.face, suit_iter);
			cards.push_back(card);
			if (wingSize == 2)
			{
				return cards;
			}
		}
	}

	if (cards.size() != 5 && cards.size() != handcardscount)
	{
		cards.clear();
	}

	return cards;
}

bool Run4RefereeLaizi::Find_Laizi_ThreeZone(const PlayedCard& left, const std::vector<CardInfo>& handCardsInfo, size_t handcardscount, Cards& cards)
{
	CardInfo cardinfo;
	Cards card_nolaizi;
	Cards card_laizi;
	if (IsLaizi(handCardsInfo, card_nolaizi, card_laizi) == false)
	{
		return false;
	}
	DLOG(INFO) << "card_nolaizi = " << card_nolaizi << " card_laizi = " << card_laizi;

	/*std::sort(handCardsInfo.begin(), handCardsInfo.end(), [](const CardInfo& left, const CardInfo& right) {
		return left.face < right.face;
	});*/
	switch (card_laizi.size())
	{
	case 1:
		for (auto iter : handCardsInfo)
		{
			if (iter.suits.size() >= 2 && iter.face > left.getFirstFace() && iter.face != laizi_)
			{
				for (auto suit_iter : iter.suits)
				{
					auto card = CardFactory::MakePokerCard(iter.face, suit_iter);
					cards.push_back(card);
				}
				cards.push_back(card_laizi.back());
				card_laizi.pop_back();
				cardinfo = iter;
				break;
			}
		}
		break;
	case 2:
		for (auto iter : handCardsInfo)
		{
			if (iter.face > left.getFirstFace() && iter.face != laizi_)
			{
				for (auto suit_iter : iter.suits)
				{
					auto card = CardFactory::MakePokerCard(iter.face, suit_iter);
					cards.push_back(card);
				}
				for (int i = 0; i < static_cast<int>(3 - iter.suits.size()); i++)
				{
					if (card_laizi.size() > 0)
					{
						cards.push_back(card_laizi.back());
						card_laizi.pop_back();
					}
				}
				cardinfo = iter;
				break;
			}
		}
		break;
	default:
		for (auto iter : handCardsInfo)
		{
			if (iter.face > left.getFirstFace() && iter.face != laizi_)
			{
				for (auto suit_iter : iter.suits)
				{
					auto card = CardFactory::MakePokerCard(iter.face, suit_iter);
					cards.push_back(card);
				}
				for (int i = 0; i < static_cast<int>(3 - iter.suits.size()); i++)
				{
					if (card_laizi.size() > 0)
					{
						cards.push_back(card_laizi.back());
						card_laizi.pop_back();
					}
				}
				cardinfo = iter;
				break;
			}
		}
		break;
	}

	if (cards.size() != 3)
	{
		cards.clear();
		return false;
	}
	DLOG(INFO) << "cards = " << cards;
	//if (handcardscount < 5 && handcardscount > 3)
	//{
	//	cards.clear();
	//	for (auto iter : handCardsInfo)
	//	{
	//		for (auto iter_suits : iter.suits)
	//		{
	//			cards.push_back(CardFactory::MakePokerCard(iter.face, iter_suits));
	//		}
	//	}
	//	return true;
	//}

	int wingSize = 0;
	for (auto iter : handCardsInfo)
	{
		if (iter.face == cardinfo.face || iter.face == laizi_)
		{		
			continue;
		}
		std::sort(iter.suits.begin(), iter.suits.end());
		for (auto suit_iter : iter.suits)
		{
			wingSize += 1;
			auto card = CardFactory::MakePokerCard(iter.face, suit_iter);
			cards.push_back(card);
			if (wingSize == 2)
			{
				return true;
			}
		}
	}
	for (auto iter_laizi : card_laizi)
	{
		cards.push_back(iter_laizi);
		wingSize += 1;
		if (wingSize == 2)
		{
			return true;
		}
	}
	
	DLOG(INFO) << "cards = " << cards;
	if (cards.size() != 5 && cards.size() != handcardscount)
	{
		cards.clear();
		return false;
	}
	return true;
}

Cards Run4RefereeLaizi::FindBomb(const PlayedCard& left, const std::vector<CardInfo>& handCardsInfo)
{
	Cards cards;
	/*if (left.getCards()[0]->getSuit() == CardInterface::NONESUIT)
	{
		return cards;
	}*/
		
	for (auto iter : handCardsInfo)
	{
		if (iter.suits.size() < 4)
		{
			continue;
		}

		if (iter.suits.size() == 4 &&
			iter.face > left.getFirstFace())
		{
			for (auto suit_iter : iter.suits)
			{
				auto card = CardFactory::MakePokerCard(iter.face, suit_iter);
				cards.push_back(card);
			}
			return cards;
		}
		if (iter.suits.size() > left.getCards().size())
		{
			for (auto suit_iter : iter.suits)
			{
				auto card = CardFactory::MakePokerCard(iter.face, suit_iter);
				cards.push_back(card);
			}
			return cards;
		}
	}
	Cards card_nolaizi;
	Cards card_laizi;
	if (IsLaizi(handCardsInfo, card_nolaizi, card_laizi))
	{
		if (Find_Laizi_Bomb(left, handCardsInfo, cards, card_laizi)) //找癞子炸弹
		{
			return cards;
		}
	}
	return GetWangCards(handCardsInfo);
}

Cards Run4RefereeLaizi:: FindBomb(const std::vector<CardInfo>& handCardsInfo)
{
	Cards cards;
	for (auto iter : handCardsInfo)
	{
		if (iter.suits.size() == 4)
		{
			for (auto suit_iter : iter.suits)
			{
				auto card = CardFactory::MakePokerCard(iter.face, suit_iter);
				cards.push_back(card);
			}
			return cards;
		}
	}
	Cards card_laizi;
	Cards card_nolaizi;
	if (IsLaizi(handCardsInfo, card_nolaizi, card_laizi))
	{
		if (Find_Laizi_Bomb(handCardsInfo, cards, card_laizi))
		{
			return cards;
		}
	}
	return Cards();
	//return GetWangCards(handCardsInfo);
}

bool Run4RefereeLaizi::Find_Laizi_Bomb(const PlayedCard& left, const std::vector<CardInfo>& handCardsInfo, Cards& cards, const Cards& card_laizi)
{
	cards.clear();
	DLOG(INFO) << "Find laizi bomb cards = " << card_laizi << " prive cards = " << left.getCards();
	auto laizi_size = card_laizi.size();

	if (laizi_size <= 0)
	{
		return false;
	}
	if (laizi_size >= 4)
	{
		for (auto laizi_iter : card_laizi)
		{
			cards.push_back(laizi_iter);
		}
		return true;
	}

	switch (card_laizi.size())
	{
	case 1:
		for (auto iter : handCardsInfo)
		{
			if (iter.suits.size() == 3 && iter.face > left.getFirstFace() && iter.face != laizi_)
			{
				for (auto suit_iter : iter.suits)
				{
					auto card = CardFactory::MakePokerCard(iter.face, suit_iter);
					cards.push_back(card);
				}
				cards.push_back(card_laizi.back());
				return true;
			}
		}
		break;
	case 2:
		for (auto iter : handCardsInfo)
		{
			if (iter.suits.size() >= 2 && iter.face > left.getFirstFace() && iter.face != laizi_)
			{
				for (auto suit_iter : iter.suits)
				{
					auto card = CardFactory::MakePokerCard(iter.face, suit_iter);
					cards.push_back(card);
				}
				for (int i = 0; i < static_cast<int>(4 - iter.suits.size()); i++)
				{
					cards.push_back(card_laizi.back());
				}			
				return true;
			}
		}
		break;
	case 3:
		for (auto iter : handCardsInfo)
		{
			if (iter.suits.size() >= 1 && iter.face > left.getFirstFace() && iter.face != laizi_)
			{
				for (auto suit_iter : iter.suits)
				{
					auto card = CardFactory::MakePokerCard(iter.face, suit_iter);
					cards.push_back(card);
				}
				for (int i = 0; i < static_cast<int>(4 - iter.suits.size()); i++)
				{
					cards.push_back(card_laizi.back());
				}
				return true;
			}
		}
		break;
	}

	if (cards.size() == 4)
	{
		DLOG(INFO) << "return cards = " << cards << "Number of cards: " << cards.size();
		return true;
	}

	DLOG(INFO) << "return cards = " << cards << "Number of cards: " << cards.size();
	cards.clear();
	return false;
}

bool Run4RefereeLaizi::Find_Laizi_Bomb(const std::vector<CardInfo>& handCardsInfo, Cards& cards, const Cards& card_laizi)
{
	DLOG(INFO) << "Find_Laizi_Bomb(): LZIAI cards = " << card_laizi << "laizi_size = " << card_laizi.size();
	
	cards.clear();
	auto laizi_size = card_laizi.size();
	if (laizi_size <= 0)
	{
		return false;
	}

	if (laizi_size >= 4)
	{
		for (auto laizi_iter : card_laizi)
		{
			cards.push_back(laizi_iter);
		}
		return true;
	}

	switch (laizi_size)
	{
	case 1:
		for (auto iter : handCardsInfo)
		{
			if (iter.suits.size() == 3 && iter.face != laizi_)
			{
				for (auto suit_iter : iter.suits)
				{
					auto card = CardFactory::MakePokerCard(iter.face, suit_iter);
					cards.push_back(card);
				}
				cards.push_back(card_laizi.back());
				break;
			}
		}
		break;
	case 2:
		for (auto iter : handCardsInfo)
		{
			if (iter.suits.size() >= 2 && iter.face != laizi_)
			{
				for (auto suit_iter : iter.suits)
				{
					auto card = CardFactory::MakePokerCard(iter.face, suit_iter);
					cards.push_back(card);
				}
				for (int i = 0; i < static_cast<int>(4 - iter.suits.size()); i++)
				{
					cards.push_back(card_laizi.back());					
				}
				break;
			}
		}
		break;
	case 3:
		for (auto iter : handCardsInfo)
		{
			if (iter.face != laizi_)
			{
				for (auto suit_iter : iter.suits)
				{
					auto card = CardFactory::MakePokerCard(iter.face, suit_iter);
					cards.push_back(card);
				}
				for (int i = 0; i < static_cast<int>(4 - iter.suits.size()); i++)
				{
					cards.push_back(card_laizi.back());
				}
				break;
			}
		}
		break;
	default:
		break;
	}
	/*for (auto iter : handCardsInfo)
	{
		if (iter.suits.size() >= (4 - static_cast<int>(laizi_size)) && iter.face != laizi_)
		{
			for (auto suit_iter : iter.suits)
			{
				auto card = CardFactory::MakePokerCard(iter.face, suit_iter);
				cards.push_back(card);
			}
			for (auto laizi_iter : card_laizi)
			{
				cards.push_back(laizi_iter);
			}
			return true;
		}
	}*/
	if (cards.size() == 4)
	{
		DLOG(INFO) << "return cards = " << cards  << " Number of cards: " << cards.size();
		return true;
	}

	DLOG(INFO) << "return cards = " << cards << " Number of cards: " << cards.size();
	cards.clear();
	return false;
}
Cards Run4RefereeLaizi::GetWangCards(const std::vector<CardInfo>& handCardsInfo)
{
	Cards cards;
	Cards card_nolaizi;
	Cards card_laizi;
	if (IsLaizi(handCardsInfo, card_nolaizi, card_laizi))
	{
		if (card_laizi.size() >= 4)
		{
			return card_laizi;
		}
	}
	for (auto iter : handCardsInfo)
	{
		if (iter.face == CardInterface::LittleJoker ||
			iter.face == CardInterface::BigJoker)
		{
			for (auto suit_iter : iter.suits)
			{
				auto card = CardFactory::MakePokerCard(iter.face, suit_iter);
				cards.push_back(card);
			}
		}
	}
	if (cards.size() != 4u)
	{
		cards.clear();
	}

	return cards;
}

bool Run4RefereeLaizi::IsInHandCard(const Cards& playedCard, const Cards& handCards)
{
	Cards clone(handCards);
	for (auto iter : playedCard)
	{
		auto it = std::find_if(clone.begin(), clone.end(), [iter](std::shared_ptr<CardInterface> card) {
			return iter->getName() == card->getName();
		});
		if (it == clone.end())
		{
			return false;
		}
		else
		{
			clone.erase(it);
		}
	}

	return true;
}

Cards Run4RefereeLaizi::SortCard(const Cards& playedCard, CardType type, int count, CardInterface::Face firstFace)
{
	Cards cards(playedCard);
	if (0 == cards.size())
	{
		return cards;
	}		
	std::sort(cards.begin(), cards.end(), ComparePoker);

	Cards newCards;

	if (cards.size() == 4 &&
		type != TYPE_LAIZI_BOMB &&
		type != TYPE_BOMB &&
		type != TYPE_ALL_LAIZI_BOMB
		)
	{
		auto card_groups = getCardInfo(cards);
		for (auto iter : card_groups)
		{
			if (iter.suits.size() == 3u)
			{
				type = TYPE_THREE_ZONE;
				count = 1;
				firstFace = iter.face;
				break;
			}
		}
	}

	if (type == TYPE_FLY || type == TYPE_THREE_ZONE)
	{
		for (int i = 0; i < count; i++)
		{
			for (auto iter : playedCard)
			{
				if (iter->getFace() == firstFace + i)
				{
					newCards.push_back(iter);
					auto it = std::find(cards.begin(), cards.end(), iter);
					if (it != cards.end())
					{
						cards.erase(it);
					}
				}
			}
		}
		for (auto iter : cards)
		{
			newCards.push_back(iter);
		}
	}
	else
	{
		newCards = cards;
	}
	return newCards;
}

bool Run4RefereeLaizi::IsEndFly(const Cards& cards)
{
	Cards res_cards;
	auto size = cards.size();

	auto handCardsInfo = getCardInfo(cards);
	std::sort(handCardsInfo.begin(), handCardsInfo.end(), [](const CardInfo& left, const CardInfo& right) {
		return left.face < right.face;
	});

	if (size < 6)
	{
		return false;
	}

	std::vector< CardInterface::Face > threeFaces;
	for (auto iter : handCardsInfo)
	{
		if (iter.suits.size() == 3u)
		{
			threeFaces.push_back(iter.face);
		}
	}

	if (threeFaces.size() < 2)
	{
		return false;
	}

	std::sort(threeFaces.begin(), threeFaces.end());
	auto count = 1;
	auto temp = 1;
	for (size_t f = 1; f < threeFaces.size(); ++f)
	{
		if ((threeFaces[f - 1] + 1) == threeFaces[f])
		{
			count += 1;
			continue;
		}
		else
		{
			if (temp < count)
			{
				temp = count;
			}
			count = 1;
		}
	}

	if (temp < count)
	{
		temp = count;
	}

	if (temp <= 1)
	{
		return false;
	}

	if (static_cast<std::int32_t>(size) > (temp * 5))
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool Run4RefereeLaizi::IsEndfly_EndThreeZone(const Cards& cards, CardInterface::Face& firstFace, CardType& type, int& count_fly)
{
	Cards res_cards;
	auto size = cards.size();

	auto handCardsInfo = getCardInfo(cards);
	std::sort(handCardsInfo.begin(), handCardsInfo.end(), [](const CardInfo& left, const CardInfo& right) {
		return left.face < right.face;
	});

	if (size < 3)
	{
		return false;
	}

	std::vector< CardInterface::Face > threeFaces;
	for (auto iter : handCardsInfo)
	{
		if (iter.suits.size() == 3u)
		{
			threeFaces.push_back(iter.face);
		}
	}

	if (threeFaces.size() < 1)
	{
		return false;
	}

	std::sort(threeFaces.begin(), threeFaces.end());
	auto count = 1;
	auto temp = 1;
	for (size_t f = 1; f < threeFaces.size(); ++f)
	{
		if ((threeFaces[f - 1] + 1) == threeFaces[f])
		{
			count += 1;
			continue;
		}
		else
		{
			if (temp < count)
			{
				temp = count;
			}
			count = 1;
		}
	}

	if (temp < count)
	{
		temp = count;
	}

	/*if (temp < 1)
	{
		return false;
	}*/

	if (static_cast<std::int32_t>(size) > (temp * 5))
	{
		return false;
	}
	else
	{
		firstFace = threeFaces.front();
		type = threeFaces.size() < 2u ? TYPE_THREE_ZONE : TYPE_FLY;
		count_fly = temp;
		return true;
	}
}

bool Run4RefereeLaizi::Is_Laizi_EndFly(const Cards& cards, CardInterface::Face& firstFace, CardType& type, int& count_fly)
{
	auto handCardsInfo = getCardInfo(cards);
	Cards card_nolaizi;
	Cards card_laizi;
	if (IsLaizi(handCardsInfo, card_nolaizi, card_laizi) == false)
	{
		DLOG(INFO) << "IsLaizi() error: cards_laizi.size() = " << card_laizi.size();
		return false;
	}
	DLOG(INFO) << "card_nolaizi = " << card_nolaizi << "  card_laizi = " << card_laizi;

	std::vector< CardInterface::Face > threeFaces;
	std::map<CardInterface::Face, size_t> card_slot;
	auto laizi_size = card_laizi.size();	
	auto nolaizi_info = getCardInfo(card_nolaizi);
	auto laizi_info = getCardInfo(card_laizi);
	std::vector<int> need_laizi(16);

	std::sort(nolaizi_info.begin(), nolaizi_info.end(), [](const CardInfo& left, const CardInfo& right) {
		return left.face < right.face;
	});
	std::sort(handCardsInfo.begin(), handCardsInfo.end(), [](const CardInfo& left, const CardInfo& right) {
		return left.face < right.face;
	});

	for (int i = 1; i <= 12; i++)
	{
		card_slot.insert(std::pair<CardInterface::Face, size_t>(static_cast<CardInterface::Face>(i), 0));
	}

	for (size_t t = 0; t < nolaizi_info.size(); ++t)
	{
		card_slot[nolaizi_info[t].face] = nolaizi_info[t].suits.size();
	}

	if (cards.size() <= 5 && cards.size() >= 3)
	{
		switch (laizi_size)
		{
		case 1:
			for (auto it = nolaizi_info.rbegin(); it != nolaizi_info.rend(); ++it)
			{
				if (it->suits.size() >= 2u)
				{
					firstFace = it->face;
					type = TYPE_THREE_ZONE;
					count_fly = 1;
					return true;
				}
			}
			return false;
		case 2:
			firstFace = nolaizi_info.back().face;
			type = TYPE_THREE_ZONE;
			count_fly = 1;
			return true;
		case 3:
			firstFace = handCardsInfo.back().face;
			type = TYPE_THREE_ZONE;
			count_fly = 1;
			return true;
		}
	}
	else if (cards.size() <= 10 && cards.size() >= 6)
	{
		type = TYPE_FLY;
		count_fly = 2;
	}
	else if (9 <= cards.size() && cards.size() <= 15)
	{
		type = TYPE_FLY;
		count_fly = 3;
	}
	else
	{
		type = TYPE_INVALID;
		count_fly = 0;
		return false;
	}

	for (int i = static_cast<int>(nolaizi_info.front().face); i <= static_cast<int>(nolaizi_info.back().face) - count_fly + 1; i++)
	{
		for (int k = i; k < count_fly + i; k++)
		{
			if (card_slot[(CardInterface::Face)k] < 3)
			{
				need_laizi[i] += 3 - card_slot[(CardInterface::Face)k];
			}
		}
		DLOG(INFO) << "first = " << i << " need_laizi:= " << need_laizi[i] << " laizi size:= " << laizi_size;
	}

	std::vector<int> tmp;
	for (size_t i = 1; i < need_laizi.size(); i++)
	{
		if (need_laizi[i] != 0)
		{
			tmp.push_back(need_laizi[i]);
		}
	}
	if (tmp.size() == 0)
	{
		return false;
	}

	//min最少需要的癞子张数
	int min = *std::min_element(tmp.begin(), tmp.end());
	DLOG(INFO) << " min:= " << min << " laizi size:= " << laizi_size;
	if (laizi_size < static_cast<std::size_t>(min))
	{
		return false;
	}

	for (std::size_t i = 0; i < need_laizi.size(); i++)
	{
		if ((min == need_laizi[i] || need_laizi[i] == static_cast<int>(laizi_size)) &&
			(i + count_fly) <=  CardInterface::Face::Two)
		{
			firstFace = static_cast<CardInterface::Face>(i);			
			DLOG(INFO) << "firstFace = " << firstFace << " need laizi:= " << need_laizi[i] << "laizi size:= " << laizi_size;
		}
		DLOG(INFO) << "firstFace: = "<< i << " need laizi:= " << need_laizi[i] << " laizi size:= " << laizi_size << " min: = " << min;
	}

	if (firstFace + count_fly > CardInterface::Face::Two)
	{
		count_fly = 0;
		type = TYPE_INVALID;
		return false;
	}

	DLOG(INFO) << "END FLY: " << cards;
	return true;
}

bool Run4RefereeLaizi::IsHaveHei3(const Cards& handCards)
{
	for (auto card_iter : handCards)
	{
		if (card_iter->getFace() == CardInterface::Face::Three &&
			card_iter->getSuit() == CardInterface::Suit::Spades)
		{
			return  true;
		}
	}

	return false;
}

void Run4RefereeLaizi::generated_laizi()
{
#ifdef TEST_CARD
    int laizi_face;
    if(TEST_CARD)
        laizi_face = TEST_CARD;
    else
        laizi_face = rand() % 13 + 1;
#else
	auto laizi_face = rand() % 13 + 1;
#endif
    laizi_ = laizi_face;

	auto card = CardFactory::MakePokerCard(static_cast<CardInterface::Face>(laizi_face), static_cast<CardInterface::Suit>(1));

	if (laizi_face > 13)
	{
		//DLOG(ERROR) << "laizi: face = " << face << " suit = " << suit;
		throw "error";
	}
	set_laizi_card(card);
	DLOG(INFO) << "generated_laizi: " << card << "laizi_card_: " << laizi_card_;

	return;
}

int Run4RefereeLaizi::get_laizi() const { return laizi_; }

std::shared_ptr<CardInterface> Run4RefereeLaizi::get_laizi_card() const{ return laizi_card_; }
void Run4RefereeLaizi::set_laizi_card(const std::shared_ptr<CardInterface>& laizi) { laizi_card_ = laizi; }

Cards Run4RefereeLaizi::match_Laizi(const PlayedCard& playcard)
{
	if (0 == playcard.getCards().size())
	{
		return Cards();
	}
	Cards cards(playcard.getCards());
	Cards card_nolaizi;
	Cards card_laizi;
	auto cards_info = getCardInfo(cards);

	DLOG(INFO) << "Splicing card name :  cards = " << cards;

	if (!IsLaizi(cards_info, card_nolaizi, card_laizi))
	{
		DLOG(INFO) << "Play the cards without Laizi";
		return Cards();
	}
	if (card_nolaizi.size() == 0 || card_laizi.size() == 0)
	{
		return Cards();
	}

	Cards new_cards(cards);
	std::vector<std::string> card_name;
	switch (playcard.getType())
	{
	case TYPE_ONE:
		break;
	case TYPE_ONEPAIR:
		DLOG(INFO) << "Splicing card name : playcard type = " << playcard.getType() << "playcard cards = " << playcard.getCards();
		card_name.push_back(card_laizi.back()->getName());
		card_name.back().push_back(':');
		card_name.back() += card_nolaizi.back()->getName();
		for (auto iter : new_cards)
		{
			if (iter->getFace() == static_cast<CardInterface::Face>(laizi_))
			{
				DLOG(INFO) << "setChangeName: " << card_name.back();
				iter->setChangeName(card_name.back());
				card_name.pop_back();
			}			
		}		
		break;
	case TYPE_PAIRS:
		 return source_Purpose_Laizi(2, card_nolaizi, card_laizi, playcard);
		 break;
	case TYPE_THREE_ZONE:
		return source_Purpose_Laizi(3, card_nolaizi, card_laizi, playcard);
		break;
	case TYPE_SHUNZI:
		return source_Purpose_Laizi(1, card_nolaizi, card_laizi, playcard);
	case TYPE_FLY:		
		return source_Purpose_Laizi(3, card_nolaizi, card_laizi, playcard);
		break;
	case TYPE_LAIZI_BOMB:
		DLOG(INFO) << "Splicing card name : playcard type = " << playcard.getType() << "playcard cards = " << playcard.getCards();
		for (auto iter : new_cards)
		{
			if (iter->getFace() == static_cast<CardInterface::Face>(laizi_) && card_laizi.size() > 0)
			{
				card_name.push_back(card_laizi.back()->getName());
				card_name.back().push_back(':');
				card_name.back() += card_nolaizi.back()->getName();
				card_laizi.pop_back();

				DLOG(INFO) << "setChangeName: " << card_name.back();
				iter->setChangeName(card_name.back());
			}
		}
		break;
	default:
		break;
	}

	return new_cards;
}

Cards Run4RefereeLaizi::source_Purpose_Laizi(const int& count, const Cards& card_nolaizi, const Cards& card_laizi, const PlayedCard& playcard)
{
	if (playcard.getFirstFace() + playcard.getCount() > 13  && playcard.getCount() > 1)
	{
		return Cards();
	}
	if (playcard.getCards().size() != (card_laizi.size() + card_nolaizi.size()))
	{
		return Cards();
	}

	Cards src_cards(card_nolaizi);
	Cards dst_cards;
	Cards clone_laizi(card_laizi);
	Cards wing;
	std::map<CardInterface::Face, Cards> card_slot;

	for (int j = static_cast<int>(playcard.getFirstFace()); j < playcard.getFirstFace() + playcard.getCount(); ++j)
	{
		for (int i = 1; i <= count; i++)
		{
			dst_cards.push_back(CardFactory::MakePokerCard(static_cast<CardInterface::Face>(j),static_cast<CardInterface::Suit>(i) ));
		}
	}
	DLOG(INFO) << "dst_cards: " << dst_cards;

	//飞机去掉翅膀
	if (count == 3)
	{
		for (auto it = src_cards.begin(); it != src_cards.end(); )
		{
			auto find_it = std::find_if(dst_cards.begin(), dst_cards.end(), [it](const std::shared_ptr<CardInterface>& card) {
				return card->getFace() == (*it)->getFace();
			});
			if (find_it == dst_cards.end())
			{
				wing.push_back(*it);
				it = src_cards.erase(it);
			}
			else
			{
				++it;
			}
		}
	}
	DLOG(INFO) << "src_cards: " << src_cards;

	auto dst_cardInfo = getCardInfo(dst_cards);
	auto src_cardInfo = getCardInfo(src_cards);
	auto laizi_cardInfo = getCardInfo(card_laizi);

	for (auto iter : src_cardInfo)
	{
		if (iter.suits.size() == 4)
		{
			auto it = find_if(src_cards.begin(), src_cards.end(), [&](const std::shared_ptr<CardInterface>& card) {
				return card->getFace() == iter.face;
			});
			if (it != src_cards.end())
			{
				wing.push_back(*it);
				src_cards.erase(it);
			}
			DLOG(INFO) << "src_cards: " << src_cards;
			src_cardInfo = getCardInfo(src_cards);
		}
	}
	

	if (wing.size() + dst_cards.size() != playcard.getCards().size() && 
		(clone_laizi.size() + src_cards.size()) % 3 == 0
		)
	{
		return Cards();
	}

	/*std::sort(dst_cardInfo.begin(), dst_cardInfo.end(), [](const CardInfo& left, const CardInfo& right){
		return left.face < right.face;
	});*/
	std::sort(src_cardInfo.begin(), src_cardInfo.end(), [](const CardInfo& left, const CardInfo& right) {
		return left.face < right.face;
	});

	for (auto iter : dst_cardInfo)
	{		
		card_slot.insert(std::pair<CardInterface::Face, Cards>(iter.face, Cards() ));
	}
	
	for (auto iter : src_cardInfo)
	{
		for (auto it_suit : iter.suits)
		{
			card_slot[iter.face].push_back(CardFactory::MakePokerCard(iter.face, it_suit));
		}
	}

	for (auto iter : card_slot)
	{
		if (iter.second.size() < static_cast<size_t>(count))
		{
			for (int m = 0; m < static_cast<int>(count - iter.second.size()); ++m)
			{
				if (clone_laizi.size() <= 0)
				{
					DLOG(ERROR) << "card_laizi.size() < count - iter.second.size(), card_laizi.size() = " << card_laizi.size();
					return Cards();
				}
				if (card_laizi.size() < count - iter.second.size())
				{
					DLOG(ERROR) << "card_laizi.size() < count - iter.second.size(), card_laizi.size() = " << card_laizi.size();
					return Cards();
				}
				auto card = CardFactory::MakePokerCard(iter.first, static_cast<CardInterface::Suit>(1));
				auto name = clone_laizi.back()->getName();
				name += ":";
				name += card->getName();
				card->setChangeName(name);
				//src_cards.push_back(card);
				DLOG(INFO) << "card->getChangeName() = " << card->getChangeName();
				card_slot[iter.first].push_back(card);
				clone_laizi.pop_back();
			}
		}
	}

	Cards new_cards;
	for (auto mapit : card_slot)
	{
		for (auto vecit : mapit.second)
		{
			DLOG(INFO) << "card->getName() = " << vecit->getName();
			DLOG(INFO) << "card->getChangeName() = " << vecit->getChangeName();			
			new_cards.push_back(vecit);
		}	
	}

	//飞机还原翅膀
	if (3 == count)
	{
		for (auto iter : wing)
		{
			new_cards.push_back(iter);
		}
		if (clone_laizi.size() > 0)
		{
			for (auto iter : clone_laizi)
			{
				new_cards.push_back(iter);
			}
		}
	}
	//DCHECK_EQ(src_cards, dst_cards);

	return 	new_cards;
}
