﻿#include "run4_referee.h"
#include <algorithm>
#include <set>
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

#define _RUNFAST_DEBUG_

int Run4Referee::Compare(const PlayedCard& left, const PlayedCard& right)
{
    auto leftType = left.getType();
    auto rightType = right.getType();

    if (leftType == TYPE_BOMB &&
        rightType == TYPE_BOMB)
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
    else if (leftType == TYPE_BOMB &&
        rightType != TYPE_BOMB
        )
    {
        return 1;
    }
    else if (leftType != TYPE_BOMB &&
        rightType == TYPE_BOMB
        )
    {
        return -1;
    }
    else
    {
        if (leftType != rightType)
        {
            return -1;
        }
        switch (leftType)
        {
        case TYPE_ONE:
        case TYPE_ONEPAIR:
        case TYPE_THREE:
            return left.getCards()[0]->getFace() > right.getCards()[0]->getFace() ? 1 : -1;
        case TYPE_PAIRS:
        case TYPE_THREE_ZONE:
        case TYPE_FLY:
            return left.getFirstFace() > right.getFirstFace() ? 1 : -1;
        case TYPE_SHUNZI:
        {
            if (IsSameType(left.getCards()) == true &&
                IsSameType(right.getCards()) == true &&
                type_ == PLAY_TYPE::TYPE_TWO)
            {
                if (left.getFirstFace() == right.getFirstFace())
                {
                    return left.getCards()[0]->getSuit() > right.getCards()[0]->getSuit() ? 1 : -1;
                }
                else
                {
                    return left.getFirstFace() > right.getFirstFace() ? 1 : -1;
                }
            }
            else if (IsSameType(left.getCards()) == true &&
                IsSameType(right.getCards()) == false &&
                type_ == PLAY_TYPE::TYPE_TWO)
            {
                return 1;
            }
            else if (IsSameType(right.getCards()) == true &&
                IsSameType(left.getCards()) == false &&
                type_ == PLAY_TYPE::TYPE_TWO)
            {
                return -1;
            }
            else
            {
//                 Cards lclone(left.getCards());
//                 Cards rclone(right.getCards());
//                 std::sort(lclone.begin(), lclone.end(), ComparePoker);
//                 std::sort(rclone.begin(), rclone.end(), ComparePoker);
// 
//                 if (left.getFirstFace() == right.getFirstFace())
//                 {
//                     return lclone[lclone.size() - 1]->getSuit() > rclone[rclone.size() - 1]->getSuit() ? 1 : -1;
//                 }
//                 else
//                 {
                    return left.getFirstFace() > right.getFirstFace() ? 1 : -1;
                //}
            }
        }
        default:
            return 0;
        }
    }

    return 0;
}

CardType Run4Referee::getCardType(const Cards& cards, int& count, CardInterface::Face& firstFace)
{
    Cards clone(cards);
    std::sort(clone.begin(), clone.end());

    DLOG(INFO) << "Run4Referee::getCardType() cards:=" << clone;

    count = 0;
    auto size = clone.size();
    if (size == 1)
    {
        return TYPE_ONE;
    }
    else if (size == 2 &&
        clone[0]->getFace() == clone[1]->getFace()
        )
    {
        return TYPE_ONEPAIR;
    }
    else if(size == 3 && 
        type_ == PLAY_TYPE::TYPE_TWO &&
        clone[0]->getFace() == clone[2]->getFace())
    {
        count = 1;
        firstFace = clone[0]->getFace();
        return TYPE_THREE;
    }
    else if (IsBomb(clone))
    {
        return TYPE_BOMB;
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

bool Run4Referee::isMaxCard(const Cards& cards, const Cards& rightHandCards)
{
    Cards clone(rightHandCards);
    std::sort(clone.begin(), clone.end(), ComparePoker);
    if (cards[0]->getFace() != clone[clone.size() - 1]->getFace())
    {
        return false;
    }

    return true;
}

Cards Run4Referee::GetMaxCard(const Cards& HandCards)
{
    Cards clone(HandCards);
    std::sort(clone.begin(), clone.end(), ComparePoker);

    Cards cards;
    auto card = CardFactory::MakePokerCard(clone[clone.size() - 1]->getFace(), clone[clone.size() - 1]->getSuit());
    cards.push_back(card);

    return cards;
}

Cards  Run4Referee::autoPlay(const PlayedCard& left, const Cards& rightHandCards, std::int32_t nseat_size)
{
    Cards clone(rightHandCards);
    std::sort(clone.begin(), clone.end(), ComparePoker);

    DLOG(INFO) << "Run4Referee::autoPlay() PlayedCards:=" << left.getCards()
        << " type:=" << left.getType() << " count:= " << left.getCount() << " fristface:= " << left.getFirstFace();
    DLOG(INFO) << "Run4Referee::autoPlay()   HandCards:=" << clone;

    Cards cards;
    auto handCardsInfo = getCardInfo(rightHandCards);
    std::sort(handCardsInfo.begin(), handCardsInfo.end(), [](const CardInfo& left, const CardInfo& right) {
        return left.face < right.face;
    });

    Cards bombCards = FindBomb(handCardsInfo);
    if (bombCards.size() == 0 &&
        left.getCards().size() > rightHandCards.size() &&
        left.getType() != TYPE_THREE_ZONE &&
        left.getType() != TYPE_FLY
        )
    {
        return cards;
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
        if (type_ == PLAY_TYPE::TYPE_TWO && IsSameType(left.getCards()) == true )
        {
            cards = FindTHShunCard(left, rightHandCards, 1, rightHandCards.size());
        }
        else
        {
            cards = FindShunCard(left, handCardsInfo, 1, rightHandCards.size());
            if (type_ == PLAY_TYPE::TYPE_TWO && cards.size() == 0)
            {
                cards = FindTHShunCard(left, rightHandCards, 1, rightHandCards.size());
            }
        }
        break;
    case TYPE_FLY:
        cards = FindShunCard(left, handCardsInfo, 3, rightHandCards.size());
        break;
    case TYPE_BOMB:
        return FindBomb(left, handCardsInfo);
    case TYPE_THREE:
        cards = FindThree(left, handCardsInfo, rightHandCards.size());
        break;
    default:
        break;
    }

    if (cards.size() == 0 &&
        bombCards.size() >= 4)
    {
        return bombCards;
    }

    DLOG(INFO) << "Run4Referee::autoPlay()   ReturnCards:=" << cards.size();
    return cards;
}

Cards Run4Referee::autoPlay(const Cards& handCards)
{
    int count = 0;
    CardInterface::Face firstface;
    Cards clone(handCards);
    auto handCardsInfo = getCardInfo(clone);
    if (handCards.size() == 0)
    {
        return Cards();
    }

    auto cards_type = getCardType(clone, count, firstface);
    if (cards_type != TYPE_INVALID)
    {
        if (FindBomb(handCardsInfo).size() == 0 ||
            FindBomb(handCardsInfo).size() == handCards.size())
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
        else
        {
            Cards cards;
            cards.push_back(clone[0]);
            return cards;
        }
    }
}

Cards Run4Referee::autoPlay_AI(const Cards& rightHandCards, std::int32_t nseat_size)
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

bool Run4Referee::IsShunZi(const Cards& cards, int& count_shun, CardInterface::Face& firstFace)
{
    if (type_ == PLAY_TYPE::TYPE_TWO) return IsShunZi_Ex(cards, count_shun, firstFace);

    count_shun = 1;
    auto size = cards.size();
    Cards allcards(cards);
    std::sort(allcards.begin(), allcards.end(), ComparePoker);

    if (size < 5)
    {
        return false;
    }

    if (allcards[size - 1]->getFace() == CardInterface::Two)
    {
        return false;
    }

    for (size_t i = 1; i < size; ++i)
    {
        if ((allcards[i - 1]->getFace() + 1) == allcards[i]->getFace())
        {
            count_shun += 1;
        }
        else
        {
            return false;
        }
    }

    if (count_shun != static_cast<std::int32_t>(size))
    {
        return false;
    }

    firstFace = allcards[0]->getFace();
    return true;
}

bool Run4Referee::IsShunZi_Ex(const Cards& cards, int& count_shun, CardInterface::Face& firstFace)
{
    count_shun = 1;
    auto size = cards.size();
    Cards allcards(cards);
    std::sort(allcards.begin(), allcards.end(), ComparePoker);

    if (size < 5)
    {
        return false;
    }

    firstFace = allcards[0]->getFace();
    if (allcards[size - 1]->getFace() == CardInterface::Two &&
        allcards[size - 2]->getFace() == CardInterface::Ace &&
        allcards[0]->getFace() == CardInterface::Three)
    {
        count_shun += 2;
        size -= 2;
        firstFace = CardInterface::nAce;
    }
    else if (allcards[0]->getFace() == CardInterface::Three &&
        allcards[size - 1]->getFace() == CardInterface::Two)
    {
        count_shun += 1;
        size -= 1;
        firstFace = CardInterface::nTwo;
    }

    for (size_t i = 1; i < size; ++i)
    {
        if (allcards[i]->getFace() >= CardInterface::LittleJoker)
        {
            return false;
        }
        if ((allcards[i - 1]->getFace() + 1) == allcards[i]->getFace())
        {
            count_shun += 1;
        }
        else
        {
            return false;
        }
    }

    if (count_shun != static_cast<std::int32_t>(cards.size()))
    {
        return false;
    }

    if (count_shun == 13)
    {
        firstFace = allcards[0]->getFace();
    }

    return true;
}

bool Run4Referee::IsPairs(const Cards& cards, int& count_pair, CardInterface::Face& firstFace)
{
    if (type_ == PLAY_TYPE::TYPE_TWO) return IsPairs_Ex(cards, count_pair, firstFace);

    count_pair = 0;
    auto size = cards.size();
    Cards allcards(cards);
    std::sort(allcards.begin(), allcards.end(), ComparePoker);

    if (size < 4)
    {
        return false;
    }

    if ((size % 2) != 0)
    {
        return false;
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
    
    if ((count_pair * 2) != static_cast<std::int32_t>(size))
    {
        return false;
    }

    firstFace = allcards[0]->getFace();

    return true;
}

bool Run4Referee::IsPairs_Ex(const Cards& cards, int& count_pair, CardInterface::Face& firstFace)
{
    count_pair = 0;
    auto size = cards.size();
    Cards allcards(cards);
    std::sort(allcards.begin(), allcards.end(), ComparePoker);

    if (size < 4)
    {
        return false;
    }

    if ((size % 2) != 0)
    {
        return false;
    }

    firstFace = allcards[0]->getFace();
    if (allcards[size - 1]->getFace() == CardInterface::Two &&
        allcards[size - 3]->getFace() == CardInterface::Ace &&
        allcards[0]->getFace() == CardInterface::Three)
    {
        count_pair += 2;
        size -= 4;
        firstFace = CardInterface::nAce;
    }
    else if (allcards[0]->getFace() == CardInterface::Three &&
        allcards[size - 1]->getFace() == CardInterface::Two)
    {
        count_pair += 1;
        size -= 2;
        firstFace = CardInterface::nTwo;
    }

    for (size_t i = 1; i < size - 2; i += 2)
    {
        if (allcards[i]->getFace() >= CardInterface::LittleJoker)
        {
            return false;
        }
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

	size = cards.size();
    if (allcards[size - 1]->getFace() == allcards[size - 2]->getFace())
    {
        if (allcards[size - 1]->getFace() >= CardInterface::LittleJoker)
        {
            return false;
        }
        else
        {
            count_pair += 1;
        }
    }
    else
    {
        return false;
    }

    if ((count_pair * 2) != static_cast<std::int32_t>(cards.size()))
    {
        return false;
    }

    if (count_pair == 13)
    {
        firstFace = allcards[0]->getFace();
    }

    return true;
}

bool Run4Referee::IsFly(const Cards& cards, int& count_fly, CardInterface::Face& firstFace)
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

bool Run4Referee::IsBomb(const Cards& cards)
{
    if (cards.size() < 4)
    {
        return false;
    }

    auto handCardsInfo = getCardInfo(cards);
    if (handCardsInfo.size() == 1)
    {
        if (handCardsInfo[0].suits.size() >= 4)
        {
            return true;
        }
    }

    for (auto iter : cards)
    {
        if (iter->getSuit() != CardInterface::NONESUIT)
        {
            return false;
        }
    }

    return true;
}

std::vector<CardInfo> Run4Referee::getCardInfo(const Cards& handCards)
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
            //cardInfo.count = 1;
            cardInfo.suits.push_back(allcards[i]->getSuit());
            handCardsInfo.push_back(cardInfo);
        }
        else
        {
            iter->suits.push_back(allcards[i]->getSuit());
            // iter->count += 1;
        }
    }

    return handCardsInfo;
}

Cards Run4Referee::FindShunCard(const PlayedCard& left, const std::vector<CardInfo>& handCardsInfo, int count, size_t handcardscount)
{
    if (type_ == PLAY_TYPE::TYPE_TWO) return FindShunCard_Ex(left, handCardsInfo, count,handcardscount);

    Cards cards;
    auto size = handCardsInfo.size();
    int cout_pairs = 1;
    int index = 0;
    for (size_t i = 0; i < size - 1; ++i)
    {
        if (handCardsInfo[i].face <= left.getFirstFace() ||
            handCardsInfo[i + 1].face >= CardInterface::Two
            )
        {
            index = 0;
            cout_pairs = 1;
            continue;
        }
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

    //如果是飞机需加上翅膀
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

Cards Run4Referee::FindShunCard_Ex(const PlayedCard& left, const std::vector<CardInfo>& handCardsInfo, int count, size_t handcardscount)
{
    Cards cards;
    auto size = handCardsInfo.size();
    int cout_pairs = 1;
    int index = 0;

    for (size_t i = 0; i < size - 1; ++i)
    {
        if (handCardsInfo[i].face <= left.getFirstFace() ||
            handCardsInfo[i + 1].face > CardInterface::Two)
        {
            index = 0;
            cout_pairs = 1;
            continue;
        }
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
        else if (left.getFirstFace() == CardInterface::nAce && cout_pairs == left.getCount() - 1)
        {
            if (handCardsInfo[index - cout_pairs + 1].face == CardInterface::Three &&
                handCardsInfo[i + 1].face != CardInterface::Two &&
                IsHaveCard(CardInterface::Two, handCardsInfo) == true)
            {
                auto card = CardFactory::MakePokerCard(CardInterface::Face::Two, CardInterface::Suit::Diamonds);
                cards.push_back(card);
                break;
            }
        }
    }

    if (static_cast<std::int32_t>(cout_pairs + cards.size()) != left.getCount())
    {
        cards.clear();
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

    //如果是飞机需加上翅膀
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

bool Run4Referee::SelectWing(int index, int size, const std::vector<CardInfo>& handCardsInfo, Cards& cards)
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

Cards Run4Referee::FindThreeZone(const PlayedCard& left, const std::vector<CardInfo>& handCardsInfo, size_t handcardscount)
{
    Cards cards;
    CardInterface::Face face;

    for (auto iter : handCardsInfo)
    {
        if (iter.suits.size() >= 3 &&
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
        cards.clear();
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

Cards Run4Referee::FindThree(const PlayedCard& left, const std::vector<CardInfo>& handCardsInfo, size_t handcardscount)
{
    Cards cards;

    for (auto iter : handCardsInfo)
    {
        if (iter.suits.size() >= 3 &&
            iter.face > left.getFirstFace())
        {
            std::sort(iter.suits.begin(), iter.suits.end());
            for (int s = 0; s < 3; ++s)
            {
                auto card = CardFactory::MakePokerCard(iter.face, iter.suits[s]);
                cards.push_back(card);
            }
            break;
        }
    }
    if (cards.size() != 3)
    {
        cards.clear();
    }

    return cards;
}

Cards Run4Referee::FindBomb(const PlayedCard& left, const std::vector<CardInfo>& handCardsInfo)
{
    Cards cards;
    if (left.getCards()[0]->getSuit() == CardInterface::NONESUIT)
    {
        return cards;
    }

    for (auto iter : handCardsInfo)
    {
        if (iter.suits.size() < 4)
        {
            continue;
        }

        if (iter.suits.size() == left.getCards().size() &&
            iter.face > left.getCards()[0]->getFace())
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

    return GetWangCards(handCardsInfo);
}

Cards Run4Referee::FindBomb(const std::vector<CardInfo>& handCardsInfo)
{
    Cards cards;
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

    return GetWangCards(handCardsInfo);
}

Cards Run4Referee::GetWangCards(const std::vector<CardInfo>& handCardsInfo)
{
    Cards cards;
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

bool Run4Referee::IsInHandCard(const Cards& playedCard, const Cards& handCards)
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

Cards Run4Referee::SortCard(const Cards& playedCard, CardType type, int count, CardInterface::Face firstFace)
{
    Cards cards(playedCard);
    std::sort(cards.begin(), cards.end(), ComparePoker);

    Cards newCards;

    if (cards.size() == 4 &&
        type != TYPE_BOMB)
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

bool Run4Referee::IsEndFly(const Cards& cards)
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

bool Run4Referee::IsEndfly_EndThreeZone(const Cards& cards, CardInterface::Face& firstFace, CardType& type, int& count_fly)
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

	if (static_cast<std::int32_t>(size) >(temp * 5))
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

bool Run4Referee::IsHaveHei3(const Cards& handCards)
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

Cards Run4Referee::FindTHShunCard(const PlayedCard& left, const Cards cards, int count, size_t handcardscount)
{
    Cards rcards(cards);
    std::sort(rcards.begin(), rcards.end(), ComparePoker);

    Cards lcards(left.getCards());
    std::sort(lcards.begin(), lcards.end(), ComparePoker);

    std::shared_ptr<CardInterface> lfirstcard = lcards[0];
    if (left.getType() == TYPE_SHUNZI && IsSameType(lcards) == true)
    {
        for (auto iter : rcards)
        {
            if (iter->getFace() < lfirstcard->getFace())
            {
                continue;
            }
            else if (iter->getFace() == lfirstcard->getFace())
            {
                if (iter->getSuit() > lfirstcard->getSuit())
                {
                    auto recards = GetTHShunCards(rcards, iter, left.getCards().size());
                    if (recards.size() != 0)
                    {
                        return recards;
                    }
                }
            }
            else
            {
                auto recards = GetTHShunCards(rcards, iter, left.getCards().size());
                if (recards.size() != 0)
                {
                    return recards;
                }
            }
        }
    }
    else
    {
        for (auto iter : rcards)
        {
            auto recards = GetTHShunCards(rcards, iter, left.getCards().size());
            if (recards.size() != 0)
            {
                return recards;
            }
        }
    }

    return Cards();
}

Cards Run4Referee::GetTHShunCards(const Cards cards, std::shared_ptr<CardInterface> firstcard, const std::int32_t count)
{
    if (type_ == PLAY_TYPE::TYPE_TWO) return GetTHShunCards_Ex(cards, firstcard, count);

    //DLOG(INFO) << "cards:" << cards << ",firstcard:=" << firstcard->getName() << ",count:" << count;

    Cards rescards;
    if (firstcard->getFace() > CardInterface::Nine)
    {
        return rescards;
    }
    
    rescards.push_back(firstcard);
    for (auto i = 1; i < count; ++i)
    {
        auto card = CardFactory::MakePokerCard(static_cast<CardInterface::Face>(firstcard->getFace() + i), firstcard->getSuit());
        if (IsHaveCard(card, cards) == true)
        {
            rescards.push_back(card);
        }
        else
        {
            rescards.clear();
            return rescards;
        }
    }

    if (static_cast<std::int32_t>(rescards.size()) != count)
    {
        rescards.clear();
    }

    return rescards;
}

Cards Run4Referee::GetTHShunCards_Ex(const Cards cards, std::shared_ptr<CardInterface> firstcard, std::int32_t count)
{
    //DLOG(INFO) << "cards:" << cards << ",firstcard:=" << firstcard->getName() ;

    Cards rescards;
    if (firstcard->getFace() > CardInterface::Jack)
    {
        return rescards;
    }

    auto temp_count = count;
    rescards.push_back(firstcard);
    if (firstcard->getFace() == CardInterface::Three)
    {
        auto card1 = CardFactory::MakePokerCard(CardInterface::Face::Ace, firstcard->getSuit());
        auto card2 = CardFactory::MakePokerCard(CardInterface::Face::Two, firstcard->getSuit());
        if (IsHaveCard(card1, cards) == true && IsHaveCard(card2, cards) == true)
        {
            rescards.push_back(card1);
            rescards.push_back(card2);
            count -= 2;
        }
        else if (IsHaveCard(card2, cards) == true)
        {
            rescards.push_back(card2);
            count -= 1;
        }
    }

    //DLOG(INFO) << "cards:" << cards << ",firstcard:=" << firstcard->getName() << ",count:" << count;

    for (auto i = 1; i < count; ++i)
    {
        auto card = CardFactory::MakePokerCard(static_cast<CardInterface::Face>(firstcard->getFace() + i), firstcard->getSuit());
        if (IsHaveCard(card, cards) == true)
        {
            rescards.push_back(card);
        }
        else
        {
            rescards.clear();
            return rescards;
        }
    }

    if (static_cast<std::int32_t>(rescards.size()) != temp_count)
    {
        rescards.clear();
    }

    return rescards;
}

bool Run4Referee::IsSameType(const Cards& cards)
{
    std::set<CardInterface::Suit> suits;
    for (const auto& iter : cards)
    {
        suits.insert(iter->getSuit());
    }

    if (suits.size() == 1)
    {
        return true;
    }

    return false;
}

bool Run4Referee::IsHaveCard(std::shared_ptr<CardInterface> card, const Cards cards)
{
    for (auto iter : cards)
    {
        if (iter->getName() == card->getName())
        {
            return true;
        }
    }

    return false;
}

bool Run4Referee::IsHaveCard(CardInterface::Face face, const std::vector<CardInfo>& handCardsInfo)
{
    for (auto iter : handCardsInfo)
    {
        if (iter.face == face)
        {
            return true;
        }
    }

    return false;
}

void Run4Referee::SetType(PLAY_TYPE type)
{
    type_ = type;
}
