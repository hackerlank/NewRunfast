#include "poker_card.h"

#include <cassert>
#include <stdexcept>
#include <iostream>

static const char face_symbols[] = 
{
   '3', '4', '5', '6', '7', '8', '9','T', 'J', 'Q', 'K','A', '2','L','B'
};

static const char suit_symbols[] = 
{
	's', 'h',  'd', 'c','w'
};

PokerCard::PokerCard(Face f, Suit s): CardBase(f, s)
{
	name_.resize(2, '\0');
	name_.at(0) = getFaceSymbol();
	name_.at(1) = getSuitSymbol();
}

char PokerCard::getFaceSymbol() const
{	
	return face_symbols[getFace() - CardInterface::FirstFace];
}

char PokerCard::getSuitSymbol() const
{
	return suit_symbols[getSuit() - CardInterface::FirstSuit];
}

std::string PokerCard::getName() const
{
	return  name_;
}

void PokerCard::setChangeName(const std::string& name)
{
	change_name_ = name;
}

std::string PokerCard::getChangeName() const
{
	return change_name_;
}

CardInterface::Face PokerCard::convertFaceSymbol(char fsym)
{
	for (unsigned int i = CardInterface::FirstFace; i <= CardInterface::LastFace; ++i)
	{
		if (fsym == face_symbols[i - CardInterface::FirstFace])
		{
			return static_cast<CardInterface::Face>(i);
		}
	}

	throw std::invalid_argument("") ;
}

CardInterface::Suit PokerCard::convertSuitSymbol(char ssym)
{
	for (unsigned int i = CardInterface::FirstSuit; i <= CardInterface::LastSuit; ++i)
	{
		if (ssym == suit_symbols[i - CardInterface::FirstSuit])
		{	
			return static_cast<CardInterface::Suit>(i);
		}
	}

	throw std::invalid_argument("") ;
}

int PokerCard::Compare( const CardInterface & c )
{
	if (getFace() == c.getFace())
	{
		return getSuit() == c.getSuit() ? 0 : (getSuit() < c.getSuit() ? -1 : 1);
	}
	else
	{
		return (getFace() < c.getFace() ? -1 : 1);
	}
}

int PokerCard::getValue() const
{
	return getFace() <= Ten ? getFace() : Ten;
}

std::shared_ptr<CardInterface > CardFactory::MakePokerCard(CardInterface::Face face, CardInterface::Suit suit)
{
	std::shared_ptr<CardInterface > card(new PokerCard(face, suit) );

	return card;
}

std::shared_ptr<CardInterface > CardFactory::MakePokerCard(const std::string & name)
{
	std::shared_ptr< CardInterface > card;

	try
	{
		CardInterface::Face f = PokerCard::convertFaceSymbol(name.at(0) );
		CardInterface::Suit s = PokerCard::convertSuitSymbol(name.at(1) );

		card = std::shared_ptr<CardInterface >(new PokerCard(f, s) );
	}
	catch (...)
	{
		std::cout  <<"CardFactory::MakePokerCard FAILED, invalid_argument:=" << name << std::endl;
	}

	return card;
}