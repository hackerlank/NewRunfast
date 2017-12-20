#include "majiang_card.h"

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

static const std::string face_symbols[] = 
{
	"-", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"
};


MaJiangCard::MaJiangCard(Face face):CardBase(face, CardInterface::NONESUIT)
{
	DCHECK_GE(face, CardInterface::Ace);
	DCHECK_LE(face, CardInterface::Ten);
}


MaJiangCard::~MaJiangCard(void)
{

}

std::string MaJiangCard::getName() const
{
	DCHECK_EQ(getSuit(), CardInterface::NONESUIT);

	DCHECK_GE(getFace(), CardInterface::Ace);
	DCHECK_LE(getFace(), CardInterface::Ten);

	return face_symbols[getFace()];
}

int MaJiangCard::Compare( const CardInterface & card )
{
	if (getFace() == card.getFace() )
	{
		return 0;
	}
	else if (getFace() < card.getFace() )
	{
		return -1;
	}
	else
	{
		return 1;
	}
}

std::shared_ptr<CardInterface > CardFactory::MakeMaJiangCard(const std::string & name)
{
	for (std::size_t index = CardInterface::Ace; index < sizeof(face_symbols) / sizeof(face_symbols[0]); ++index)
	{
        if (name == face_symbols[index])
		{
			DCHECK_GE(static_cast<CardInterface::Face>(index), CardInterface::Ace);
			DCHECK_LE(static_cast<CardInterface::Face>(index), CardInterface::Ten);
      std::shared_ptr<CardInterface > card(new MaJiangCard(static_cast<CardInterface::Face>(index) ) );

			return card;
		}
	}

	LOG(FATAL)<<"CardFactory::MakeMaJiangCard, INVALID CARD:="<<name;
	return std::shared_ptr<CardInterface >();
}

std::shared_ptr<CardInterface > CardFactory::MakeMaJiangCard(CardInterface::Face face)
{
	DCHECK_GE(face, CardInterface::Ace);
	DCHECK_LE(face, CardInterface::Ten);

  std::shared_ptr<CardInterface > card(new MaJiangCard(face) );

	return card;
}