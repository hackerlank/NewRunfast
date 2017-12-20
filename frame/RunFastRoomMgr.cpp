#include "RunFastRoomMgr.h"

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

#include "ConfigMgr.h"
#include "GameRoom.h"
#include "DataLayer.h"
#include "playerinterface.h"
#include "ProxyCmd.h"
#include "PokerCmd.h"
#include "RoomTimer.h"
#include "RunFastRoom.h"
#include "RunFast4Room.h"
#include "RoomListener.h"
#include "DataLayer.h"
#include "DataCenter.h"

#include "RunFast4RoomLaizi.h"
#include "game_obj.h"

extern boost::int32_t g_server_id;

RunFastRoomMgr::RunFastRoomMgr()
    :guard_(GameObj::GetInstance()), connect_(guard_->gatewayconnector())
{   

}

RunFastRoomMgr::~RunFastRoomMgr(void)
{
    for (std::map<boost::int32_t, RoomInterface * >::iterator it = rooms_.begin(); it != rooms_.end(); ++it)
    {
        delete it->second;
    }
    rooms_.clear();
    roomgroups_.clear();
    private_rooms_.clear();
}

int32_t RunFastRoomMgr::OnMessage(boost::shared_ptr<assistx2::NativeStream> packet, PlayerInterface* player)
{
    DLOG(INFO) << "RunFastGameMgr::OnMessage()->cmd:" << cmd;

    switch (cmd)
    {
    case Texas::CLEINT_REQUEST_CREATE_ROOM:
        OnCreateRoom(player, &packet);
        return 0;
    case Texas::CLIENT_REQUEST_ENTER_ROOM:
        OnEnterRoom(player, &packet);
        break;
    case Texas::CLIENT_REQUEST_LEAVE_ROOM:
        OnLeaveRoom(player, 0);
        break;
    case RunFast::CLIENT_FIND_Z_ROOM:
        OnFindRoom(player);
        break;
    case RunFast::CLIENT_CHANGE_Z_ROOM:
        OnChangeRoom(player);
        break;
    case RunFast::CLIENT_REQUEST_DISBAND:
        //OnDisbandRoom(player, &packet);
        break;
    default:
        break;
    }
    return 0;
}

std::int32_t RunFastRoomMgr::Initialize(std::function<std::int32_t(PlayerInterface *, std::int32_t) > kick_cb,
    assistx2::TcpHanlderWrapper * connector)
{    
    DataCenter::getInstance()->init(this);

    rfroomscfg_ = ConfigMgr::getInstance()->getRunFastRoomCfg();

    for (auto it = rfroomscfg_.begin(); it != rfroomscfg_.end(); ++it)
    {
        for (std::int32_t i = it->begin; i <= it->end; ++i)
        {
            RoomInterface * room = nullptr;
            if (it->type[0] == 'A')
            {
                room = new RunFastRoom(i, *it, 16);
            }
            else if (it->type[0] == 'B')
            {
                room = new RunFastRoom(i, *it, 15);
            }
            else if(it->type[0] == 'D')
            {
                room = new RunFast4Room(i, *it);
            }
			else if (it->type[0] == 'C')
			{
                room = new RunFast4RoomLaizi(i, *it);
			}
            else
            {
                continue;
            }
            auto owner = 0;
            auto res = DataCenter::getInstance()->ReSetRoom(room, owner);
            if (res == true && owner != 0)
            {
                AddRrivateRoom(owner, room);
            }
            
            rooms_.insert(std::make_pair(i, room));

            for (auto it : room_listeners_)
            {
                room->RegisterRoomEventObserver(it);
            }

            room->Init(kick_cb);
            auto iter = roomgroups_.find(it->type);
            if (iter != roomgroups_.end())
            {
                iter->second.push_back(room);
            }
            else
            {
                roomgroups_.emplace(it->type, std::vector<RoomInterface* >(1, room));
            }
        }
    }

    return 0;
}

RoomInterface * RunFastRoomMgr::GetRoomObject(std::int32_t room)
{
    auto it = rooms_.find(room);
    if (it == rooms_.end())
    {
        return nullptr;
    }
    else
    {
        return it->second;
    }
}

RoomInterface * RunFastRoomMgr::GetPrivateRoomObject(std::int32_t room)
{
    auto it = private_rooms_.find(room);
    if (it == private_rooms_.end())
    {
        DLOG(INFO) << "RunFastGameMgr::GetPrivateRoomObject() nullptr";
        return nullptr;
    }
    else
    {
        DLOG(INFO) << "RunFastGameMgr::GetPrivateRoomObject() roomid=" << room;
        return it->second;
    }
}

std::int32_t RunFastRoomMgr::CreateRoomByType(int32_t mid, const std::string& type, int32_t& cost)
{
    auto iter = roomgroups_.find(type);
    if (iter == roomgroups_.end())
    {
        return 0;
    }
    std::random_shuffle(iter->second.begin(), iter->second.end());

    for (auto room : iter->second)
    {
        if (room->GetState() == RoomInterface::EMPTY)
        {
            std::string value;
            if (DataLayer::getInstance()->GetRoomInfo(room->GetID(), value) != false)
            {
                continue;
            }
            AddRrivateRoom(mid,room);
            auto roomcfg = room->getRunFastRoomCfg();
            cost = roomcfg.cost;
            return room->GetID();
        }
    }
    return 0;
}

RoomInterface* RunFastRoomMgr::GetMatchRoom()
{
   if (match_rooms_.empty())
   {
       return nullptr;
   }
   auto room = match_rooms_.front();
   match_rooms_.pop();

   return room;
}

std::vector<RoomInterface * > RunFastRoomMgr::GetRoomsByType(std::int32_t type_id)
{
    return std::vector<RoomInterface * >();
}

void RunFastRoomMgr::RemovePrivateRoom(std::int32_t room)
{
    auto it = private_rooms_.find(room);
    if (it != private_rooms_.end())
    {
        it->second->SetState(RoomInterface::EMPTY);
        it->second->SetOwner(0);

        private_rooms_.erase(it);
        DLOG(INFO) << "RunFastGameMgr::RemovePrivateRoom() Success"
            << "roomid:=" << room;
    }
}

void RunFastRoomMgr::AddRrivateRoom(std::int32_t owner, RoomInterface *room)
{
    auto iter = private_rooms_.find(room->GetID());
    if (iter == private_rooms_.end())
    {
        room->SetState(RoomInterface::WAITING);
        room->SetOwner(owner);
        private_rooms_.insert(std::make_pair(room->GetID(), room));

        DLOG(INFO) << "RunFastGameMgr::AddRrivateRoom() Success"
            << "roomid:=" << room->GetID();
    }
}

void RunFastRoomMgr::AddRoomListener(RoomEventListener * listener)
{
    //必须在Initialize前调用
    DCHECK(guard_->gatewayconnector() == nullptr);
    room_listeners_.push_back(listener);
}

std::vector<RoomInterface * > RunFastRoomMgr::GetWatingGlodRoomByType(const std::string& type)
{
    auto iter = waiting_glod_room_groups_.find(type);
    if (iter != waiting_glod_room_groups_.end())
    {
        return iter->second;
    }

    return std::vector<RoomInterface * >();
}

RoomInterface * RunFastRoomMgr::GetGoldRoomByType(const std::string& type)
{
    auto waiting_room = GetWatingGlodRoomByType(type);
    if (waiting_room.size() != 0)
    {
        for (auto iter : waiting_room)
        {
            if (iter->GetSeatPlayerCount() < 3)
            {
                return iter;
            }
        }
    }
    
     return GetRoomByType(type);
}

void RunFastRoomMgr::AttachWatingGoldRoom(const std::string& type, RoomInterface * room)
{
    DLOG(INFO) << "RunFastGameMgr::AttachWatingGoldRoom() type:=" << type
        << " room:=" << room->GetID();

    auto iter = waiting_glod_room_groups_.find(type);
    if (iter != waiting_glod_room_groups_.end())
    {
        auto it = std::find_if(iter->second.begin(), iter->second.end(), [room](const RoomInterface * value) {
            return room->GetID() == value->GetID();
        });
        if (it == iter->second.end())
        {
            DLOG(INFO) << "RunFastGameMgr::AttachWatingGoldRoom() push_back type:=" << type
                << " room:=" << room->GetID();
            iter->second.push_back(room);
        }
    }
    else
    {
        DLOG(INFO) << "RunFastGameMgr::AttachWatingGoldRoom() insert type:=" << type
            << " room:=" << room->GetID();
        waiting_glod_room_groups_.insert(std::make_pair(type, std::vector<RoomInterface * >(1, room)));
    }
}

void RunFastRoomMgr::DetachWatingGoldRoom(const std::string& type, RoomInterface * room)
{
    DLOG(INFO) << "RunFastGameMgr::DetachWatingGoldRoom() type:=" << type
        << " room:=" << room->GetID();

    auto iter = waiting_glod_room_groups_.find(type);
    if (iter != waiting_glod_room_groups_.end())
    {
        auto it = std::find_if(iter->second.begin(), iter->second.end(), [room](const RoomInterface * value) {
            return room->GetID() == value->GetID();
        });
        if (it != iter->second.end())
        {
            DLOG(INFO) << "RunFastGameMgr::DetachWatingGoldRoom() erase type:=" << type
                << " room:=" << room->GetID();
            iter->second.erase(it);
        }
    }
}

RoomInterface * RunFastRoomMgr::GetRoomByType(const std::string& type)
{
    auto iter = roomgroups_.find(type);
    if (iter == roomgroups_.end())
    {
        return nullptr;
    }
  
    for (auto room : iter->second)
    {
        if (room->GetState() == RoomInterface::EMPTY)
        {
            return room;
        }
    }

    return nullptr;
}

std::map<std::string, std::vector<RoomInterface * > >  RunFastRoomMgr::GetWatingGlodRoom()
{
    return waiting_glod_room_groups_;
}

void RunFastRoomMgr::OnChangeRoom(PlayerInterface *player)
{

    auto room = player->GetRoomObject();
    if (room != nullptr)
    {
        if (room->GetState() == RoomInterface::PLAYING)
        {
            assistx2::Stream result(RunFast::SERVER_CHANGE_Z_ROOM);
            result.Write(player->GetUID());
            result.Write(RunFast::ErrorCode::ERROR_Z_ROOM_PLAYING);
            result.End();

            guard_->gatewayconnector()->SendTo(result.GetNativeStream());

            return;
        }
        if (g_server_stopped == true)
        {
            assistx2::Stream result(Texas::SERVER_PUSH_SERVERS_STOPPED);
            result.Write(player->GetUID());
            result.Write(std::int32_t(0));
            result.End();

            guard_->gatewayconnector()->SendTo(result.GetNativeStream());

            DLOG(INFO) << "RunFastGameMgr::OnChangeRoom() g_server_stopped is true";
            return;
        }
       auto room_cfg = room->getRunFastRoomCfg();
       auto target_room = roommgr_->GetGoldRoomByType(room_cfg.type);
       if (target_room != nullptr)
       {
           OnLeaveRoom(player,0);

           GlobalTimerProxy::getInstance()->NewTimer(std::bind(&RunFastGameMgr::EnterNextRoom, this,player,
               target_room->GetID(), room_cfg.type), 1);
       }
       else
       {
           DLOG(ERROR) << "RunFastGameMgr::OnChangeRoom() No Much Room !";
       }
    }
    else
    {
        DLOG(ERROR) << "RunFastGameMgr::OnChangeRoom() Not In Room !";
    }
}

void RunFastRoomMgr::OnFindRoom(PlayerInterface *player)
{
    if (g_server_stopped == true)
    {
        assistx2::Stream result(Texas::SERVER_PUSH_SERVERS_STOPPED);
        result.Write(player->GetUID());
        result.Write(std::int32_t(0));
        result.End();

        guard_->gatewayconnector()->SendTo(result.GetNativeStream());

        DLOG(INFO) << "RunFastGameMgr::OnFindRoom() g_server_stopped is true";
        return;
    }

    auto roomcfgs = ConfigMgr::getInstance()->getRunFastRoomCfg();
    auto glod = player->GetGameBaseInfo().gold();
    std::string room_type;
    for (auto iter : roomcfgs)
    {
        if (iter.type[0] != 'Z')
        {
            continue;
        }
        if (glod >= iter.min &&
             glod <= iter.max)
        {
            room_type = iter.type;
        }
    }

    if (room_type.empty())
    {
        assistx2::Stream result(RunFast::CLIENT_FIND_Z_ROOM);
        result.Write(player->GetUID());
        result.Write(RunFast::ErrorCode::ERROR_GLOD_ENTER_LIMIT);
        result.End();

        guard_->gatewayconnector()->SendTo(result.GetNativeStream());

        DLOG(INFO) << "RunFastGameMgr::OnFindRoom() glod limit";
        return;
    }

    assistx2::Stream stream(Texas::CLIENT_REQUEST_ENTER_ROOM);
    stream.Write(-1);
    stream.Write(room_type);
    stream.End();
    OnEnterRoom(player, &stream);
}

void RunFastRoomMgr::OnCreateRoom(PlayerInterface *player, assistx2::Stream *packet)
{
    int32_t cost = 0;
    int32_t err = 0;
    auto isPayByOther = false;
    const std::int32_t mid = player->GetUID();
    const std::string type = packet->Read<std::string>();
    auto playe_type = packet->Read<std::int32_t>();
    const auto operation = packet->Read<std::int32_t>();
    const auto players = packet->Read<std::int32_t>();
    const auto isNotEnter = packet->Read<std::int32_t>();
    auto isfreetime = ConfigMgr::getInstance()->IsFreeTime(type);

    if (isNotEnter == 1)
    {
        isPayByOther = true;
    }

    if (g_server_stopped == true)
    {
        assistx2::Stream result(Texas::SERVER_PUSH_SERVERS_STOPPED);
        result.Write(player->GetUID());
        result.Write(std::int32_t(0));
        result.End();

        connect_->SendTo(result.GetNativeStream());

        DLOG(INFO) << "RunFastGameMgr::OnCreateRoom() g_server_stopped is true";
        return;
    }

    LOG(INFO) << " RunFastGameMgr::OnCreateRoom type:=" << type << ",playe_type:=" << playe_type
        << ",operation:=" << operation << ",players:=" << players  << ",isNotEnter:=" << isNotEnter << ",mid:=" << mid
        << ",isfreetime:=" << isfreetime;

    std::int32_t proxy_mid = 0;
    std::int32_t room_id = 0;
    auto room = player->GetRoomObject();
    if (room == nullptr)
    {
        room_id = CreateRoomByType(mid, type, cost);
        if (room_id != 0)
        {
            if (isfreetime == false)
            {
                auto pay = PlayCost(player, cost, isPayByOther, proxy_mid);
                if (pay != 0)
                {
                    RemovePrivateRoom(room_id);
                    err = pay;
                }
            }
        }
        else
        {
            err = -1; //创建失败
        }
    }
    else
    {
        room_id = room->GetID();
        err = -3;//已有房间
    }

    packet->Flush();
    packet->Write(mid);
    packet->Write(err);
    packet->Write(room_id);
    packet->Write(cost);
    packet->Write(type);
    packet->Write(playe_type);
    packet->Write(operation);
    packet->Write(players);
    packet->Write(isNotEnter);
    packet->End();

    guard_->gatewayconnector()->SendTo(packet->GetNativeStream());

    if (err == 0)
    {
        auto room = GetRoomObject(room_id);
        DCHECK(room != nullptr);

        auto table = new Table("", players);
        room->ReSetTable(table);

        auto runfastroom = dynamic_cast<PrivateRoom*>(room);
        runfastroom->SetPlayType(playe_type);
        runfastroom->SetOperation(operation);
        runfastroom->SetCreateTime(time(nullptr));
        runfastroom->SetProxyMid(proxy_mid);

        {
            assistx2::Stream stream(Texas::CLIENT_REQUEST_ENTER_ROOM);
            stream.Write(room_id);
            stream.Write(type);
            stream.End();
            OnEnterRoom(player, &stream);

            auto info = DataCenter::getInstance()->MakeRoomInfo(room->getRunFastRoomCfg().ju,
                mid, playe_type, operation, 0, runfastroom->GetCreateTime(), players, proxy_mid, table);
            DataLayer::getInstance()->AddRoomInfo(room_id, info);
        }
    }

    LOG(INFO) << " RunFastGameMgr::OnCreateRoom mid:=" << mid << ",err:=" << err;
}

int32_t RunFastRoomMgr::OnLeaveRoom(PlayerInterface *player, int32_t err)
{
    DLOG(INFO) << "RunFastGameMgr::OnLeaveRoom(), mid:=" << player->GetUID() << ",kick_err:" << kick_err;

    std::int32_t err = 0;

    RoomInterface * room = player->GetRoomObject();
    if (room != nullptr)
    {
        DLOG(INFO) << "RunFastGameMgr::OnLeaveRoom mid:=" << player->GetUID();

        err = room->Leave(player, kick_err);

        DLOG(INFO) << "RunFastGameMgr::OnLeaveRoom, mid:=" << player->GetUID() << ", err:=" << err << ", room:=" << room->GetID();

        if (err == 0)
        {
            auto type = room->getRunFastRoomCfg().type;
            if (type[0] == 'Z' &&
                room->GetSeatPlayerCount() == 0)
            {
                room->SetState(RoomInterface::EMPTY);
                DetachWatingGoldRoom(type, room);
            }
            DCHECK(room->GetPlayer(player->GetUID()) == nullptr);
            player->SetRoomObject(nullptr);
        }
        else
        {
            DCHECK(player->GetRoomObject()->GetID() == room->GetID());
            DCHECK(room->GetPlayer(player->GetUID()) != nullptr);
        }
    }
    else
    {
        LOG(INFO) << "RunFastGameMgr::OnLeaveRoom. ASSERT(player->GetRoomObject() != nullptr). mid:=" << player->GetUID();
    }

    if (player->GetRoomObject() == nullptr)
    {
        if (PlayerInterface::IsRobot(player) == true)
        {
            player->Destroy();
        }
        else if (player->GetLoginStatus() == false)
        {
            DLOG(INFO) << "RunFastGameMgr::OnLeaveRoom RemovePlayer()";
            RemovePlayer(player);
        }
    }

    return err;
}

int32_t RunFastRoomMgr::OnEnterRoom(PlayerInterface *player, assistx2::Stream *packet)
{
    if (g_server_stopped == true &&
        player->GetRoomObject() == nullptr)
    {
        assistx2::Stream result(Texas::SERVER_PUSH_SERVERS_STOPPED);
        result.Write(player->GetUID());
        result.Write(std::int32_t(0));
        result.End();

        guard_->gatewayconnector()->SendTo(result.GetNativeStream());

        DLOG(INFO) << "RunFastGameMgr::OnEnterRoom() g_server_stopped is true";
        return 0;
    }

    ///*const boost::int32_t sid =*/ packet->Read<boost::int32_t>();
    boost::int32_t roomid = packet->Read<std::int32_t>();
    std::string type = packet->Read<std::string>();

    DLOG(INFO) << "RunFastGameMgr::OnEnterRoom() mid:=" << player->GetUID()
        << " roomid:=" << roomid  << " type:=" << type;

    RoomInterface * room = player->GetRoomObject();
    //玩家掉线后，进了别的房间. 因为普通场禁止中途退出，因此禁止玩家进入不同的房间
    if (room != nullptr && room->GetID() != roomid)
    {
        roomid = room->GetID();
        type = room->getRunFastRoomCfg().type;
    }

    RoomInterface * target_room = roommgr_->GetPrivateRoomObject(roomid);

    if (target_room != nullptr)
    {
        const boost::int32_t err = target_room->Enter(player);
        if (err == 0)
        {
            DCHECK(target_room->GetPlayer(player->GetUID()) != nullptr) << "roomid:=" << roomid << ", mid:=" << player->GetUID();
            if (type[0] == 'Z')
            {
                roommgr_->AttachWatingGoldRoom(type, target_room);
            }
            player->SetRoomObject(target_room);
            DLOG(INFO) << "RunFastGameMgr::OnEnterRoom() Success mid:=" << player->GetUID();
        }
        else
        {
            DCHECK(player->GetRoomObject() == nullptr) << "roomid:=" << roomid << ", err:=" << err
                << ", old_room:=" << (room == nullptr ? 0 : room->GetID()) << ", target_room:=" << target_room->GetID();

            DLOG(INFO) << "RunFastGameMgr::OnEnterRoom() Failed mid:=" << player->GetUID();
        }
    }
    else
    {
        auto err = Texas::error_code::ERROR_ENTER_ROOM_NOT_FIND_ROOM;
        if (roomid < 0)
        {
            err = RunFast::ErrorCode::ERROR_ROOM_ID_ERROR;
        }
        assistx2::Stream result(Texas::SERVER_RESPONSE_ENTER_ROOM);
        result.Write(player->GetUID());
        result.Write(err);
        result.Write(XPDKPOKER_VERSION);
        result.Write(0);
        result.Write(0);
        result.End();

        guard_->gatewayconnector()->SendTo(result.GetNativeStream());

        DCHECK(player->GetRoomObject() == nullptr);
        DLOG(INFO) << "RunFastGameMgr::OnEnterRoom() Not Find Room mid:=" << player->GetUID();
    }

    return 0;
}

void RunFastRoomMgr::EnterNextRoom(PlayerInterface *player, int32_t id, std::string type)
{
    assistx2::Stream stream(Texas::CLIENT_REQUEST_ENTER_ROOM);
    stream.Write(id);
    stream.Write(type);
    stream.End();

    OnEnterRoom(player, &stream);
}
