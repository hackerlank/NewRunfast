#include "player_day_data.h"

#include <sstream>

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

#include <boost/date_time/gregorian/gregorian.hpp>

#include <assistx2/string_wrapper.h>

std::string PlayerDayData::ToString() const
{
	std::stringstream value;
	value<<yday_<<","<<gold_gift_count_<<","<<playtime_<<","<<round_count_<<","<<gold_incr_<<","<<max_win_chips_;

	return value.str();
}

void PlayerDayData::FromString( const std::string & str, PlayerDayData & data )
{
	std::vector<std::string > v;
	assistx2::SplitString(str, ", ", v);

	if (v.size() == 6 && assistx2::atoi_s(v[0], 0) == boost::gregorian::day_clock::local_day().day() )
	{
		data.yday_							= assistx2::atoi_s(v[0], 0);
		data.gold_gift_count_		= assistx2::atoi_s(v[1], 0);
		data.playtime_					= assistx2::atoi_s(v[2], 0);
		data.round_count_			= assistx2::atoi_s(v[3], 0);
		data.gold_incr_					= assistx2::atoi_s(v[4], 0);
		data.max_win_chips_		= assistx2::atoi_s(v[5], 0);
	}
	else
	{
		data.Clear();
	}
}

int PlayerDayData::IncrRoundCount()
{
	if (IsToday() == false)
	{
		Clear();
	}

	return ++round_count_;
}

int PlayerDayData::GetRoundCount()
{
	if (IsToday() == false)
	{
		Clear();
	}

	return round_count_;
}

void PlayerDayData::IncrGoldGiftCount()
{
	if (IsToday() == false)
	{
		Clear();
	}

	++gold_gift_count_;
}

int PlayerDayData::GetGoldGiftCount()
{
	if (IsToday() == false)
	{
		Clear();
	}

	return gold_gift_count_;
}

void PlayerDayData::SetMaxWin( const chips_type win_gold )
{
	if (IsToday() == false)
	{
		Clear();
	}

	if (max_win_chips_ <  win_gold)
	{
		max_win_chips_ = win_gold;
	}
}

time_t PlayerDayData::GetPlayTime()
{
	if (IsToday() == false)
	{
		Clear();
	}

	return playtime_;
}

time_t PlayerDayData::IncrPlayTime( const time_t incr )
{
	if (IsToday() == false)
	{
		Clear();
	}

	DLOG(INFO)<<"PlayerDayData::IncrPlayTime, incr:="<<incr<<", playtime_:="<<playtime_;

	return playtime_ += incr;
}

void PlayerDayData::SetGoldIncr( const chips_type incr )
{
	if (IsToday() == false)
	{
		Clear();
	}

	gold_incr_ += incr;
}

bool PlayerDayData::IsToday()
{
	return (yday_ == boost::gregorian::day_clock::local_day().day() );
}

void PlayerDayData::Clear()
{
	yday_ = boost::gregorian::day_clock::local_day().day();
	round_count_			= 0;
	gold_gift_count_		= 0;
	max_win_chips_		= 0;
	gold_incr_					= 0;
	playtime_					= 0;
}
