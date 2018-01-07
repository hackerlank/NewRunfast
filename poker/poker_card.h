#ifndef _XPDKPOKER_POKER_POKER_CARD_H_
#define _XPDKPOKER_POKER_POKER_CARD_H_

#include "card_base.h"

class PokerCard : public CardBase
{
public:
	PokerCard(Face f, Suit s);

	virtual int getValue()const;

	char getFaceSymbol() const;
	char getSuitSymbol() const;

	virtual std::string getName() const;

	virtual void setChangeName(const std::string& name);
	virtual std::string getChangeName() const;

	virtual int Compare(const CardInterface & c);

	static Face convertFaceSymbol(char fsym);
	static Suit convertSuitSymbol(char ssym);
};

#endif /* _XNNPOKER_POKER_POKER_CARD_H_ */


