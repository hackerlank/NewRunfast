#ifndef _XPDKPOKER_POKER_CARD_INTERFACE_H_
#define _XPDKPOKER_POKER_CARD_INTERFACE_H_

#include <vector>
#include <string>
#include <functional>
#include <memory>

class CardInterface
{
public:
	enum Face
	{
        nAce = -1,
        nTwo = 0,
		FirstFace =1,
		Three = 1,
		Four,
		Five,
		Six,
		Seven,
		Eight,
		Nine,
		Ten,
		Jack,
		Queen,
		King,
		Ace ,
		Two,
        LittleJoker,
        BigJoker,
        LastFace = BigJoker,
	};

	enum Suit
	{
		FirstSuit		= 1,					
		Diamonds	=1,					//方块
		Clubs,									//梅花
		Hearts,								//红桃
		Spades,								//黑桃
        NONESUIT ,					//没有花色，不区分花色
		LastSuit		= NONESUIT
	} ;

	CardInterface() {}
	virtual ~CardInterface() {}

	virtual Face getFace() const = 0;
	virtual Suit getSuit() const = 0;

	virtual int getValue()const = 0;
	
	virtual std::string getName() const = 0;

	virtual void setChangeName(const std::string&) = 0;
	virtual std::string getChangeName() const = 0;

	// == 返回 0， 小于返回 -1， 大于返回 1
	virtual int Compare(const CardInterface & ) = 0;
};

class CompareCard : public std::binary_function<std::shared_ptr< CardInterface >, std::shared_ptr< CardInterface >, bool>
{
public:
	bool operator()(const std::shared_ptr< CardInterface > & left, const std::shared_ptr< CardInterface > & right) const
	{
		return left->getValue() == right->getValue() ;
	}
};

class CompareCardByValue : public std::binary_function<std::shared_ptr< CardInterface >, int, bool>
{
public:
	bool operator()(const std::shared_ptr< CardInterface > & left, const int & right) const
	{
		return left->getValue() == right;
	}
};

typedef std::vector<std::shared_ptr< CardInterface > > Cards;

bool ComparePoker(const std::shared_ptr<CardInterface > & left, const std::shared_ptr<CardInterface > & right);

bool operator < (const std::shared_ptr<CardInterface > & , const std::shared_ptr<CardInterface > & );

bool operator == (const std::shared_ptr<CardInterface > & , const std::shared_ptr<CardInterface > & );

bool operator > (const std::shared_ptr<CardInterface > & , const std::shared_ptr<CardInterface > & );

std::ostream& operator << (std::ostream& stream,const Cards& cards);

class CardFactory
{
public:
	static std::shared_ptr<CardInterface > MakePokerCard(const std::string & name);

	static std::shared_ptr<CardInterface > MakePokerCard(CardInterface::Face face, CardInterface::Suit suit);

	static std::shared_ptr<CardInterface > MakeMaJiangCard(const std::string & name);

	static std::shared_ptr<CardInterface > MakeMaJiangCard(CardInterface::Face face);
};

#endif //_XPDKPOKER_POKER_CARD_INTERFACE_H_