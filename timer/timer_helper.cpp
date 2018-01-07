#include "timer_helper.h"

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

class ContextImpl : public assistx2::timer_wapper::Context
{
public:
	ContextImpl(std::function<void() > cb)
		:cb_(cb)
	{

	}

	virtual ~ContextImpl()
	{

	}

	virtual bool Compare(const Context & context)
	{
		return false;
	}

public:
	std::function<void() > cb_;
};

TimerHelper::TimerHelper() :timer_queue_(nullptr)
{

}

TimerHelper::~TimerHelper()
{
    if (timer_queue_ != nullptr)
	{
		timer_queue_->Destroy();
		timer_queue_ = nullptr;
	}
}

void TimerHelper::NewTimer(std::function< void() > cb, time_t delay)
{
	DCHECK(delay >= 1);

	boost::shared_ptr<assistx2::timer_wapper::Context > context(new ContextImpl(cb));
	timer_queue_->NewTimer(delay, context, boost::bind(&TimerHelper::OnTimer, this, _1));
}

void TimerHelper::OnTimer(boost::shared_ptr<assistx2::timer_wapper::Context > context)
{
	try
	{
		ContextImpl * data = dynamic_cast<ContextImpl *>(context.get());
		DCHECK(data != nullptr);
		data->cb_();
	}
	catch (...)
	{
		
	}
}

int32_t TimerHelper::Init(boost::asio::io_service & ios)
{
	timer_queue_ = new assistx2::timer_wapper::TimerQueue(ios);
	if (timer_queue_ == nullptr)
	{
		DCHECK(timer_queue_ != nullptr);
		return -1;
	}

	return 0;
}
