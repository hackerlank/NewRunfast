#include "Table.h"

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

#include "player_interface.h"

static const int32_t DEFAULT_PLAYER = 4;

Table::Table(const std::string & room_type, std::int32_t max_seat):playercount_(0)
{
    for (std::int32_t i = 1; i <= max_seat; ++i)
    {
        seats_.push_back(new Seat(i));
    }

    for (size_t it = 0; it < seats_.size(); it++)
    {
        seats_[it]->visible_ = true;
    }
}

Table::Table()
{
	//1-3ºÅ×ùÎ»
	for (std::int32_t i = 1; i <= DEFAULT_PLAYER; ++i)
	{
		auto seat = new Seat(i);
		seat->visible_ = true;

		seats_.push_back(seat );
	}
}

Table::~Table(void)
{
	for (std::vector<Seat * >::iterator it = seats_.begin(); it != seats_.end(); ++it)
	{
		delete *it;
	}

	seats_.clear();
}

std::int32_t Table::Enter( PlayerInterface * player, const std::int32_t seatno )
{
    DCHECK(seatno >= 1 && seatno <= GetSeatCount());
    DCHECK(player != nullptr);
    DCHECK(player->GetSeat() == INVALID_SEAT);

	Seat * seat = get(seatno);
	//
	if (seat->user_ != nullptr)
	{
    DCHECK(seat->user_ != nullptr);
		return -1;
	}
	else
	{
		seat->user_		= player;
		seat->status_	= Seat::PLAYER_STATUS_WAITING;

		player->SitDown(seatno);

		++playercount_;

		DLOG(INFO) << "Enter mid:=" << player->GetUID() << ", playercount_:=" << playercount_;
    DCHECK(playercount_ >= 1 && playercount_ <= GetSeatCount());

		return 0;
	}
}

std::int32_t Table::Quit( PlayerInterface * player )
{
    DCHECK(player != nullptr);
    DCHECK(player->GetSeat() != INVALID_SEAT);
    DCHECK(player->GetSeat() >= 1 && player->GetSeat() <= GetSeatCount());

	Seat * seat = get(player->GetSeat());

    DCHECK(seat != nullptr && seat->user_ != nullptr);
    DCHECK(player->GetUID() == seat->user_->GetUID());

	seat->user_ = nullptr;
	seat->status_ = Seat::PLAYER_STATUS_WAITING;

	player->SitUp();

	--playercount_;

	DLOG(INFO) << "Quit mid:=" << player->GetUID() << ", playercount_:=" << playercount_;

	return 0;
}

Seat * Table::get( const std::int32_t seat )
{
  DCHECK(seat >= 1 && seat <= GetSeatCount());
	return seats_.at(seat - 1);
}

Table::Iterator Table::begin()
{
	CHECK(get(BEGIN_SEAT)->visible_ == true);
	return Iterator(this, BEGIN_SEAT);
}

Table::Iterator Table::end()
{
	return Iterator(this, INVALID_SEAT);
}

Table::Iterator Table::find( const std::int32_t seat )
{
	if (seat < BEGIN_SEAT || seat > GetSeatCount())
	{
		return Iterator(this, INVALID_SEAT);
	}
	else
	{
		if (get(seat)->visible_ == false)
		{
			return Iterator(this, INVALID_SEAT);
		}
		else
		{
			return Iterator(this, seat);
		}
	}
}


Table::Iterator::Iterator( Table * table, const std::int32_t position ) :table_(table), position_(position)
{

}

Table::Iterator::Iterator( const Iterator & it ) :table_(it.table_), position_(it.position_)
{

}

Table::Iterator Table::Iterator::operator ++()
{
	for (++position_; position_ <= table_->GetSeatCount();++position_ )
	{
		if (table_->get(position_)->visible_ == true)
		{
			return Table::Iterator(table_, position_);
		}
	}

	position_ = INVALID_SEAT;

	return Table::Iterator(table_, INVALID_SEAT);
}

Seat * Table::Iterator::operator ->()
{
	CHECK_NE(position_, INVALID_SEAT);
	return table_->get(position_);
}

Seat * Table::Iterator::operator *()
{
	CHECK_NE(position_, INVALID_SEAT);
	return table_->get(position_);
}

bool Table::Iterator::operator == (const Iterator & it)const
{
	CHECK(table_ == it.table_);

	return position_ == it.position_;
}

bool Table::Iterator::operator != (const Table::Iterator & it)const
{
	CHECK(table_ == it.table_);

	return position_ != it.position_;
}


bool Table::CycleIterator::operator==( const CycleIterator & it ) const
{
	DCHECK(table_ == it.table_);
	return position_ == it.position_;
}

bool Table::CycleIterator::operator!=( const CycleIterator & it ) const
{
	DCHECK(table_ == it.table_);
	return position_ != it.position_;
}

bool Table::CycleIterator::operator!=( const Iterator & it ) const
{
	DCHECK(table_ == it.table_);
	return position_ != it.position_;
}

Table::CycleIterator::CycleIterator( Table * table, const std::int32_t position ) :table_(table), position_(position)
{

}

Table::CycleIterator::CycleIterator( const CycleIterator & it ) :table_(it.table_), position_(it.position_)
{

}

Table::CycleIterator::CycleIterator( const Iterator & it ) :table_(it.table_), position_(it.position_)
{

}

Table::CycleIterator Table::CycleIterator::operator ++()
{
	DCHECK_NE(position_, Table::INVALID_SEAT);
	DCHECK(table_->get(position_)->visible_ == true);

	for (++position_; position_ <= table_->GetSeatCount();++position_ )
	{
		if (table_->get(position_)->visible_ == true)
		{
			return Table::CycleIterator(table_, position_);
		}
	}

	position_ = Table::BEGIN_SEAT;
	return Table::CycleIterator(table_, position_);
}

Seat *Table:: CycleIterator::operator->()
{
	DCHECK_NE(position_, Table::INVALID_SEAT);
	return table_->get(position_);
}

Seat * Table::CycleIterator::operator *()
{
	DCHECK_NE(position_, Table::INVALID_SEAT);
	return table_->get(position_);
}


