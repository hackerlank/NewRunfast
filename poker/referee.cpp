#include "referee.h"

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

#include <algorithm>

#include "xPoker.h"


std::string g_aRankingDesc[15] =
{
	"Ã»Å£",//NIN_None,
	"Å£¶¡",//NIN_Ace,
	"Å£¶þ",//NIN_Two,
	"Å£Èý",//NIN_Three,
	"Å£ËÄ",//NIN_Four,
	"Å£Îå",//NIN_Five,
	"Å£Áù",//NIN_Six,
	"Å£Æß",//NIN_Seven,
	"Å£°Ë",//NIN_Eight,
	"Å£¾Å",//NIN_Nine,
	"Å£Å£",//NIN_Nin,						//Å£Å£
	"°×Æ¤Å£",//NIN_WHITE,					//°×Æ¤Å£ //white
	"ËÄÕ¨",//NIN_BOMB,					//ËÄÌõ
	"Îå»¨Å£",//NIN_FULL,						//ÂúÅ£, Îå»¨Å£
	"ÎåÐ¡Å£",//NIN_SMALL					//ÎåÐ¡Å£
};

class PokerStyleGameLogicImpl : public Referee
{
public:
	PokerStyleGameLogicImpl() {}
	virtual ~PokerStyleGameLogicImpl() {}

	virtual int Compare(const HandStrength & left, const HandStrength & right) const
	{
		DCHECK_EQ(left.GetRank().front()->Compare(*left.GetRank().back() ), -1);
		DCHECK_EQ(right.GetRank().front()->Compare(*right.GetRank().back() ), -1);

		if (left.getRanking() == right.getRanking())
		{
			if (left.getRanking() == NIN_BOMB )
			{
				return left.GetRank().at(1)->Compare(*right.GetRank().at(1) );
			}
			else
			{
				return left.GetRank().back()->Compare(*right.GetRank().back() );
			}
		}
		else
		{
			return left.getRanking() < right.getRanking() ? -1 : 1;
		}
	}
	
	virtual bool getStrength( const Cards & holdcards, const Cards & commcards, HandStrength & strength ) const
	{
		DCHECK_EQ(commcards.size(), 3);
		DCHECK_EQ(holdcards.size(), 2);

		Cards allcards(holdcards);
		allcards.insert(allcards.begin(), commcards.begin(), commcards.end() );
		std::sort(allcards.begin(), allcards.end(), ComparePoker);

		DCHECK_EQ(allcards.front()->Compare(*allcards.back() ), -1);

		strength.cards_.insert(strength.cards_.end(), holdcards.begin(), holdcards.end());
		std::sort(strength.cards_.begin(), strength.cards_.end(), ComparePoker);

		DCHECK_EQ(allcards.size(), 5);

		if ( static_cast<int>(allcards[4]->getFace() ) < static_cast<int>(NIN_Five) &&
			(allcards[0]->getFace() + allcards[1]->getFace() + allcards[2]->getFace() + allcards[3]->getFace() + allcards[4]->getFace() )<= 10)
		{
			strength.ranking_ = NIN_SMALL;			
		}
		else if (allcards[0]->getFace() > CardInterface::Ten)
		{
			strength.ranking_ = NIN_FULL;
		}
		else if ( allcards[0]->getFace() == allcards[3]->getFace() ||  allcards[1]->getFace() == allcards[4]->getFace()) 
		{
			strength.ranking_ = NIN_BOMB;
		}
		else
		{
			bool hasniu = false;

			int c1 = 0;
			int c2 = 0;
			int c3 = 0;
			for (c1 = 0; c1 < 5; ++c1)
			{
				for ( c2 = c1 + 1; c2 < 5; ++c2)
				{
					for ( c3 = c2 + 1; c3 < 5 ; ++c3)
					{
						if ((allcards[c1]->getValue() + allcards[c2]->getValue() + allcards[c3]->getValue()) % 10 == 0)
						{
							c1 = allcards[c1]->getValue();
							c2 = allcards[c2]->getValue();
							c3 = allcards[c3]->getValue();
							allcards.erase( std::find_if(allcards.begin(), allcards.end(), std::bind2nd(CompareCardByValue(),  c1 )) );
							allcards.erase( std::find_if(allcards.begin(), allcards.end(), std::bind2nd(CompareCardByValue(), c2 )) );
							allcards.erase( std::find_if(allcards.begin(), allcards.end(), std::bind2nd(CompareCardByValue(), c3 )) );
							hasniu = true;
							break;
						}
					}

					if (hasniu == true)
					{
						break;
					}
				}

				if (hasniu == true)
				{
					break;
				}
			}

			if (hasniu == true)
			{
				const int ranking = (( allcards[0]->getValue() + allcards[1]->getValue() ) % 10);
				strength.ranking_ = static_cast<Ranking >(ranking == 0 ? (NIN_Nin) :  ranking);
			}
			else
			{
				strength.ranking_ = NIN_None;
			}
		}

		return true;
	}

	virtual void getStrength( const Cards & allcards, HandStrength & strength ) const
	{
		DCHECK_EQ(allcards.size() , 5);

		strength.Clear();

		Cards cards_clone(allcards.begin(), allcards.end());

		std::sort(cards_clone.begin(), cards_clone.end(), ComparePoker );

		DCHECK_EQ(cards_clone.front()->Compare(*cards_clone.back() ), -1);
		DCHECK_EQ(cards_clone.back()->Compare(*cards_clone.front() ), 1);

		if ( static_cast<int>(cards_clone[4]->getFace() ) < static_cast<int>(NIN_Five) &&
			(cards_clone[0]->getFace() + cards_clone[1]->getFace() + cards_clone[2]->getFace() + cards_clone[3]->getFace() + cards_clone[4]->getFace() )<= 10)
		{
			strength.ranking_ = NIN_SMALL;

			strength.cards_.insert(strength.cards_.end(), cards_clone.begin(), cards_clone.end());
		}
		else if (cards_clone[0]->getFace() > CardInterface::Ten)
		{
			strength.ranking_ = NIN_FULL;

			strength.cards_.insert(strength.cards_.end(), cards_clone.begin(), cards_clone.end());
		}
		else if ( cards_clone[0]->getFace() == cards_clone[3]->getFace() ||  cards_clone[1]->getFace() == cards_clone[4]->getFace()) 
		{
			strength.ranking_ = NIN_BOMB;

			if (cards_clone[0]->getFace() == cards_clone[1]->getFace())
			{
				strength.cards_.insert(strength.cards_.end(), cards_clone.begin(), cards_clone.end());
			}
			else
			{
				strength.cards_.push_back(cards_clone.at(1 ) );
				strength.cards_.push_back(cards_clone.at(2 ) );
				strength.cards_.push_back(cards_clone.at(3 ) );
				strength.cards_.push_back(cards_clone.at(4 ) );
				strength.cards_.push_back(cards_clone.at(0 ) );
			}
		}
		else
		{
			bool hasniu = false;

			int c1 = 0;
			int c2 = 0;
			int c3 = 0;
			for (c1 = 0; c1 < 5; ++c1)
			{
				for ( c2 = c1 + 1; c2 < 5; ++c2)
				{
					for ( c3 = c2 + 1; c3 < 5 ; ++c3)
					{
						if ((cards_clone[c1]->getValue() + cards_clone[c2]->getValue() + cards_clone[c3]->getValue()) % 10 == 0)
						{
							strength.cards_.push_back(cards_clone[c1]);
							strength.cards_.push_back(cards_clone[c2]);
							strength.cards_.push_back(cards_clone[c3]);

							c1 = cards_clone[c1]->getValue();
							c2 = cards_clone[c2]->getValue();
							c3 = cards_clone[c3]->getValue();

							cards_clone.erase( std::find_if(cards_clone.begin(), cards_clone.end(), std::bind2nd(CompareCardByValue(),  c1 )) );
							cards_clone.erase( std::find_if(cards_clone.begin(), cards_clone.end(), std::bind2nd(CompareCardByValue(), c2 )) );
							cards_clone.erase( std::find_if(cards_clone.begin(), cards_clone.end(), std::bind2nd(CompareCardByValue(), c3 )) );

							strength.cards_.insert(strength.cards_.end(), cards_clone.begin(), cards_clone.end());

							hasniu = true;
							break;
						}
					}

					if (hasniu == true)
					{
						break;
					}
				}

				if (hasniu == true)
				{
					break;
				}
			}

			if (hasniu == true)
			{
				const int ranking = (( cards_clone[0]->getValue() + cards_clone[1]->getValue() ) % 10);
				strength.ranking_ = static_cast<Ranking >(ranking == 0 ? (NIN_Nin) :  ranking);
			}
			else
			{
				strength.ranking_ = NIN_None;
				strength.cards_.insert(strength.cards_.end(), cards_clone.begin(), cards_clone.end());
			}

			DCHECK_EQ(strength.cards_.size(), 5);
		}
	}
};

class MaJiangrStyleGameLogicImpl : public Referee
{
public:
	MaJiangrStyleGameLogicImpl() {}
	virtual ~MaJiangrStyleGameLogicImpl() {}

	virtual bool getStrength(const Cards & , const Cards & , HandStrength & ) const
	{
		return false;
	}

	virtual int Compare(const HandStrength & left, const HandStrength & right) const
	{
		if (left.getRanking() == right.getRanking())
		{
			if (left.getRanking() == NIN_BOMB)
			{
				return left.GetRank().at(1)->Compare(*right.GetRank().at(1) );
			}
			else
			{
				Cards left_cards(left.GetRank() );

				std::sort(left_cards.begin(), left_cards.end(), ComparePoker );

				Cards right_cards(right.GetRank() );

				std::sort(right_cards.begin(), right_cards.end(), ComparePoker );

				DCHECK_EQ(left_cards.front()->Compare(*left_cards.back() ), -1 );
				DCHECK_EQ(right_cards.front()->Compare(*right_cards.back() ), -1 );

				return left_cards.back()->Compare(*right_cards.back() );
			}
		}
		else
		{
			return left.getRanking() < right.getRanking() ? -1 : 1;
		}
	}

	virtual void getStrength(const Cards & allcards, HandStrength & strength) const
	{
		DCHECK_EQ(allcards.size() , 5);

		strength.Clear();

		Cards cards_clone(allcards.begin(), allcards.end());

		std::sort(cards_clone.begin(), cards_clone.end(), ComparePoker );

		DCHECK_EQ(cards_clone.front()->Compare(*cards_clone.back() ), -1);

		CHECK_LT(cards_clone.front()->getFace(), cards_clone.back()->getFace() );

		if ( cards_clone[0]->getFace() == cards_clone[3]->getFace() ||  cards_clone[1]->getFace() == cards_clone[4]->getFace()) 
		{
			strength.ranking_ = NIN_BOMB;

			if (cards_clone[0]->getFace() == cards_clone[1]->getFace())
			{
				strength.cards_.insert(strength.cards_.end(), cards_clone.begin(), cards_clone.end());
			}
			else
			{
				strength.cards_.push_back(cards_clone.at(1 ) );
				strength.cards_.push_back(cards_clone.at(2 ) );
				strength.cards_.push_back(cards_clone.at(3 ) );
				strength.cards_.push_back(cards_clone.at(4 ) );
				strength.cards_.push_back(cards_clone.at(0 ) );
			}
		}
		else if (cards_clone[0]->getFace() == CardInterface::Three &&
			cards_clone[1]->getFace() == CardInterface::Eight &&
			cards_clone[2]->getFace() == CardInterface::Nine &&
			cards_clone[3]->getFace() == CardInterface::Ten &&
			cards_clone[4]->getFace() == CardInterface::Ten)
		{
			strength.ranking_ = NIN_WHITE;
			strength.cards_.insert(strength.cards_.end(), cards_clone.begin(), cards_clone.end());
		}
		else
		{
			bool hasniu = false;

			int c1 = 0;
			int c2 = 0;
			int c3 = 0;
			for (c1 = 0; c1 < 5; ++c1)
			{
				for ( c2 = c1 + 1; c2 < 5; ++c2)
				{
					for ( c3 = c2 + 1; c3 < 5 ; ++c3)
					{
						if ((cards_clone[c1]->getValue() + cards_clone[c2]->getValue() + cards_clone[c3]->getValue()) % 10 == 0)
						{
							strength.cards_.push_back(cards_clone[c1]);
							strength.cards_.push_back(cards_clone[c2]);
							strength.cards_.push_back(cards_clone[c3]);

							c1 = cards_clone[c1]->getValue();
							c2 = cards_clone[c2]->getValue();
							c3 = cards_clone[c3]->getValue();

							cards_clone.erase( std::find_if(cards_clone.begin(), cards_clone.end(), std::bind2nd(CompareCardByValue(),  c1 )) );
							cards_clone.erase( std::find_if(cards_clone.begin(), cards_clone.end(), std::bind2nd(CompareCardByValue(), c2 )) );
							cards_clone.erase( std::find_if(cards_clone.begin(), cards_clone.end(), std::bind2nd(CompareCardByValue(), c3 )) );

							strength.cards_.insert(strength.cards_.end(), cards_clone.begin(), cards_clone.end());

							hasniu = true;
							break;
						}
					}

					if (hasniu == true)
					{
						break;
					}
				}

				if (hasniu == true)
				{
					break;
				}
			}

			if (hasniu == true)
			{
				const int ranking = (( cards_clone[0]->getValue() + cards_clone[1]->getValue() ) % 10);
				strength.ranking_ = static_cast<Ranking >(ranking == 0 ? (NIN_Nin) :  ranking);
			}
			else
			{
				strength.ranking_ = NIN_None;
				strength.cards_.insert(strength.cards_.end(), cards_clone.begin(), cards_clone.end());
			}

			DCHECK_EQ(strength.cards_.size(), 5);
		}
	}
};

Referee * CreateReferee( char room_type )
{
	if (room_type == 'J' || room_type == 'B')
	{
		return new MaJiangrStyleGameLogicImpl();
	}
	else
	{
		//DCHECK(room_type == 'G' || room_type == 'S' || room_type == 'K' || room_type == 'Z');
		return new PokerStyleGameLogicImpl();
	}
}

bool CompareAdaptation( Referee * referee, const HandStrength & left, const HandStrength & right )
{
	return referee->Compare(left, right) == 1;
}

void SortByShowCards( const HandStrength & hs, Cards & cards )
{
	DCHECK_EQ(cards.size(), 0);
	DCHECK_EQ(hs.GetRank().size(), 5);

	if (hs.getRanking() == NIN_BOMB && hs.GetRank().at(0)->getFace() != hs.GetRank().at(1)->getFace())
	{
		cards.push_back(hs.GetRank().at(1 ) );
		cards.push_back(hs.GetRank().at(2 ) );
		cards.push_back(hs.GetRank().at(3 ) );
		cards.push_back(hs.GetRank().at(4 ) );
		cards.push_back(hs.GetRank().at(0 ) );
	}
	else
	{
		cards.insert(cards.end(), hs.GetRank().begin(), hs.GetRank().end() );
	}
}

