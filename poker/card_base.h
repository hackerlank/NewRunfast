#ifndef _XPDKPOKER_POKER_CARDBASE_H_
#define _XPDKPOKER_POKER_CARDBASE_H_

#include "card_interface.h"

class CardBase : public CardInterface
{
private:
	CardBase(void);

public:
	CardBase(Face face, Suit  suit):face_(face), suit_(suit) {}

	virtual ~CardBase(void);

	virtual Face getFace() const
	{
		return face_;
	}

	virtual Suit getSuit() const
	{
		return suit_;
	}

	void setChangeName(const std::string&) {};
	std::string getChangeName() const {};

private:
	Face face_;//��С
	Suit  suit_;//��ɫ

protected:
	std::string name_;
	std::string change_name_;
};

#endif //_XPDKPOKER_POKER_CARDBASE_H_