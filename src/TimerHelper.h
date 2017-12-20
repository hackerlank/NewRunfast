#ifndef _XPOKER_PLUGIN_TIMER_HELPER_H_
#define _XPOKER_PLUGIN_TIMER_HELPER_H_

#include <functional>

#include <assistx2/timer_wrapper.h>
#include <assistx2/singleton.h>

class TimerHelper
{
public:
	TimerHelper();
	~TimerHelper();

	std::int32_t Init(boost::asio::io_service & ios);

	void NewTimer( std::function< void () > cb, time_t delay);

	void OnTimer(boost::shared_ptr<assistx2::timer_wapper::Context > context);

private:
	assistx2::timer_wapper::TimerQueue * timer_queue_;
};

typedef Singleton<TimerHelper > GlobalTimerProxy;
#endif //_XPOKER_PLUGIN_TIMER_HELPER_H_

