#ifndef _XPDKPOKER_CARD_GENERATOR_H_
#define _XPDKPOKER_CARD_GENERATOR_H_

#include "card_interface.h"
#include <memory>

class CardGenerator
{
public:
  enum GeneratorType { POKER_CARD_GENERATOR, MAJIANG_CARD_GENERATOR };
  enum { MAJIANG_CARDS_COUNT = 40 };
public:
  CardGenerator();
  explicit CardGenerator(GeneratorType type = POKER_CARD_GENERATOR);
  virtual ~CardGenerator();

  void Reset(int player_count = 0,std::int32_t type = 16);
  std::shared_ptr< CardInterface > Pop();
  int count() const;

  bool PopBestCard(Cards& vec) { return true; }

protected:
    void MakeCards_One(std::int32_t type);
    void MakeCards_Two(std::int32_t type);
    void MakeTestCard();
private:
  Cards cards_;
  int player_count_;
  int pop_count_;
  unsigned int count_;
};

#endif
