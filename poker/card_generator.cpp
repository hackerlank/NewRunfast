#include "card_generator.h"
#include <algorithm>

//#define TEST_CARD

CardGenerator::CardGenerator():
  cards_(),
  player_count_(0),
  pop_count_(0),
  count_(0)
{

}

CardGenerator::CardGenerator(GeneratorType type):
  cards_(),
  player_count_(0),
  pop_count_(0),
  count_(0)
{

}

CardGenerator::~CardGenerator()
{

}

void CardGenerator::Reset(int player_count, std::int32_t type)
{
  player_count_ = player_count;
  pop_count_ = 0;

  cards_.clear();
  switch (type)
  {
  case 13:
  case 15:
  case 16:
      MakeCards_One(type);
      break;
  case 27:
      MakeCards_Two(type);
      break;
  default:
      break;
  }
  

#ifdef TEST_CARD
  MakeTestCard();
#else
  std::random_shuffle(cards_.begin(), cards_.end());
  std::random_shuffle(cards_.begin(), cards_.end());
#endif // _DEBUG
  count_ += 1;
}

std::shared_ptr< CardInterface > CardGenerator::Pop()
{
#ifndef TEST_CARD
    std::random_shuffle(cards_.begin(), cards_.end());
#endif
  auto card = cards_.back();
  cards_.pop_back();

  return card;
}

int CardGenerator::count() const
{
  return static_cast<int>(cards_.size());
}

void CardGenerator::MakeCards_One(std::int32_t type)
{
    for (int f = CardInterface::Three; f <= CardInterface::Two; ++f)
    {
        for (int s = CardInterface::Diamonds; s <= CardInterface::Spades; ++s)
        {
            if (type == 16 || type == 15)
            {
                //去掉黑桃A
                if (CardInterface::Ace == f &&
                    CardInterface::Spades == s
                    )
                {
                    continue;
                }
                //只保留红桃2
                if (CardInterface::Two == f &&
                    CardInterface::Hearts != s
                    )
                {
                    continue;
                }
            }

            if (type == 15)
            {
                //去掉黑桃A
                if (CardInterface::King == f &&
                    CardInterface::Spades == s
                    )
                {
                    continue;
                }
                //只保留红桃A
                if (CardInterface::Ace == f &&
                    CardInterface::Hearts != s
                    )
                {
                    continue;
                }
            }

            auto card = CardFactory::MakePokerCard(static_cast<CardInterface::Face>(f), static_cast<CardInterface::Suit>(s));
            cards_.push_back(card);

            std::random_shuffle(cards_.begin(), cards_.end());
            std::random_shuffle(cards_.begin(), cards_.end());
        }
        std::random_shuffle(cards_.begin(), cards_.end());
        std::random_shuffle(cards_.begin(), cards_.end());
    }
}

void CardGenerator::MakeCards_Two(std::int32_t type)
{
    for (int f = CardInterface::Three; f <= CardInterface::Two; ++f)
    {
        for (int s = CardInterface::Diamonds; s <= CardInterface::Spades; ++s)
        {
            auto card = CardFactory::MakePokerCard(static_cast<CardInterface::Face>(f), static_cast<CardInterface::Suit>(s));
            cards_.push_back(card);
            std::random_shuffle(cards_.begin(), cards_.end());
        }
        std::random_shuffle(cards_.begin(), cards_.end());
    }

    std::random_shuffle(cards_.begin(), cards_.end());
    for (int f = CardInterface::LittleJoker; f <= CardInterface::BigJoker; ++f)
    {
        auto card = CardFactory::MakePokerCard(static_cast<CardInterface::Face>(f), CardInterface::NONESUIT);
        cards_.push_back(card);
    }

    std::random_shuffle(cards_.begin(), cards_.end());
    for (int f = CardInterface::Three; f <= CardInterface::Two; ++f)
    {
        for (int s = CardInterface::Diamonds; s <= CardInterface::Spades; ++s)
        {
            auto card = CardFactory::MakePokerCard(static_cast<CardInterface::Face>(f), static_cast<CardInterface::Suit>(s));
            cards_.push_back(card);
        }
        std::random_shuffle(cards_.begin(), cards_.end());
    }
    std::random_shuffle(cards_.begin(), cards_.end());

    std::random_shuffle(cards_.begin(), cards_.end());
    for (int f = CardInterface::LittleJoker; f <= CardInterface::BigJoker; ++f)
    {
        auto card = CardFactory::MakePokerCard(static_cast<CardInterface::Face>(f), CardInterface::NONESUIT);
        cards_.push_back(card);
    }
}

void CardGenerator::MakeTestCard()
{
//     std::vector<std::string> cards = {
//         "9d","9h","7s","Ts","6h","5s","3d","8s","8h","3h","4d","Td","3c",
//         "4h","Ad","5h","4s","Qs","6c","8d","Th","Qd","Kh","Jd","Kd","7d","Ks","3s","9c",
//         "Jc","5c","2d","Tc","5d","7c","8c","7h","Qh","Jh","9s","6s","4c","Js","Qc","6d",
//     };
#if 0
    std::vector<std::string> cards = {
        "Kd","Kh","Kc","Th","Ts","6h","5s","3d","8s","8h","8c","3h","4d",
        "4h","Ad","5h","4s","Qs","6c","8d","Qd","Kh","Jd","2d","2s","2c"
        "Jc","5c","2d","Tc","5d","7c","7h","7s","Qh","Jh","9s","6s","4c",
       "3c","3s","Js","Qc","6d","Td","Kd","7d","Ks","3c","3s","Js","Qc",
        "3c","3s","Js","Qc","6d","Td","Kd","7d","Ks","3c","3s","Js","Qc",
       "3c","3s","Js","Qc","6d","Td","Kd","7d","Ks","3c","3s","Js","Qc",
       "4h","Ad","5h","4s","Qs","6c","8d","Qd","Kh","Jd","2d","2s","2c"
       "4h","Ad","5h","4s","Qs","6c","8d","Qd","Kh","Jd","2d","2s","2c"
       "4h","Ad","5h","4s","Qs","6c","8d","Qd","Kh","Jd","2d","2s","2c"
        "4h","Ad","5h","4s","Qs","6c","8d","Qd","Kh","Jd","2d","2s","2c"
        "4h","Ad","5h","4s","Qs","6c","8d","Qd","Kh","Jd","2d","2s","2c"
        "3c","3s","Js","Qc","6d","Td","Kd","7d","Ks","3c","3s","Js","Qc",
        "3c","3s","Js","Qc","6d","Td","Kd","7d","Ks","3c","3s","Js","Qc",
        "3c","3s","Js","Qc","6d","Td","Kd","7d","Ks","3c","3s","Js","Qc",
    };
#endif

    std::vector<std::string> cards = {
	

	
	"3c","3d","3h","3s","6d","6c","6h","6s","7c","7h","7d","7s","9c","9d","9h","9s",
	"4d","4c","4s","4h","8d","8c","8s","8h","Tc","Td","Th","Ts","5c","5d","5s","5h",
	
	

};   
cards_.clear();
    for (auto iter = cards.rbegin(); iter != cards.rend(); iter++)
    {
        auto card = CardFactory::MakePokerCard(*iter);
        cards_.push_back(card);
    }
}
