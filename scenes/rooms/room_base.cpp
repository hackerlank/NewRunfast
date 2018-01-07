#include "room_base.h"

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

#include <json_spirit_writer_template.h>

#include "table.h"
#include "player_interface.h"
#include "room_listener.h"
#include "poker_cmd.h"
#include "data_layer.h"
#include "proxy_cmd.h"
#include "handle_obj.h"

RoomBase::RoomBase(const boost::int32_t roomid, const roombasecfg_type & cfg): obj_(GameObj::GetInstance()),
    RoomInterface(roomid), table_(new Table(cfg.type, cfg.seat)), roombasecfg_(cfg)
{

}

RoomBase::~RoomBase(void)
{
	if (table_ != nullptr)
	{
		delete table_;
		table_ = nullptr;
	}
}

void RoomBase::ReSetTable(Table* table)
{
    if (table_ != nullptr)
    {
        delete table_;
        table_ = nullptr;
    }
    table_ = table;
}

boost::int32_t RoomBase::Enter( PlayerInterface * user )
{
	DLOG(INFO)<<"RoomBase::Enter, mid:="<<user->GetUID()<<", room:="<<GetID();

	LOG_IF(ERROR, static_cast<size_t>(roombasecfg_.maxplayer) < players_.size() )
		<<"maxplayer:="<<roombasecfg_.maxplayer<<", players_:="<<players_.size();

	DCHECK(players_.find(user->GetUID()) == players_.end());

	if (roombasecfg_.maxplayer > players_.size() )
	{
		DLOG(INFO) << "Enter insert mid:=" << user->GetUID();
		players_.insert(std::make_pair(user->GetUID(), user));

		return 0;
	}
	else
	{
		return Texas::error_code::ERROR_ENTER_ROOM_FILLED;
	}
}

boost::int32_t RoomBase::Leave( PlayerInterface * user, std::int32_t err)
{
	DLOG(INFO)<<"RoomBase::Leave, mid:="<<user->GetUID()<<", room:="<<GetID();

	DCHECK_EQ(user->GetSeat(), Table::INVALID_SEAT);

	assistx2::Stream packet(Texas::SERVER_RESPONSE_LEAVE_ROOM);
	packet.Write(user->GetUID());
	packet.Write(err);
	packet.End();
	BroadCast(packet);

	DLOG(INFO) << "Leave mid:=" << user->GetUID();
	DCHECK(players_.find(user->GetUID()) != players_.end());
	players_.erase(user->GetUID());

	return 0;
}

boost::int32_t RoomBase::SitUp( PlayerInterface * user )
{
	Signal(RoomEventListener::PLAYER_SITUP_EVENT, user);
	
	return table_->Quit(user);
}

boost::int32_t RoomBase::GetNextPlayer( const boost::int32_t seat )
{
	Table::CycleIterator it = table_->find(seat);
	DCHECK(it != table_->end() );

	Table::CycleIterator end = table_->find(seat);

    Table::CycleIterator seat_it = std::find_if(++it, end, [](Seat * seat)->bool
    {
        return seat->ingame();
    });

	if (seat_it != end)
	{
		return seat_it->no_;
	}
	else
	{
		return Table::INVALID_SEAT;
	}
}

Seat * RoomBase::GetSeat( const boost::int32_t seat )
{
    DCHECK(seat >= 1 && seat <= 4);

	auto it = table_->find(seat);
	if (it != table_->end())
	{
		return *it;
	}

	return nullptr;
}

bool RoomBase::HasRealPlayer()
{
	for (auto seat = table_->begin(); seat != table_->end();++seat)
	{
		if (seat->user_ != nullptr && !PlayerInterface::IsRobot(seat->user_))
		{
			return true;
		}
	}

	return false;
}

boost::int32_t RoomBase::SitDown( PlayerInterface * user, boost::int32_t seat )
{
	Signal(RoomEventListener::PLAYER_SITDOWN_EVENT, user);

	return table_->Enter(user, seat);
}

void RoomBase::BroadCast(assistx2::Stream & packet, const std::vector<std::int32_t > & mids)
{
    DCHECK_GE(mids.size(), 1u);
    if (mids.size() < 2u)
    {
        packet.Insert(mids.front());
        packet.End();

        obj_->gatewayconnector()->SendTo(packet.GetNativeStream());
    }
    else 
    {
        assistx2::Stream stream(xProxy::CONVERGE_PACKET);
        stream.WriteBinary(packet.GetNativeStream().GetData(), packet.GetNativeStream().GetSize());

        stream.Write(static_cast<std::int16_t>(mids.size()));

        std::for_each(mids.begin(), mids.end(), [&stream](std::int32_t mid)
        {
            stream.Write(mid);
        });

        stream.End();

        obj_->gatewayconnector()->SendTo(stream.GetNativeStream());
    }
}

void RoomBase::BroadCast(assistx2::Stream & packet, PlayerInterface * exclude /*= nullptr*/)
{
    std::vector<std::int32_t > mids;
    for (auto it = players_.begin(); it != players_.end(); ++it)
    {
        if (it->second->GetConnectStatus() == false)
        {
            continue;
        }

        if (exclude == nullptr || it->second != exclude)
        {
            if (PlayerInterface::IsRobot(it->second) == true)
            {
                it->second->SendTo(packet);
            }
            else
            {
                mids.push_back(it->first);
            }
        }
    }

    if (mids.empty() == false)
    {
        BroadCast(packet, mids);
    }
}


boost::int32_t RoomBase::GetSeatPlayerCount() const
{
	return table_->GetPlayerCount();
}

boost::int32_t RoomBase::GetVisitorCount() const
{
	return players_.size() - table_->GetPlayerCount();
}

void RoomBase::update()const
{
	json_spirit::Array players;
	for (Players::const_iterator it = players_.begin(); it != players_.end(); ++it)
	{
		json_spirit::Object object;
		DLOG(INFO) << "update mid:=" << it->first;
		object.push_back(json_spirit::Pair("mid", static_cast<boost::int32_t>(it->first)));// ['mid'] = static_cast<boost::int32_t>(it->first);	
		object.push_back(json_spirit::Pair("seat", static_cast<boost::int32_t>(it->second->GetSeat() )));// = static_cast<boost::int32_t>(it->second->GetSeat());
		players.push_back( object );
	}

	DataLayer::getInstance()->UpdateRoom(GetID(), json_spirit::write_string(json_spirit::Value(players)));
}

void RoomBase::SendTo( PlayerInterface * player, const assistx2::Stream & stream )
{
	if (player->GetConnectStatus() == true)
	{
		if (PlayerInterface::IsRobot(player) == true)
		{
			player->SendTo(stream);
		}
		else
		{
			assistx2::Stream clone(stream);
			clone.Insert(player->GetUID() );

            obj_->gatewayconnector()->SendTo(clone.GetNativeStream() );
		}
	}
	else
	{
		DLOG(INFO)<<"RoomBase::SendTo FAILED, mid:="<<player->GetUID();
	}
}

void RoomBase::Signal( boost::int32_t event, PlayerInterface * trigger, EventContext * context /*= nullptr*/)
{
	for (std::vector<RoomEventListener * >::iterator it = observer_.begin() ; it != observer_.end(); ++it)
	{
		(*it)->OnEvent(event, trigger, this, context);
	}
}




