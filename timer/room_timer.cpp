#include "room_timer.h"

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

#include "Table.h"

RoomTimer::RoomTimer(boost::asio::io_service & ios):ios_(ios), queue_(nullptr)
{

}

RoomTimer::~RoomTimer(void)
{
	if (queue_ != nullptr)
	{
		queue_->Destroy();
		queue_ = nullptr;
	}
}

boost::int32_t RoomTimer::Init()
{
    DCHECK(queue_ == nullptr);
	queue_ = new  assistx2::timer2::TimerQueue(ios_);
	if (queue_ == nullptr)
	{
		return -1;
	}

	return 0;	
}

bool RoomTimer::CancelTimer( const RoomTimerContext & id )
{
	boost::shared_ptr<assistx2::timer2::TimerContext > context;
	return queue_->CancelTimer(id, context);
}

boost::int32_t RoomTimer::NewTimer( long expires_from_now_millisecond, Trigger_type trigger, 
						boost::int32_t room, RoomTimerContext::TimerType type,  Seat * seat )
{
	DCHECK_NE(type,  RoomTimerContext::DELAY_BROADCAST_TIMER);

	boost::shared_ptr<RoomTimerContext> context(new EventTimerContext(trigger, room, type, seat) );

	return queue_->NewTimer(context, expires_from_now_millisecond);
}

boost::int32_t RoomTimer::NewTimer( long expires_from_now_millisecond, Trigger_type trigger, 
						boost::int32_t room, boost::shared_ptr<assistx2::Stream> stream, DelayTimerContext::BroadCastType type, uid_type mid)
{
	boost::shared_ptr<RoomTimerContext> context(new DelayTimerContext(trigger, room, stream, type, mid) );

	return queue_->NewTimer(context, expires_from_now_millisecond);
}

bool EventTimerContext::Equal( const TimerContext * other ) const
{
	const EventTimerContext * other_context = dynamic_cast<const EventTimerContext *>(other);

	if (other_context == nullptr)
	{
		return false;
	}
	 
	if ( other_context->type_ != type_ || other_context->room_ != room_)
	{
		return false;
	}

    if (seat_ == nullptr)
	{
        DCHECK(other_context->seat_ == nullptr);
		return true;
	}
	else
	{
		return other_context->seat_->no_ == seat_->no_;
	}
}
