#include "play_logic.h"
#include <algorithm>

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

int PlayLogic::Compare(const PlayedCard& left, const PlayedCard& right)
{
    auto leftType = left.getType();
    auto rightType = right.getType();

    if (leftType == TYPE_BOMB &&
        rightType == TYPE_BOMB)
    {
        return left.getCards()[0]->getFace() < right.getCards()[0]->getFace() ? -1 : 1;
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
        switch (leftType)
        {
        case TYPE_ONE:
        case TYPE_ONEPAIR:
        case TYPE_THREE:
            return left.getCards()[0]->getFace() > right.getCards()[0]->getFace() ? 1 : -1;
        case TYPE_PAIRS:
        case TYPE_SHUNZI:
        case TYPE_THREE_ZONE:
        case TYPE_FLY:
            return left.getFirstFace() > right.getFirstFace() ? 1 : -1;
        default:
            return 0;
        }
    }

    return 0;
}

CardType PlayLogic::getCardType(const Cards& cards, int& count, CardInterface::Face& firstFace)
{
    Cards clone(cards);
    std::sort(clone.begin(), clone.end());

    DLOG(INFO) << "PlayLogic::getCardType() cards:=" << clone;

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
    else if (size == 4 &&
        clone[0]->getFace() == clone[3]->getFace()
        )
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

bool PlayLogic::isMaxCard(const Cards& cards, const Cards& rightHandCards)
{
    Cards clone(rightHandCards);
    std::sort(clone.begin(), clone.end(), ComparePoker);
    if (cards[0]->getFace() != clone[clone.size() - 1]->getFace())
    {
        return false;
    }

    return true;
}

Cards PlayLogic::GetMaxCard(const std::vector<CardInfo>& handCardsInfo)
{
    Cards cards;
    auto iter = handCardsInfo.rbegin();
    if (iter->suits.size() == 4)
    {
        for (auto it : iter->suits)
        {
            auto card = CardFactory::MakePokerCard(iter->face, it);
            cards.push_back(card);
        }
        return cards;
    }
  
    auto card = CardFactory::MakePokerCard(iter->face, iter->suits[0]);
    cards.push_back(card);

    return cards;
}

Cards  PlayLogic::autoPlay(const PlayedCard& left, const Cards& rightHandCards, std::int32_t nseat_size)
{
    Cards clone(rightHandCards);
    std::sort(clone.begin(), clone.end(), ComparePoker);

    DLOG(INFO) << "PlayLogic::autoPlay() PlayedCards:=" << left.getCards()
        << " type:=" << left.getType() << " count:= " << left.getCount() << " fristface:= " << left.getFirstFace();
    DLOG(INFO) << "PlayLogic::autoPlay()   HandCards:=" << clone;

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
        Cards max_cards = GetMaxCard(handCardsInfo);
        if (max_cards[0]->getFace() > left.getCards()[0]->getFace())
        {
            cards = max_cards;
        }

        if (nseat_size == 1)
        {
            return cards;
        }
        auto res_cards = FindOne(left.getCards(), handCardsInfo);
        if (res_cards.size() != 0)
        {
            return res_cards;
        }
        else
        {
            if (cards.size() != 0)
            {
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
        cards = FindShunCard(left, handCardsInfo, 1, rightHandCards.size());
        break;
    case TYPE_FLY:
        cards = FindShunCard(left, handCardsInfo, 3, rightHandCards.size());
        break;
    case TYPE_BOMB:
        return FindBomb(left, handCardsInfo);
    default:
        break;
    }

    if (cards.size() == 0 &&
        bombCards.size() == 4)
    {
        return bombCards;
    }

    DLOG(INFO) << "PlayLogic::autoPlay()   ReturnCards:=" << cards.size();
    return cards;
}

Cards PlayLogic::autoPlay(const Cards& handCards)
{
    int count = 0;
    CardInterface::Face firstface;
    Cards clone(handCards);
    auto cards_type = getCardType(clone, count, firstface);
    if (cards_type != TYPE_INVALID)
    {
        return clone;
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

Cards PlayLogic::autoPlay_AI(const Cards& rightHandCards, std::int32_t nseat_size)
{
    Cards cards;
    if (rightHandCards.empty())
    {
        return cards;
    }
    Cards clone(rightHandCards);
    auto handCardsInfo = getCardInfo(clone);
    std::sort(handCardsInfo.begin(), handCardsInfo.end(), [](const CardInfo& left, const CardInfo& right) {
        return left.face < right.face;
    });

    if ( !FindCard(CardInterface::Face::Two, clone,1) && 
         !FindCard(CardInterface::Face::Ace, clone,2))
    {
        cards = FindShunCard(handCardsInfo, 3);
        if (cards.size() > 0)
        {
            return cards;
        }

        cards = FindShunCard(handCardsInfo, 1);
        if (cards.size() > 0)
        {
            auto res = CheckShunCard(cards, handCardsInfo);
            if (res)
            {
                return cards;
            }
        }

        cards = FindShunCard(handCardsInfo, 2);
        if (cards.size() > 0)
        {
            return cards;
        }

        cards = FindThreeZone(handCardsInfo);
        if (cards.size() > 0)
        {
            return cards;
        }
    }

    if (nseat_size == 1)
    {
        cards = FindPairs(handCardsInfo);
        if (cards.size() > 0)
        {
            return cards;
        }
        return GetMaxCard(handCardsInfo);
    }
    else if (nseat_size == 2)
    {
        return GetMinCard(handCardsInfo);
    }

    std::sort(handCardsInfo.begin(), handCardsInfo.end(), [](const CardInfo& left, const CardInfo& right) {
        return left.face < right.face;
    });
    Cards bomb;
    for (auto iter : handCardsInfo)
    {
        if (iter.suits.size() == 4u)
        {
            if (bomb.size() != 0)
            {
                continue;
            }
            for (auto suit_iter : iter.suits)
            {
                auto card = CardFactory::MakePokerCard(iter.face, suit_iter);
                bomb.push_back(card);
            }
            continue;
        }
        for (auto suit_iter : iter.suits)
        {
            auto card = CardFactory::MakePokerCard(iter.face, suit_iter);
            cards.push_back(card);
        }
        break;
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
   
    if (cards.size() == 0)
    {
        return bomb;
    }

    return cards;
}

bool PlayLogic::IsShunZi(const Cards& cards, int& count_shun, CardInterface::Face& firstFace)
{
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

    firstFace = allcards[0]->getFace();
    return true;
}

bool PlayLogic::IsPairs(const Cards& cards, int& count_pair, CardInterface::Face& firstFace)
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

bool PlayLogic::IsFly(const Cards& cards, int& count_fly, CardInterface::Face& firstFace)
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
        if (iter.suits.size() == 3u)
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

std::vector<CardInfo> PlayLogic::getCardInfo(const Cards& handCards)
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

Cards PlayLogic::FindShunCard(const PlayedCard& left, const std::vector<CardInfo>& handCardsInfo, int count, size_t handcardscount)
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
        if (handCardsInfo[i].suits.size() == 4 ||
            handCardsInfo[i + 1].suits.size() == 4)
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

bool PlayLogic::SelectWing(int index, int size, const std::vector<CardInfo>& handCardsInfo, Cards& cards)
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


Cards PlayLogic::FindThreeZone(const PlayedCard& left, const std::vector<CardInfo>& handCardsInfo, size_t handcardscount)
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

Cards PlayLogic::FindBomb(const PlayedCard& left, const std::vector<CardInfo>& handCardsInfo)
{
    Cards cards;
    for (auto iter : handCardsInfo)
    {
        if (iter.suits.size() == 4 &&
            iter.face > left.getCards()[0]->getFace())
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
}

Cards PlayLogic::FindBomb(const std::vector<CardInfo>& handCardsInfo)
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
    return cards;
}

bool PlayLogic::IsInHandCard(const Cards& playedCard, const Cards& handCards)
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

Cards PlayLogic::SortCard(const Cards& playedCard, CardType type, int count, CardInterface::Face firstFace)
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

Cards PlayLogic::FindOne(const Cards& cards, const std::vector<CardInfo>& handCardsInfo)
{
    auto shun_cards = FindShunCard(handCardsInfo,1);

    Cards res_cards;
    for (auto iter : handCardsInfo)
    {
        if (iter.face > cards[0]->getFace())
        {
            auto it = std::find_if(shun_cards.begin(), shun_cards.end(), [&iter](const std::shared_ptr<CardInterface> value) {
                return iter.face == value->getFace();
            });
            if (it != shun_cards.end())
            {
                if (iter.suits.size() == 2)
                {
                    auto card = CardFactory::MakePokerCard(iter.face, iter.suits[0]);
                    res_cards.push_back(card);
                    break;
                }
                else
                {
                    continue;
                }
            }
           if (iter.suits.size() > 1)
           {
               continue;
           }
           auto card = CardFactory::MakePokerCard(iter.face, iter.suits[0]);
           res_cards.push_back(card);
           break;
        }
    }

    return res_cards;
}

Cards PlayLogic::FindShunCard(const std::vector<CardInfo>& handCardsInfo, int count)
{
    Cards cards;
    auto size = handCardsInfo.size();
    int cout_pairs = 1;
    int index = 0;

    auto num = 2;
    if (count == 1)
    {
        num = 5;
    }

    for (size_t i = 0; i < size - 1; ++i)
    {
        if (handCardsInfo[i + 1].face == CardInterface::Two ||
            handCardsInfo[i + 1].face == CardInterface::Ace)
        {
            if (cout_pairs >= num)
            {
                break;
            }
            else
            {
                index = 0;
                cout_pairs = 1;
            }
            continue;
        }
        if (handCardsInfo[i].suits.size() == 4 ||
            handCardsInfo[i + 1].suits.size() == 4)
        {
            if (cout_pairs >= num)
            {
                break;
            }
            else
            {
                index = 0;
                cout_pairs = 1;
            }
            continue;
        }
        if (handCardsInfo[i].suits.size() < static_cast<std::uint32_t>(count) ||
            handCardsInfo[i + 1].suits.size() < static_cast<std::uint32_t>(count))
        {
            if (cout_pairs >= num)
            {
                break;
            }
            else
            {
                index = 0;
                cout_pairs = 1;
            }
            continue;
        }
        if ((handCardsInfo[i].face + 1) == handCardsInfo[i + 1].face)
        {
            index = i + 1;
            cout_pairs += 1;
        }
        else
        {
            if (cout_pairs >= num)
            {
                break;
            }
            else
            {
                index = 0;
                cout_pairs = 1;
            }
        }
    }

    if (cout_pairs < num)
    {
        return cards;
    }
    auto count_num = index;
    if (cout_pairs >= 8)
    {
        count_num = index - (cout_pairs - 8);
    }

    for (int i = index - cout_pairs + 1; i <= count_num; ++i)
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
        auto res = SelectWing(index - cout_pairs + 1, cout_pairs, handCardsInfo, cards);
        if (!res)
        {
            cards.clear();
        }
    }

    return cards;
}

bool PlayLogic::CheckShunCard(const Cards& cards, const std::vector<CardInfo>& handCardsInfo)
{
    size_t num = 0;
    for (auto iter : cards)
    {
        auto it = std::find_if(handCardsInfo.begin(), handCardsInfo.end(), [&iter](const CardInfo& value) {
            return iter->getFace() == value.face;
        });
        if (it == handCardsInfo.end())
        {
            break;
        }
        if (it->suits.size() > 1)
        {
            num += 1;
        }
        if (it->suits.size() == 4u)
        {
            return false;
        }
    }

    if (cards.size() / 2 >= num)
    {
        return true;
    }

    return false;
}

Cards PlayLogic::FindThreeZone(const std::vector<CardInfo>& handCardsInfo)
{
    Cards cards;
    CardInterface::Face face;

    for (auto iter : handCardsInfo)
    {
        if (iter.face == CardInterface::Ace)
        {
            continue;
        }
        if (iter.suits.size() == 3)
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

    return cards;
}

Cards PlayLogic::FindPairs(const std::vector<CardInfo>& handCardsInfo)
{
    Cards cards;

    for (auto iter : handCardsInfo)
    {
        if (iter.suits.size() == 2)
        {
            for (auto it : iter.suits)
            {
                auto card = CardFactory::MakePokerCard(iter.face, it);
                cards.push_back(card);
            }
            break;
        }
    }

    return cards;
}

bool PlayLogic::IsEndFly(const Cards& cards)
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

bool PlayLogic::FindCard(const CardInterface::Face face, const Cards& cards, const std::int32_t num)
{
    auto card_num = 0;
    for (auto iter : cards)
    {
        if (iter->getFace() == face)
        {
            card_num += 1;
        }
        if (card_num == num)
        {
            return true;
        }
    }

    return false;
}

Cards PlayLogic::GetMinCard(const std::vector<CardInfo>& handCardsInfo)
{
    Cards cards;
    auto iter = handCardsInfo.begin();
   
    auto card = CardFactory::MakePokerCard(iter->face, iter->suits[0]);
    cards.push_back(card);

    return cards;
}
