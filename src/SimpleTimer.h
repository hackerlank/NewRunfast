#pragma once

#include <boost/asio.hpp>

class SimpleTimer
{
private:
	SimpleTimer();

public:
	explicit SimpleTimer(boost::asio::io_service & ios):timer_(ios)
	{

	}

	virtual ~SimpleTimer();

	virtual std::int32_t OnAlarm(const boost::system::error_code & error) = 0;

	void EnableAlarm(long delay);

	void DisableAlam();

	
private:
	boost::asio::deadline_timer	timer_;

};

