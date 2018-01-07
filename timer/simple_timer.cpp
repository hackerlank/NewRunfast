#include "simple_timer.h"

#include <boost/bind.hpp>


SimpleTimer::~SimpleTimer()
{
}

void SimpleTimer::DisableAlam()
{
	boost::system::error_code e;
	timer_.cancel(e);
}

void SimpleTimer::EnableAlarm(long delay)
{
	boost::system::error_code ec;
	timer_.expires_from_now(boost::posix_time::seconds(delay), ec);
	timer_.async_wait(boost::bind(&SimpleTimer::OnAlarm, this, _1));
}
