#include "private_room.h"
#include "poker_cmd.h"

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#include <assistx2/json_wrapper.h>
#include <json_spirit_writer_template.h>
#include <json_spirit_reader_template.h>
#include "player_interface.h"
#include "version.h"
#include "run_referee.h"
#include "card_generator.h"
#include "data_layer.h"
#include "room_listener.h"
#include "runfast_room_mgr.h"
#include "runfast_tracer.h"
#include "data_center.h"
#include "timer_helper.h"
#include "player_room_manager.h"
#include "config_mgr.h"
#include "datemanager.h"
#include "run4_referee_laizi.h"

extern bool g_server_stopped;
extern std::int32_t g_game_session;

const static std::int16_t CLIENT_QUERY_IF_SET_GPS = 1014;
const static std::int16_t SERVER_RESPONSE_IF_SET_GPS = 1014;
const static std::int16_t SERVER_BROADCAST_ALL_SET_GPS = 1015;
const static std::int16_t CLIENT_SET_GPS = 8001;

const static std::int16_t CLEINT_REQUEST_DISBAND_ROOM = 6001;
const static std::int16_t SERVER_BROADCAST_DISBAND_ROOM = 6001;
const static std::int16_t COMMAND_DISBAND_VOTE = 6002;

//道具转发
const static std::int16_t SERVER_BROADCAST_PROP = 1122;

//static const int32_t MAX_PLAYER = 3;
const static std::int32_t MAX_DIABAND_TIME = 3;

PrivateRoom::PrivateRoom(const boost::int32_t roomid, const runfastroomcfg & cfg) :
    GameRoom(roomid, roomcfg_type()),
    rfroomcfg_(cfg),
    winner_(nullptr),
    num_of_games_(cfg.ju),
    bet_(1),
    roommgr_(new RunFastRoomMgr)
{

}

PrivateRoom::~PrivateRoom()
{

}

void PrivateRoom::Init(KickCallbcak & call)
{
    kickcallback_ = call;
    card_generator_ = new CardGenerator(CardGenerator::POKER_CARD_GENERATOR);

    auto res = PlayerRoomManager::getInstance()->IsOwnerRoom(owner_, GetID());
    if (res == false) return;

    //ResetRoomData();
}

std::int32_t PrivateRoom::OnMessage(PlayerInterface * player, assistx2::Stream * packet)
{
    const short cmd = packet->GetCmd();

    DLOG(INFO) << "PrivateRoom::OnMessage() cmd:=" << cmd
        << "mid:= " << player->GetUID();

    switch (cmd)
    {
    case Texas::GATEWAY_EVENT_CONNECT_CLOSE:
        OnConnectClose(player);
        break;
    case Texas::CLIENT_REQUEST_SITUP:
        OnUnReady(player);
        break;
    case Texas::CLIENT_REQUEST_SITDOWN:
        OnReady(player);
        break;
    case RunFast::CLIEN_REQUEST_TUOGUAN:
        OnTuoGuan(player, packet);
        break;
    case RunFast::CLIENT_REQUEST_ZANLI:
        OnZanli(player);
        break;
    case RunFast::CLIENT_SEND_MESSAGE:
        OnBroadMessage(player, packet);
        break;
    case RunFast::CLINET_ADD_ROBOT:
        OnAddRobot(player);
        break;
    case CLEINT_REQUEST_DISBAND_ROOM:
        OnRequestDisbandRoom(player);
        break;
    case COMMAND_DISBAND_VOTE:
        OnVote(player, packet);
        break;
    case CLIENT_QUERY_IF_SET_GPS:
        OnQueryIfSetGps(player, packet);
        break;
    case CLIENT_SET_GPS:
        OnSetGps(player, packet);
        break;
    case RunFast::SERVER_NOTIFY_MSG_LIST:
    {
        auto begin = packet->Read<std::int32_t>();
        auto size = packet->Read<std::int32_t>();

        if (begin == -1)
        {
            begin = msg_list_.size() - 1;
        }

        SendMsgList(player, begin, size);
    }
	case SERVER_BROADCAST_PROP:
		OnBroadProp(packet);
		break;
    default:
        break;
    }

    return 0;
}

std::int32_t PrivateRoom::Enter(PlayerInterface * player)
{
    std::int32_t err = 0;
    bool isreconnect = true;

    player->SetConnectStatus(true);

    if (DataCenter::getInstance()->
        CheckRoomIsFull(player->GetUID(), GetID()) == true)
    {
        err = -1;
    }
    else if (players_.find(player->GetUID()) == players_.end())
    {
        isreconnect = false;
        err = OnEnterRoom(player);
    }

    assistx2::Stream result(Texas::SERVER_RESPONSE_ENTER_ROOM);
    result.Write(err);
    result.Write(XPDKPOKER_VERSION);
    result.Write(num_of_games_);
    result.Write(rfroomcfg_.ju);
    result.Write(GetID());
    result.Write(rfroomcfg_.type);
    result.Write(play_type_);
    result.Write(operation_);
    result.Write(table_->GetSeatCount());
    result.End();

    SendTo(player, result);

    if (err == 0)
    {
        auto seat = GetSeat(player->GetSeat());
        DCHECK(seat != nullptr);
        seat->is_zanli_ = false;

        stUserInfo userinfo;
        auto res = DataCenter::getInstance()->
            FindMyRoom(player->GetUID(), userinfo);
        if (res == true)
        {
            ResetWinner(winner_mid_);
            seat->room_score_ = userinfo.score;
            game_data_mgr_.ResetGameData(seat->no_, userinfo.game_data);
        }

        SendRoomSnapShot(player);

        SendTableSnapShot(player);
        if (!isreconnect)
        {
            BroadCastOnEnterPlayer(player);
            //UpdateRoomData();
        }
        else
        {
            OnComeBack(player);
        }

        OnReConnect(player);

        if (!isreconnect)
        {
            if (PlayerInterface::IsRobot(player) == false && isRobotMode_ == true)
            {
                Signal(RoomEventListener::PLAYER_ENTER_EVNET, nullptr);
            }
        }

        zanli_players_.erase(player->GetUID());
        DataCenter::getInstance()->RemoveUserInfo(player->GetUID());
    }

    return err;
}

std::int32_t PrivateRoom::Leave(PlayerInterface * user, std::int32_t err)
{
    DCHECK(players_.find(user->GetUID()) != players_.end()) << ", room:=" << GetID()
        << ", mid:=" << user->GetUID();

    assistx2::Stream packet(Texas::SERVER_RESPONSE_LEAVE_ROOM);
    packet.Write(user->GetUID());

    if (err != RunFast::SERVER_RESPONSE_DISBAND)
    {
        if (user->GetUID() == owner_ || num_of_games_ != rfroomcfg_.ju)
        {
            DLOG(ERROR) << "PrivateRoom::Leave() Failed: Player is owner or game is started!";
            return -2;
        }
    }
    else
    {
        err = 0;
    }

    const std::int32_t seat_no = user->GetSeat();
    if (seat_no != Table::INVALID_SEAT)
    {
        Seat * seat = GetSeat(seat_no);
        if (seat->ingame())
        {
            packet.Write(-1);
            packet.End();
            SendTo(user, packet);
            return -1;
        }
        else
        {
            seat->ready_ = false;
            seat->is_tuoguan_ = false;
            seat->is_zanli_ = false;
            table_->Quit(user);
        }
    }

    DCHECK_EQ(user->GetSeat(), Table::INVALID_SEAT);

    packet.Write(err);
    packet.End();

    BroadCast(packet);

    auto iter = players_is_set_gps_.find(user->GetUID());
    if (iter != players_is_set_gps_.end())
    {
        players_is_set_gps_.erase(iter);
    }

    players_.erase(user->GetUID());
    winner_ = nullptr;

    if (PlayerInterface::IsRobot(user) == false && isRobotMode_ == true)
    {
        Signal(RoomEventListener::PLAYER_LEAVE_EVNET, nullptr);
    }



    //UpdateRoomData();

    return 0;
}

std::int32_t PrivateRoom::Kick(const uid_type mid, bool add_to_blacklist)
{
    return 0;
}

void PrivateRoom::OnTimer(boost::shared_ptr<assistx2::timer2::TimerContext > context)
{
    EventTimerContext *  ptr = dynamic_cast<EventTimerContext *>(context.get());

    switch (ptr->type_)
    {
    case  RoomTimerContext::TIME_DISBAND_ROOM:
        OnDisbandRoom(DisbandType::TIME_OUT);
        break;
    default:
        break;
    }
}

std::int32_t PrivateRoom::OnEnterRoom(PlayerInterface * player)
{
    auto size = players_.size();

    DLOG(INFO)
        << "PrivateRoom::OnEnterRoom mid:="
        << player->GetUID()
        << " Now Players:=" << size;
    if (owner_ == 0)
    {
        DLOG(INFO) << "PrivateRoom::OnEnterRoom owner = 0 mid:="
            << player->GetUID();
        return  Texas::error_code::ERROR_ENTER_ROOM_NOT_FIND_ROOM;
    }

    if (static_cast<std::int32_t>(size) < table_->GetSeatCount())
    {
        players_.insert(std::make_pair(player->GetUID(), player));

        std::int32_t seatno = Table::INVALID_SEAT;
        for (Table::Iterator p = table_->begin(); p != table_->end(); ++p)
        {
            if (p->user_ == nullptr)
            {
                seatno = p->no_;
                break;
            }
        }
        DCHECK(seatno != Table::INVALID_SEAT);
        Seat * seat = GetSeat(seatno);
        seat->ready_ = false;
        seat->is_zanli_ = false;
        seat->is_tuoguan_ = false;
        seat->holecards_.clear();
        seat->playedCard_ = nullptr;
        return table_->Enter(player, seatno);
    }
    else
    {
        return Texas::error_code::ERROR_ENTER_ROOM_FILLED;
    }
}

void PrivateRoom::SendRoomSnapShot(PlayerInterface * player)
{
    assistx2::Stream packet(Texas::SERVER_PUSH_PLAYERS_SNAPSHOT);

    packet.Write(static_cast<boost::int32_t>(players_.size()));

    DLOG(INFO) << "RoomSnapShot mid:=" << player->GetUID() << ", " << betting_round_ << ", " << players_.size();

    for (Players::const_iterator it = players_.begin(); it != players_.end(); ++it)
    {
        const auto & baseinfo = it->second->getRoleInfo();

        packet.Write(it->second->GetSeat());
        packet.Write(baseinfo.mid());
        packet.Write(baseinfo.gp());
        packet.Write(baseinfo.sex());
        packet.Write(baseinfo.name());
        packet.Write(baseinfo.icon());

        const auto & game_base = it->second->GetGameBaseInfo();
        packet.Write(game_base.vip());
        packet.Write(game_base.gold());

        const auto & gameinfo = it->second->GetGameInfo();
        packet.Write(gameinfo.jifen());
        packet.Write(it->second->GetLoginAddr());

        DLOG(INFO) << "RoomSnapShot mid:=" << player->GetUID() << ", seatid:=" << it->second->GetSeat() << ", basemid:=" << baseinfo.mid()
            << ", gp:=" << baseinfo.gp() << ", sex:=" << baseinfo.sex() << ", name:=" << baseinfo.name() << ", icon:=" << baseinfo.icon() << ", vip:=" << game_base.vip()
            << ", gold:=" << game_base.gold();
    }
    packet.Write(static_cast<std::int32_t>(state_));
    for (Players::const_iterator it = players_.begin(); it != players_.end(); ++it)
    {
        packet.Write(it->second->GetSeat());
        packet.Write(it->second->GetGPS());
        DLOG(INFO) << "RoomSnapShot mid:=" << player->GetUID() << ", seatid:=" << it->second->GetSeat() << ", gps:=" << it->second->GetGPS();
    }
    packet.End();
    DLOG(INFO) << "RoomSnapShot mid:=" << player->GetUID() << "Room state:" << state_ <<
        ", packetsize:=" << packet.GetNativeStream().GetSize();
    SendTo(player, packet);
}

void PrivateRoom::BroadCastOnEnterPlayer(PlayerInterface * player)
{
    const auto & role = player->getRoleInfo();

    assistx2::Stream  result(Texas::SERVER_BROADCAST_PLAYER_INFO);
    result.Write(player->GetUID());
    result.Write(role.gp());
    result.Write(role.sex());
    result.Write(role.name());
    result.Write(role.icon());

    const auto & game_base = player->GetGameBaseInfo();

    result.Write(game_base.vip());
    result.Write(game_base.gold());
    result.Write(GetSeat(player->GetSeat())->room_score_);
    result.Write(player->GetSeat());
    result.Write(player->GetLoginAddr());
    result.Write(player->GetGPS());

    DLOG(INFO) << "BroadCastPlayerEnter seatid:" << player->GetSeat() << ",room:="
        << GetID() << ", mid:=" << player->GetUID() << ", gold:=" << game_base.gold();

    result.End();

    BroadCast(result, player);
}

void PrivateRoom::OnReConnect(PlayerInterface * user)
{
    const std::int32_t seat_no = user->GetSeat();

    DLOG_IF(INFO, seat_no != Table::INVALID_SEAT) << "GameRoom::OnReConnect, room:=" << GetID() << ", mid:=" << user->GetUID()
        << ", seat_no:=" << seat_no;

    if (seat_no != Table::INVALID_SEAT)
    {
        Seat * seat = GetSeat(seat_no);
        DCHECK(seat != nullptr);
        DCHECK(seat->user_ != nullptr);
        DCHECK(seat->user_ == user);

        seat->status_ = (seat->status_ & (~Seat::PLAYER_STATUS_NET_CLOSE));

        if (seat->holecards_.empty() == false)
        {
            assistx2::Stream result(RunFast::SERVER_NET_RECONNECT_HANDCARDS);
            result.Write(static_cast<std::int32_t>(seat->holecards_.size()));
            for (Cards::iterator it = seat->holecards_.begin(); it != seat->holecards_.end(); ++it)
            {
                result.Write((*it)->getName());
            }

            result.End();

            SendTo(user, result);
        }
    }
}

void PrivateRoom::OnConnectClose(PlayerInterface* player)
{
    DLOG(INFO) << "PrivateRoom::OnConnectClose mid:=" << player->GetUID();

    if (player->GetSeat() == Table::INVALID_SEAT)
    {
        return;
    }

    auto seat = GetSeat(player->GetSeat());
    if (state_ == PLAYING && seat->ingame() == true)
    {
        DLOG(INFO) << "OnConnectClose NET CLOSE mid:=" << player->GetUID() << ", " << seat->status_;
        seat->status_ |= Seat::PLAYER_STATUS_NET_CLOSE;

        seat->is_zanli_ = true;
        zanli_players_.insert(std::make_pair(player->GetUID(), player));

        assistx2::Stream stream(RunFast::SERVER_RESPONSE_ZANLI);
        stream.Write(seat->no_);
        stream.Write(0);
        stream.End();

        BroadCast(stream);

        player->SetConnectStatus(false);
    }
    else
    {
        DLOG(INFO) << "OnConnectClose Quit mid:=" << player->GetUID();
        OnZanli(player);
        player->SetConnectStatus(false);
        //kickcallback_(player, 0);
    }
}

void PrivateRoom::OnReady(PlayerInterface* player)
{
    DLOG(INFO) << "PrivateRoom::OnReady mid:=" << player->GetUID();

    if (player->GetSeat() == Table::INVALID_SEAT)
    {
        return;
    }

    auto seat = GetSeat(player->GetSeat());
    if (seat->ingame() || seat->ready_ == true || seat->is_zanli_ == true)
    {
        return;
    }
    auto err = 0;
    if (g_server_stopped == true)
    {
        assistx2::Stream stream(Texas::SERVER_PUSH_SERVERS_STOPPED);
        stream.Write(std::int32_t(0));
        stream.End();

        SendTo(player, stream);
        return;
    }
    else
    {
        seat->ready_ = true;
    }
    assistx2::Stream result(Texas::SERVER_RESPONSE_SITDOWN);
    result.Write(player->GetUID());
    result.Write(player->GetSeat());
    result.Write(err); //err code
    result.End();

    BroadCast(result);

    if (PlayerInterface::IsRobot(player) == false)
    {
        Signal(RoomEventListener::PLAYER_READY_EVNET, nullptr);
    }

    OnStartGame();
}

void PrivateRoom::OnUnReady(PlayerInterface* player)
{
    DLOG(INFO) << "PrivateRoom::OnUnReady mid:=" << player->GetUID();

    if (player->GetSeat() == Table::INVALID_SEAT)
    {
        return;
    }

    auto seat = GetSeat(player->GetSeat());
    seat->ready_ = false;
    seat->status_ = Seat::PLAYER_STATUS_WAITING;
    assistx2::Stream result(Texas::SERVER_RESPONSE_SITUP);
    result.Write(player->GetUID());
    result.Write(player->GetSeat());
    result.Write(0);  //err code
    result.End();

    BroadCast(result);
}

void PrivateRoom::OnStartGame()
{

}

void PrivateRoom::RemoveCards(Seat* seat, const Cards& cards)
{
    DLOG(INFO) << "Before Played HandCard seatno:=" << seat->no_ << " count:=" << seat->holecards_.size();
    for (auto iter : cards)
    {
        auto it = find(seat->holecards_.begin(), seat->holecards_.end(), iter);
        if (it != seat->holecards_.end())
        {
            seat->holecards_.erase(it);
        }
        else
        {
            DLOG(ERROR) << "PrivateRoom::RemoveCards():"
                << "The Card Face: = " << iter->getFace()
                << " Suit:= " << iter->getSuit()
                << "not in holecards";
        }
    }
    DLOG(INFO) << "---->seatno:=" << seat->no_ << "Played Card:=" << cards;
    DLOG(INFO) << "After Played HandCard seatno:=" << seat->no_ << " count:=" << seat->holecards_.size();
}

void PrivateRoom::OnGameOver(std::int32_t seatno)
{	
	for (Table::Iterator seat = GetTable()->begin(); seat != GetTable()->end(); ++seat)
	{
		auto paly_time = static_cast<std::int32_t>(time(nullptr) - seat->sitdown_time_);
		RunFastTracer::getInstance()->WriteUserDay(seat->user_->GetUID(), paly_time, 1);
	}
}

Seat* PrivateRoom::prevSeat(std::int32_t seatno)
{
    if (seatno > 1)
    {
        return GetSeat(seatno - 1);
    }
    else
    {
        return GetSeat(table_->GetSeatCount());
    }
}

Seat* PrivateRoom::nextSeat(std::int32_t seatno)
{
    if (seatno < table_->GetSeatCount())
    {
        return GetSeat(seatno + 1);
    }
    else
    {
        return GetSeat(1);
    }
}

int PrivateRoom::Disband(PlayerInterface * player)
{
    auto isCreateInFreeTime = ConfigMgr::getInstance()->IsInFreeTime(rfroomcfg_.type, create_time_);
    if (num_of_games_ == rfroomcfg_.ju && IsPlayed == false && isCreateInFreeTime == false)
    {
        ReturnGold();
    }
    else
    {
        DataManager::getInstance()->CreateRoomPlayer(this, isCreateInFreeTime);
        DataManager::getInstance()->PlayingPlayer(this);
		auto pay_master = owner_;
		if (proxy_mid_ != 0)
		{
			pay_master = proxy_mid_;
		}
		RunFastTracer::getInstance()->WriteGameIntegral(pay_master, rfroomcfg_.cost);
		RunFastTracer::getInstance()->IncrCreateRoomCount(owner_);

        /*for (Table::Iterator seat = GetTable()->begin(); seat != GetTable()->end(); ++seat)
		{
			auto paly_time = static_cast<std::int32_t>(time(nullptr) - seat->sitdown_time_);
			RunFastTracer::getInstance()->WriteUserDay(seat->user_->GetUID(), paly_time, 0, 1);
        }*/
    }

    assistx2::Stream stream(RunFast::SERVER_RESPONSE_DISBAND);
    stream.Write(0);
    stream.Write(type_);
    stream.End();
    BroadCast(stream);

    auto players = players_;
    for (auto iter : players)
    {
        RunFastTracer::getInstance()->addDisbandPlayers(iter.first, GetID());
        if (PlayerInterface::IsRobot(iter.second) == false)
        {
            kickcallback_(iter.second, RunFast::SERVER_RESPONSE_DISBAND);
            if (isRobotMode_ == true)
            {
                break;
            }
        }
    }
    CancelTimer(RoomTimerContext::TIME_DISBAND_ROOM);
    DataLayer::getInstance()->DeleteRoomData(owner_, GetID());
    PlayerRoomManager::getInstance()->DeleteRoom(owner_, GetID());

    zanli_players_.clear();
    roommgr_->RemovePrivateRoom(GetID());
    winner_ = nullptr;
    num_of_games_ = rfroomcfg_.ju;
    author_ = 0;
    create_time_ = time(nullptr);
    type_ = 0;
    play_type_ = 0;
    msg_list_.clear();
    room_state_ = 0;
    IsPlayed = false;
    bomb_info_.clear();
    state_ = EMPTY;

    DataCenter::getInstance()->RemoveFullRoom(GetID());
    DataLayer::getInstance()->RemoveRoomInfo(GetID());
    DataLayer::getInstance()->RemoveRoomRecordInfo(GetID());

    if (player == nullptr)
    {
        DLOG(INFO) << "RunFastGameMgr::Disband() Success"
            << "roomid:=" << GetID() << "num_of_games_ :=" << num_of_games_;
    }
    else
    {
        DLOG(INFO) << "RunFastGameMgr::Disband() Success:"
            << "roomid:=" << GetID() << " mid:=" << player->GetUID();
    }

    return 0;
}

void PrivateRoom::SendTableSnapShot(PlayerInterface * player)
{
    assistx2::Stream packet(RunFast::SERVER_BROADCAST_TABLE_SNAPSHOT);

    auto owner_obj = GetPlayer(owner_);
    if (owner_obj != nullptr)
    {
        packet.Write(owner_obj->GetSeat());
    }
    else
    {
        packet.Write(0);
    }
    packet.Write(table_->GetPlayerCount());

    for (Table::Iterator p = table_->begin(); p != table_->end(); ++p)
    {
        if (p->user_ != nullptr)
        {
            packet.Write(p->no_);
            packet.Write(p->user_->GetUID());
            packet.Write(p->room_score_);
            packet.Write(static_cast<std::int32_t>(p->ready_));
            packet.Write(static_cast<std::int32_t>(p->is_tuoguan_));
            packet.Write(static_cast<std::int32_t>(p->is_zanli_));
            packet.Write(static_cast<std::int32_t>(p->holecards_.size()));
            if (activeplayer_ == p->no_)
            {
                p->playedCard_ = nullptr;
            }
            if (p->playedCard_ == nullptr)
            {
                packet.Write(0);
				packet.Write(-1);
            }
            else
            {
                auto size = p->playedCard_->getCards().size();
                packet.Write(static_cast<std::int32_t>(size));
                for (auto iter : p->playedCard_->getCards())
                {
					auto name = getName(iter);
					packet.Write(name);					
                }
				packet.Write(static_cast<std::int32_t>(p->playedCard_->getType()));
            }
            DLOG(INFO) << "PrivateRoom::SendTableSnapShot(): mid:=" << p->user_->GetUID()
                << " p->ready_:=" << p->ready_ << " p->is_tuoguan_:=" << p->is_tuoguan_ << " p->is_zanli_:="
                << p->is_zanli_ << "p->room_score_:" << p->room_score_;
        }
    }
    packet.Write(activeplayer_);
    if (activeplayer_ != Table::INVALID_SEAT)
    {
        auto time_now = static_cast<std::int32_t>(time(nullptr) - GetSeat(activeplayer_)->start_time_);
        auto value = rfroomcfg_.bettime - time_now;
        DLOG(INFO) << "PrivateRoom::SendTableSnapShot(): time:=" << value;
        if (value > 0)
        {
            packet.Write(static_cast<std::int32_t>(value));
        }
        else
        {
            packet.Write(static_cast<std::int32_t>(0));
        }

    }
    else
    {
        packet.Write(static_cast<std::int32_t>(0));
    }
    packet.Write(0);
    packet.End();

    SendTo(player, packet);
}

void PrivateRoom::OnRoomAccount(bool isreturn)
{
    for (Table::Iterator seat = GetTable()->begin(); seat != GetTable()->end(); ++seat)
    {
        if(seat->user_ != nullptr)
            RunFastTracer::getInstance()->UpdateUserDay(seat->user_->GetUID(), 1);
    }
}

void PrivateRoom::OnZanli(PlayerInterface * player)
{
    if (player->GetSeat() == Table::INVALID_SEAT)
    {
        return;
    }

    auto seat = GetSeat(player->GetSeat());
    auto err = 0;
    if (state_ == PLAYING && seat->ingame())
    {
        err = -1;
    }
    else
    {
        err = 0;
        seat->is_zanli_ = true;
        seat->ready_ = false;
        seat->is_tuoguan_ = false;
        zanli_players_.insert(std::make_pair(player->GetUID(), player));
    }
    assistx2::Stream stream(RunFast::SERVER_RESPONSE_ZANLI);
    stream.Write(seat->no_);
    stream.Write(err);
    stream.End();

    BroadCast(stream);

    if (err == 0)
    {
        DLOG(INFO) << "PrivateRoom::OnZanli(): mid:=" << player->GetUID();
    }
}

void PrivateRoom::OnTuoGuan(PlayerInterface * player, assistx2::Stream * packet)
{
    if (player->GetSeat() == Table::INVALID_SEAT)
    {
        return;
    }

    auto isTG = packet->Read<std::int32_t>();

    auto seatno = player->GetSeat();
    auto seat = GetSeat(seatno);

    assistx2::Stream stream(RunFast::SERVER_RESPONSE_TUOGUAN);
    stream.Write(seatno);
    stream.Write(isTG);
    stream.End();

    BroadCast(stream);

    if (!seat->ingame())
    {
        return;
    }
    if (isTG == 1)
    {
        seat->is_tuoguan_ = true;
        if (seatno == activeplayer_)
        {
            NewTimer(2000, RoomTimerContext::TIME_OUT_PLAY, nullptr);
        }
    }
    else
    {
        seat->is_tuoguan_ = false;
        if (seatno == activeplayer_)
        {
            NewTimer(rfroomcfg_.bettime * 1000, RoomTimerContext::TIME_OUT_PLAY, nullptr);
        }
    }

}


void PrivateRoom::OnBroadMessage(PlayerInterface * player, assistx2::Stream * packet)
{
    if (player->GetSeat() == Table::INVALID_SEAT)
    {
        return;
    }

    auto message = packet->Read<std::string>();
    auto name = player->getRoleInfo().name();

    msg_list_.push_back(std::make_pair(name, message));

    assistx2::Stream stream(RunFast::SERVER_BROADCAST_MESSAGE);
    stream.Write(player->GetSeat());
    stream.Write(message);
    stream.Write(static_cast<std::int32_t>(msg_list_.size()));
    stream.Write(name);
    stream.End();

    BroadCast(stream);
}

void PrivateRoom::OnBroadProp(assistx2::Stream* packet)
{
	auto send_seat = packet->Read<std::int32_t>();
	auto rev_seat = packet->Read<std::int32_t>();
	auto data = packet->Read<std::int32_t>();
	DLOG(INFO) << "send_seat = " << send_seat << " rev_seat = " << rev_seat << " data = " << data;

	assistx2::Stream stream(SERVER_BROADCAST_PROP);
	stream.Write(send_seat);
	stream.Write(rev_seat);
	stream.Write(data);
	stream.End();
	
	BroadCast(stream);
}

void PrivateRoom::OnComeBack(PlayerInterface * player)
{
    if (player->GetSeat() == Table::INVALID_SEAT)
    {
        return;
    }
    auto seat = GetSeat(player->GetSeat());

    assistx2::Stream stream(RunFast::SERVER_RESPONSE_ZANLI);
    stream.Write(seat->no_);
    stream.Write(1);
    stream.End();

    BroadCast(stream);

    if (static_cast<std::int32_t>(players_is_set_gps_.size()) == table_->GetSeatCount())
    {
        assistx2::Stream stream(SERVER_BROADCAST_ALL_SET_GPS);
        stream.End();

        BroadCast(stream);
    }

    if (disband_author_ != Table::INVALID_SEAT)
    {
        SendVoteMessage(player, -1);
    }
}

void PrivateRoom::OnAddRobot(PlayerInterface * player)
{
    Signal(RoomEventListener::PLAYER_ENTER_EVNET, nullptr);
}

void PrivateRoom::ResetWinner(std::int32_t mid)
{
    DLOG(INFO) << "PrivateRoom::ResetWinner" << mid;
    auto iter = players_.find(mid);
    if (iter != players_.end())
    {
        winner_ = iter->second;
    }
}

void PrivateRoom::SendMsgList(PlayerInterface * player, std::int32_t begin, std::int32_t size)
{
    auto max_size = static_cast<std::int32_t>(msg_list_.size());
    assistx2::Stream stream(RunFast::SERVER_NOTIFY_MSG_LIST);
    if (size > max_size)
    {
        stream.Write(max_size);
    }
    else
    {
        stream.Write(static_cast<std::int32_t>(size));
    }
    for (std::int32_t i = begin; i >= (begin - size); --i)
    {
        if (i >= static_cast<std::int32_t>(msg_list_.size()) ||
            i < 0)
        {
            continue;
        }
        stream.Write(i);
        stream.Write(msg_list_[i].first);
        stream.Write(msg_list_[i].second);
    }
    stream.End();

    SendTo(player, stream);
}

void PrivateRoom::EnableDisbandTimer()
{
    DLOG(INFO) << "EnterTime roomid:=" << GetID();
    NewTimer(60 * 60 * 1000, RoomTimerContext::TIME_DISBAND_ROOM);
}

void PrivateRoom::ReturnGold()
{
    auto pay_master = owner_;
    if (proxy_mid_ != 0)
    {
        pay_master = proxy_mid_;
    }

    chips_type amount = 0;
    chips_type real_pay = 0;
    const int err = DataLayer::getInstance()->Pay(pay_master, -rfroomcfg_.cost, amount, real_pay, false);
    if (err == 0)
    {
        LOG(INFO) << "GoldPay " << ", mid:=" << pay_master << ",delta:" << -rfroomcfg_.cost << ",amount:" << amount;
        assistx2::Stream stream(RunFast::SERVER__UPDATE_GOLD);
        stream.Write(pay_master);
        stream.Write(amount);
        stream.End();
        gatewayconnector_->SendTo(stream.GetNativeStream());
        if (proxy_mid_ != 0)
        {
            RunFastTracer::getInstance()->WriteGoldLog(pay_master, rfroomcfg_.cost, amount, 5, owner_);
        }
        else
        {
            RunFastTracer::getInstance()->WriteGoldLog(pay_master, rfroomcfg_.cost, amount, 5);
        }
    }
    else
    {
        LOG(ERROR) << "Player::GoldPay FALIED, err:=" << err << ", mid:=" << pay_master << ", gold:=" << -rfroomcfg_.cost;
    }
}

void PrivateRoom::OnQueryIfSetGps(PlayerInterface * player, assistx2::Stream* packet)
{
    DCHECK(player != nullptr);
    std::int32_t is_set_gps = 0;
    auto iter = players_is_set_gps_.find(player->GetUID());
    if (iter != players_is_set_gps_.end())
    {
        is_set_gps = 1;
    }

    assistx2::Stream package(SERVER_RESPONSE_IF_SET_GPS);
    package.Write(is_set_gps);
    package.End();

    SendTo(player, package);
}

void PrivateRoom::OnSetGps(PlayerInterface * player, assistx2::Stream* packet)
{
    DCHECK(player != nullptr);
    auto str = packet->Read<std::string>();
    if (!str.empty())
    {
        player->SetGPS(str);
        DataLayer::getInstance()->set_player_gps(player->GetUID(), str);
    }
    else
    {
        auto gps = DataLayer::getInstance()->player_gps(player->GetUID());
        player->SetGPS(str);
    }

    players_is_set_gps_.insert(player->GetUID());

    assistx2::Stream package(CLIENT_SET_GPS);
    package.Write(player->GetSeat());
    package.Write(player->GetGPS());
    package.End();

    BroadCast(package);

    if (static_cast<std::int32_t>(players_is_set_gps_.size()) == table_->GetSeatCount())
    {
        assistx2::Stream stream(SERVER_BROADCAST_ALL_SET_GPS);
        stream.End();

        BroadCast(stream);
    }
}

Seat* PrivateRoom::GetPrevPlayedSeat(std::int32_t seatno)
{
    for (auto seat = prevSeat(seatno); seat->no_ != seatno; )
    {
        if (seat->playedCard_ != nullptr)
        {
            return seat;
        }
        seat = prevSeat(seat->no_);
    }

    return nullptr;
}

void PrivateRoom::OnRequestDisbandRoom(PlayerInterface * player)
{
    auto now_players = table_->GetPlayerCount();
    auto max_seats = table_->GetSeatCount();

    if (time(nullptr) - create_time_ <= MAX_DIABAND_TIME)
    {
        return;
    }

    if (now_players < static_cast<std::int32_t>(max_seats))
    {
        OnDisbandRoom(DisbandType::ALL_AGREE);
        return;
    }

    auto err = 0;
    if (disband_author_ == Table::INVALID_SEAT)
    {
        disband_author_ = player->GetSeat();
        disband_start_time_ = time(nullptr);

#ifdef TEST_TIME
        NewTimer(10 * 1000, RoomTimerContext::TIME_DISBAND_ROOM);
#else
        NewTimer(5 * 60 * 1000, RoomTimerContext::TIME_DISBAND_ROOM);
#endif
    }
    else
    {
        err = -1003;
    }

    SendVoteMessage(player, err);
}

void PrivateRoom::OnVote(PlayerInterface * player, assistx2::Stream* packet)
{
    auto vote = packet->Read<std::int32_t>();

    auto seatno = player->GetSeat();
    DCHECK(seatno != Table::INVALID_SEAT);
    auto seat = GetSeat(seatno);
    DCHECK(seat != nullptr);

    auto err = 0;
    if (disband_author_ == Table::INVALID_SEAT)
    {
        err = -1;
    }

    if (seat->disband_vote_ != 0)
    {
        err = -2;
    }

    assistx2::Stream stream(COMMAND_DISBAND_VOTE);
    stream.Write(err);
    stream.End();
    SendTo(player, stream);

    if (err != 0)
    {
        return;
    }

    if (vote == 1)
    {
        seat->disband_vote_ = vote;
    }
    else
    {
        seat->disband_vote_ = 2;
    }

    SendVoteMessage(player, 0);

    auto vote_count_true = 0;
    for (auto iter = table_->begin(); iter != table_->end(); ++iter)
    {
        if (iter->disband_vote_ == 2)
        {
            ClearVoteData();
            return;
        }
        if (iter->disband_vote_ == 1)
        {
            vote_count_true += 1;
        }
    }

    if (vote_count_true >= static_cast<std::int32_t>(table_->GetSeatCount() - 2))
    {
        ClearVoteData();
        OnDisbandRoom(DisbandType::ALL_AGREE);
    }
}

void PrivateRoom::SendVoteMessage(PlayerInterface * player, std::int32_t err)
{
    auto now_players = table_->GetPlayerCount();
#ifdef TEST_TIME
    auto over_time = 10 - (time(nullptr) - disband_start_time_);
#else
    auto over_time = 300 - (time(nullptr) - disband_start_time_);
#endif // TEST_TIME


    assistx2::Stream stream(CLEINT_REQUEST_DISBAND_ROOM);
    stream.Write(err);
    stream.Write(disband_author_);
    stream.Write(static_cast<std::int32_t>(over_time));
    stream.Write(now_players - 1);

    for (auto iter = table_->begin(); iter != table_->end(); ++iter)
    {
        if (iter->user_ == nullptr)
        {
            continue;
        }
        if (iter->no_
            == disband_author_)
        {
            continue;
        }

        stream.Write(iter->no_);
        stream.Write(iter->disband_vote_);
    }
    stream.End();

    if (err == 0)
    {
        BroadCast(stream);
    }
    else
    {
        SendTo(player, stream);
    }
}

void PrivateRoom::ClearVoteData()
{
    CancelTimer(RoomTimerContext::TIME_DISBAND_ROOM);
    disband_author_ = Table::INVALID_SEAT;
    for (auto iter = table_->begin(); iter != table_->end(); ++iter)
    {
        iter->disband_vote_ = 0;
    }
}

void PrivateRoom::OnDisbandRoom(DisbandType type)
{
    bool isreturn = false;
    auto isCreateInFreeTime = ConfigMgr::getInstance()->IsInFreeTime(rfroomcfg_.type, create_time_);
    if (num_of_games_ == rfroomcfg_.ju && IsPlayed == false && isCreateInFreeTime == false)
    {
        isreturn = true;
        ReturnGold();
        type = DisbandType::NOT_START;
    }
    else
    {
        DataManager::getInstance()->CreateRoomPlayer(this, isCreateInFreeTime);
        DataManager::getInstance()->PlayingPlayer(this);
		auto pay_master = owner_;
		if (proxy_mid_ != 0)
		{
			pay_master = proxy_mid_;
		}
		RunFastTracer::getInstance()->WriteGameIntegral(pay_master, rfroomcfg_.cost);
        RunFastTracer::getInstance()->IncrCreateRoomCount(owner_);
    }

//     if (state_ == RoomInterface::PLAYING)
//     {
//         OnGameAccount();
//     }

    if (table_->GetSeatCount() ==
        table_->GetPlayerCount())
    {
        OnRoomAccount(isreturn);
    }

    assistx2::Stream stream(RunFast::SERVER_RESPONSE_DISBAND);
    stream.Write(0);
    stream.Write(static_cast<std::int32_t>(type));
    stream.End();
    BroadCast(stream);

    auto players = players_;
    for (auto iter : players)
    {
        RunFastTracer::getInstance()->addDisbandPlayers(iter.first, GetID());
        if (PlayerInterface::IsRobot(iter.second) == false)
        {
            kickcallback_(iter.second, RunFast::SERVER_RESPONSE_DISBAND);
        }
    }
    CancelTimer(RoomTimerContext::TIME_DISBAND_ROOM);
    DataLayer::getInstance()->DeleteRoomData(owner_, GetID());
    PlayerRoomManager::getInstance()->DeleteRoom(owner_, GetID());

    roommgr_->RemovePrivateRoom(GetID());

    ClearRoomData();
    DataCenter::getInstance()->RemoveFullRoom(GetID());
    DataLayer::getInstance()->RemoveRoomInfo(GetID());
    DataLayer::getInstance()->RemoveRoomRecordInfo(GetID());
}

void PrivateRoom::ClearGameData()
{
    state_ = WAITING;
    activeplayer_ = Table::INVALID_SEAT;

    for (Table::Iterator seat = table_->begin(); seat != table_->end(); ++seat)
    {
        if (seat->user_ == nullptr)
        {
            continue;
        }
        seat->ready_ = false;
        seat->is_tuoguan_ = false;
        seat->is_zanli_ = false;
        seat->status_ = ((seat->status_ & ~Seat::PLAYER_STATUS_PLAYING) | Seat::PLAYER_STATUS_WAITING);
        seat->playedCard_ = nullptr;
        seat->holecards_.clear();
        seat->bomb_ = 0;
        seat->score_ = 0;
        seat->bomb_score_ = 0;
        DataLayer::getInstance()->RemovePlayerStatus(seat->user_->GetUID());
        RunFastTracer::getInstance()->UpdatePlayingPlayer(seat->user_->GetLoginSource(), seat->user_->GetUID(), false);
    }
    bomb_info_.clear();
    ClearVoteData();
}

void PrivateRoom::ClearRoomData()
{
    ClearGameData();
    winner_ = nullptr;
    num_of_games_ = rfroomcfg_.ju;
    disband_start_time_ = 0;
    create_time_ = time(nullptr);
    disband_author_ = 0;
    msg_list_.clear();
    bomb_info_.clear();
    game_data_mgr_.ClearAll();
    zanli_players_.clear();
    author_ = 0;
    type_ = 0;
    play_type_ = 0;
    room_state_ = 0;
    IsPlayed = false;
    state_ = EMPTY;
}

std::string PrivateRoom::getName(std::shared_ptr<CardInterface> card)
{ 
    return card->getName();
}

void PrivateRoom::OnPiaoScore()
{
    assistx2::Stream stream(RunFast::SERVER_BROADCAST_ACCOUNTS);
    stream.Write(table_->GetPlayerCount());
    for (Table::Iterator p = table_->begin(); p != table_->end(); ++p)
    {
        p->room_score_ += p->score_ + p->bomb_score_;
        stream.Write(p->no_);
        stream.Write(p->score_);
        stream.Write(p->room_score_);
    }
    stream.End();

    BroadCast(stream);
}
