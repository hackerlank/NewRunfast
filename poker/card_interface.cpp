#include "card_interface.h"

bool ComparePoker( const std::shared_ptr<CardInterface > & left, const std::shared_ptr<CardInterface > & right )
{
	return left->Compare(*right) < 0;
}

bool operator < (const std::shared_ptr<CardInterface > & left, const std::shared_ptr<CardInterface > & right)
{
	return left->Compare(*right) == -1;
}

bool operator == (const std::shared_ptr<CardInterface > & left, const std::shared_ptr<CardInterface > & right)
{
	return left->Compare(*right) == 0;
}

bool operator > (const std::shared_ptr<CardInterface > & left, const std::shared_ptr<CardInterface > & right)
{
	return left->Compare(*right) == 1;
}

std::ostream& operator << (std::ostream& stream, const Cards& cards)
{
	std::string strCards;
	for (auto iter : cards)
	{
		strCards += iter->getName() + ",";
	}
	return stream << strCards;
}