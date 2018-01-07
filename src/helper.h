#ifndef _XPOKER_SRC_HELPER_
#define _XPOKER_SRC_HELPER_

#include <assistx2/stream.h>
#include <assistx2/json_wrapper.h>

#include <boost/noncopyable.hpp>

#include "xpoker.h"

#include "card_interface.h"

#define Prt(exp) ""#exp":"<<exp<<","
#define PrtN(name_,exp) ""<< name_ << ":"<<exp<<","


class Seat;

//匹配一个参与了游戏的并且未弃牌的玩家
bool IsLivePlayer(const std::string room_type, Seat * seat);

bool IsNullSeat(Seat * seat);

bool IsNotNullSeat(Seat * seat);

json_spirit::Array ToJson(const Cards & cards);

std::string ToString(const Cards & cards);

json_spirit::Array & ToJson( const Cards & cards, json_spirit::Array &  array);

class LivePlayerCounter : public boost::noncopyable
{
public:
	explicit LivePlayerCounter(const std::string & roomtype):roomtype_(roomtype), count_(0) {}

	virtual ~LivePlayerCounter() {}

	void operator()(Seat * seat)
	{
		if (IsLivePlayer(roomtype_, seat) == true)
		{
			++count_;
		}
	}

	const std::string & roomtype_;

	boost::int32_t count_;
};

class IPChecker
{
public:
	explicit IPChecker(const std::string & ip):ip_(ip) {}

	bool operator()(Seat * seat)const;

private:
	std::string ip_;
};

class ActivatingHelper
{
public:
	explicit ActivatingHelper (const roomcfg_type & cfg):room_cfg_(cfg), counter_(0), taxation_(0) {}
	~ActivatingHelper () {}

	void operator()(Seat * seat);

private:
	roomcfg_type room_cfg_;

public:
	boost::int32_t counter_;

	chips_type taxation_;
};

#endif //_XPOKER_SRC_HELPER_

