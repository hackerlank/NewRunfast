#include "helper.h"

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

#include "Table.h"
#include "card_interface.h"
#include "player_interface.h"

extern std::vector<std::int32_t > g_ip_white_list;

bool IsLivePlayer(const std::string room_type, Seat * seat)
{
	if (room_type[0] == 'S' || room_type[0] == 'J')
	{
		return seat->ingame();
	}
	else
	{
		return (seat->ingame() == true && seat->lastop_ != PlayerInterface::Fold);
	}
}

std::string ToString( const Cards & cards )
{
	std::stringstream stream;

	for (Cards::const_iterator it = cards.begin(); it != cards.end(); ++it)
	{
		stream<<(*it)->getName()<<",";
	}

	return stream.str().substr(0, stream.str().size() - 1);
}

json_spirit::Array & ToJson(const Cards & cards, json_spirit::Array &  array)
{
    for (Cards::const_iterator it = cards.begin(); it != cards.end(); ++it)
    {
        array.push_back((*it)->getName());
    }

    return array;
}

json_spirit::Array ToJson( const Cards & cards )
{
	json_spirit::Array array;

	ToJson(cards, array);

	return array;
}

bool IsNullSeat( Seat * seat )
{
	return seat->user_ == NULL;
}

bool IsNotNullSeat(Seat * seat)
{
	return seat->user_ != NULL;
}

bool IPChecker::operator()( Seat * seat ) const
{
	const char INVALID_IP_HEAD[] ="192.168";

    DCHECK(seat != NULL);
	if (seat->user_ != NULL)
	{

		if (find(g_ip_white_list.begin(), g_ip_white_list.end(), seat->user_->GetUID()) != g_ip_white_list.end())
		{
			return false;
		}

		const std::string & ip = seat->user_->GetLoginAddr();
		if (ip.empty() == true ||  strncmp(ip.c_str(), INVALID_IP_HEAD, strlen(INVALID_IP_HEAD)) == 0)
		{
			return false;
		}
		else if (ip_ == ip)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

void ActivatingHelper::operator()( Seat * seat )
{
	DCHECK(seat->bet_chips_amount_ == 0);
	DCHECK(seat->win_chips_ == 0);
	DCHECK(seat->show_cards_.empty() );
	DCHECK(seat->holecards_.empty() );

	if (seat->user_ != NULL )
	{
		DCHECK_EQ( (seat->status_ & Seat::PLAYER_STATUS_PLAYING),  0);
		DCHECK_NE( (seat->status_ & Seat::PLAYER_STATUS_WAITING), 0);
		DCHECK_EQ(seat->kickout_, false);
		DCHECK_EQ(seat->bet_, 0);
		DCHECK_EQ(seat->gift_chips_, 0);

		seat->show_cards_.clear();
		seat->holecards_.clear();
		seat->handstrength_.Clear();
		seat->status_						= ( seat->status_ | Seat::PLAYER_STATUS_WAITING );
		seat->lastop_						= PlayerInterface::None;
		seat->maxbet_						= 0;
		seat->minraise_					= 0;
		seat->betting_time_			= 0;
		seat->win_chips_					= 0;
		seat->ranking_						= 0;
		seat->bet_chips_amount_	= 0;
		seat->handhog_					= -1;
		seat->show_ranking_			= INVALID_RANKING;
		seat->readying_					= false;

		if (room_cfg_.type[0] == 'S' || room_cfg_.type[0] == 'J')
		{
			seat->bankroll_ = seat->user_->GetGameBaseInfo().gold();
			if (seat->bankroll_ < room_cfg_.minchips)
			{
				return;
			}
		}
		else
		{
			DCHECK_EQ(room_cfg_.type[0], 'G');
			if (seat->bankroll_ < room_cfg_.sb )
			{
				return;
			}
		}

		DCHECK(seat->buytime_ == 0);
		//DCHECK( (seat->status_ & Seat::PLAYER_STATUS_BUY_CHIPS) == 0);
		DCHECK(seat->bankroll_ > room_cfg_.taxation);

		++counter_;

		DLOG_IF(INFO, PlayerInterface::IsRobot(seat->user_ ) == false )<<"ActivatingHelper mid:="<<seat->user_->GetUID()
			<<", bankroll_:="<<seat->bankroll_<<", gold:="<<seat->user_->GetGameBaseInfo().gold();

		if (room_cfg_.taxes_mode == FIXED_TAXES_MODE)
		{
			DCHECK_GT(seat->bankroll_, room_cfg_.taxation);
			seat->bankroll_ -= room_cfg_.taxation;

			if (PlayerInterface::IsRobot(seat->user_) == false)
			{
				taxation_ += room_cfg_.taxation;
			}
		}

		seat->status_	=  ((seat->status_ & ~ Seat::PLAYER_STATUS_WAITING) | Seat::PLAYER_STATUS_PLAYING);
	}
	else
	{
		DCHECK_EQ(seat->status_, Seat::PLAYER_STATUS_WAITING);
	}
}
