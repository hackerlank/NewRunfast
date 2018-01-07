#ifndef _XNNPOKER_POKER_MAJIANG_CARD_H_
#define _XNNPOKER_POKER_MAJIANG_CARD_H_

#include "card_base.h"

class MaJiangCard : public CardBase
{
public:
	explicit MaJiangCard(Face face);

	virtual ~MaJiangCard(void);

	virtual int getValue()const
	{
		return getFace();
	}

	virtual std::string getName() const;

	// == 返回 0， 小于返回 -1， 大于返回 1
	virtual int Compare(const CardInterface & card);
};

#endif //_XNNPOKER_POKER_MAJIANG_CARD_H_
