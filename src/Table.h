#ifndef _XPOKER_SRC_TABLE_H_
#define _XPOKER_SRC_TABLE_H_

#include <algorithm>

#include "SeatImpl.h"

class Table
{
public:
	const static std::int32_t BEGIN_SEAT			=	(1);

	//const static std::int32_t END_SEAT				= (3);

	const static std::int32_t INVALID_SEAT		=	(0);

	//const static std::int32_t SEAT_AMOUNT	=	(3);

public:
	Table();

	Table(const std::string & room_type, std::int32_t max_seat);
	~Table(void);

	//座下
    std::int32_t Enter(PlayerInterface * player, const std::int32_t seat);

	//站起
    std::int32_t Quit(PlayerInterface * player);

    std::int32_t GetPlayerCount()const
	{
		return playercount_;
	}

	//返回坐位总数
    std::int32_t GetSeatCount() const
	{
		return seats_.size();
	}

private:
	Seat * get(const std::int32_t seat);

public:
	class CycleIterator;
	
	class Iterator
	{
		friend class Table::CycleIterator;
	public:
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;
		typedef std::forward_iterator_tag iterator_category;
		typedef Seat *        value_type;
		typedef Seat **           pointer;
		typedef Seat *&         reference;

		Iterator(Table * table, const std::int32_t position);

		Iterator(const Iterator & it);

		Iterator operator ++();

		Seat * operator ->();

		Seat * operator *();

		bool operator == (const Iterator & it)const;

		bool operator != (const Iterator & it)const;

	private:
		Table * table_;

        std::int32_t position_;
	};

	class CycleIterator
	{
	protected:
		Table * table_;

        std::int32_t position_;
	public:
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;
		typedef std::forward_iterator_tag iterator_category;
		typedef Seat *        value_type;
		typedef Seat **           pointer;
		typedef Seat *&         reference;

		CycleIterator(Table * table, const std::int32_t position);

		CycleIterator(const CycleIterator & it);

		CycleIterator(const Iterator & it);

		CycleIterator operator ++();

		Seat * operator ->();

		Seat * operator *();

		bool operator == (const CycleIterator & it)const;

		bool operator != (const CycleIterator & it)const;

		bool operator != (const Iterator & it)const;
	};

	Iterator begin();

	Iterator end();

	Iterator find(const std::int32_t seat);
		
private:
    std::int32_t playercount_ = 0;

	std::vector<Seat * > seats_;
};

#endif //_XPOKER_SRC_TABLE_H_