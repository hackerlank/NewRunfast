#include "game_room.h"

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

#include "xpoker.h"
#include "poker_cmd.h"
#include "player_interface.h"
#include "table.h"
#include "room_listener.h"
#include "data_layer.h"
#include "room_timer.h"
#include "player_day_data.h"
#include "version.h"
#include "proxy_cmd.h"

static const std::size_t MAX_FRIENDS_COUNT = 256;

GameRoom::GameRoom(const boost::int32_t roomid, const roomcfg_type & cfg)
                   :GameRoomBase(roomid, cfg)
{

}

GameRoom::~GameRoom(void)
{

}

boost::int32_t GameRoom::OnMessage( PlayerInterface * player, assistx2::Stream * packet )
{
	switch(packet->GetCmd())
	{
	case Texas::CLEINT_REQUEST_ADD_FRIEND:
		OnRequestAddFriend(player, packet);
		break;
	case Texas::CLEINT_RESPONSE_ADD_FRIEND:
		OnResponseAddFriend(player, packet);
		break;
	case Texas::CLIENT_REQUEST_USE_FACE:
		OnUseFace(player, packet);
		break;
	default:
		return -1;
		break;
	}

	return 0;
}

void GameRoom::RoomSnapShot( PlayerInterface * player,  Players & players )
{
	
}

void GameRoom::RoomSnapShot( PlayerInterface * player )
{
	const std::size_t PAGE_SIZE = 10;

	DLOG(INFO) << "RoomSnapShot mid:=" << player->GetUID() << ", playersize:=" << players_.size();

	for (Players::iterator it = players_.begin(); it != players_.end();  )
	{
		Players::iterator end_it = it;
		for (std::size_t count = 0; count < PAGE_SIZE && end_it != players_.end(); ++count )
		{
			++end_it;
		}

		DLOG(INFO) << "RoomSnapShot mid:=" << player->GetUID() << ", playersize:=" << players_.size();

		DCHECK_LE(std::distance(it, end_it), PAGE_SIZE);

		Players players(it, end_it);
		RoomSnapShot(player,  players);

		it = end_it;
	}
}

void GameRoom::OnRequestAddFriend( PlayerInterface * player, assistx2::Stream * packet )
{
	const uid_type target = packet->Read<boost::int32_t>();

	//player->GetFriend().size() < MAX_FRIENDS_COUNT 最多MAX_FRIENDS_COUNT位好友
	if (target != player->GetUID() &&  player->GetFriend().size() < MAX_FRIENDS_COUNT
		&& player->GetFriend().find(target) == player->GetFriend().end() )
	{
		std::map<uid_type, PlayerInterface * >::iterator it = players_.find(target);
		if (it != players_.end() && it->second->GetFriend().size() < MAX_FRIENDS_COUNT)
		{
			assistx2::Stream stream(Texas::SERVER_BROADCAST_ADD_FRIEND);
			stream.Write(player->GetUID());
			stream.Write(target);
			stream.End();

			BroadCast(stream);
		}
	}
}

void GameRoom::OnResponseAddFriend( PlayerInterface * player, assistx2::Stream * packet )
{
	const uid_type from = packet->Read<boost::int32_t>();
	const uid_type err = packet->Read<boost::int32_t>();

	//player->GetFriend().size() < MAX_FRIENDS_COUNT 最多MAX_FRIENDS_COUNT位好友
	if (err == 0 && from != player->GetUID() && player->GetFriend().size() < MAX_FRIENDS_COUNT)
	{
		player->GetFriend().insert(from);

		std::map<uid_type, PlayerInterface * >::iterator it = players_.find(from);
		if (it != players_.end())
		{
			it->second->GetFriend().insert(player->GetUID());
		}

		DataLayer::getInstance()->AddPokerFriend(from, player->GetUID());
	}

	assistx2::Stream stream(Texas::SERVER_RESPONSE_ADD_FRIEND);
	stream.Write(from);
	stream.Write(player->GetUID());
	stream.Write(err);
	stream.End();

	BroadCast(stream);
}

void GameRoom::Init(KickCallbcak & call )
{
	kickcallback_ = call;

	Signal(RoomEventListener::ROOM_INIT_EVENT, nullptr);

	referee_ = CreateReferee(roomcfg_.type[0] );
	CHECK_NOTNULL(referee_);
}

void GameRoom::PeekWinlist( std::vector<HandStrength> & winlist )
{
	CHECK(false);
}

void GameRoom::BroadCastPlayerEnter( PlayerInterface * player )
{

}

boost::int32_t GameRoom::BlackListCheck( PlayerInterface * player )
{
	std::map<uid_type, time_t >::iterator mit = blacklisk_.find(player->GetUID());
	if (mit != blacklisk_.end() && mit->second > time(nullptr))
	{
		assistx2::Stream result(Texas::SERVER_RESPONSE_ENTER_ROOM);
		result.Write(Texas::error_code::ERROR_ENTER_ROOM_BLACKLIST);
		result.Write(XNNPOKER_VERSION);    
		result.Write(static_cast<std::int32_t>(mit->second - time(nullptr)));
		result.End();

		SendTo(player, result);

		return Texas::error_code::ERROR_ENTER_ROOM_BLACKLIST;
	}
	else
	{
		return 0;
	}
}

void GameRoom::PackShowCardsPacket( boost::shared_ptr<assistx2::Stream > & stream, Seat * seat )
{
	stream = boost::shared_ptr<assistx2::Stream >(new assistx2::Stream(Texas::SERVER_RESPONSE_SHOW_CARDS));
	stream->Write(seat->no_);
	DCHECK_EQ(seat->show_cards_.size(), 5U);
	for (auto it = seat->show_cards_.begin(); it != seat->show_cards_.end(); ++it)
	{
		stream->Write(*it);
	}

	DCHECK_NE(seat->show_ranking_, INVALID_RANKING);
	stream->Write(static_cast<boost::int32_t>(seat->show_ranking_));
	stream->End();
}

chips_type GameRoom::DeductedTaxes( const chips_type win_amount )
{
	if (roomcfg_.taxes_mode == FIXED_TAXES_MODE)
	{
		return roomcfg_.taxation;
	}
	else
	{
		chips_type taxes = 0;

		DCHECK(roomcfg_.taxes_mode == RATIO_TAXES_MODE);
		if (win_amount > 0)
		{
			taxes = static_cast<chips_type>(boost::int64_t(win_amount) * boost::int64_t(roomcfg_.taxation) / 1000 );
		}
		return taxes;
	}
}

void GameRoom::OnUseFace( PlayerInterface * player, assistx2::Stream * packet )
{
	const boost::int32_t pcate = packet->Read<boost::int32_t>();
	const boost::int32_t pframe = packet->Read<boost::int32_t>();

	assistx2::Stream stream(Texas::SERVER_RESPONSE_USE_FACE);
	stream.Write(player->GetUID());
	stream.Write(pcate);
	stream.Write(pframe);
	stream.End();

	BroadCast(stream);
}

void GameRoom::ForEachLivePlayer(std::function< bool(PlayerInterface *) > cb)
{
	for (Table::Iterator seat = table_->begin(); seat != table_->end(); ++seat)
	{
		//DCHECK(seat->bet_ == 0);

		if (seat->ingame() == true)
		{
			if (cb(seat->user_) == false)
			{
				break;
			}
		}
	}
}

std::int32_t GameRoom::GetLivePlayerCount()
{
	return GetTable()->GetPlayerCount();
// 	std::int32_t count = 0;
// 	for (Table::Iterator seat = table_->begin(); seat != table_->end(); ++seat)
// 	{
// 		if (seat->ingame() == true)
// 		{
// 			++count;
// 		}
// 	}
// 
// 	return count;
}

