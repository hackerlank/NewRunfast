#include "player_mgr.h"
#include "handle_obj.h"
#include "player_interface.h"
#include "poker_cmd.h"
#include "runfast_tracer.h"
#include "room_interface.h"

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

PlayerMgr::PlayerMgr()
{

}

PlayerMgr::~PlayerMgr()
{

}

int32_t PlayerMgr::OnMessage(assistx2::Stream &packet, PlayerInterface* player)
{
    DCHECK_NOTNULL(player);

    const std::int32_t cmd = packet.GetCmd();
    DLOG(INFO) << "RunFastGameMgr::OnMessage()->cmd:" << cmd;

    switch(cmd)
    {
    case Texas::CLEINT_REQUEST_GET_PLAYER_ROOM:
        GetPlayerRoom(&packet);
        break;
    case RunFast::SERVER_BROADCAST_ROOM_ACCOUNTS:
        RunFastTracer::getInstance()->RemoveRoomAccount(player->GetUID());
        break;
    case RunFast::SERVER_BROADCAST_ACCOUNTS:
        RunFastTracer::getInstance()->RemoveAccount(player->GetUID());
        break;
    case RunFast::SERVER_MATCH_ELIMINATE_PAYER:
    case RunFast::SERVER_MATCH_PRIZE:
        RunFastTracer::getInstance()->RemoveMatchMessage(player->GetUID());
        break;
    case RunFast::SERVER_BROADCAST_ROOM_ACCOUNTS_EX:
    case RunFast::SERVER_BROADCAST_ACCOUNTS_EX:
        RunFastTracer::getInstance()->RemoveRoomMessage(player->GetUID(), cmd);
        break;
//    case GAMESERVER_ROUTE_PACKET:
//        OnRouteMatchServerResult(player, &packet);
//        return 0;
    case Texas::GATEWAY_EVENT_CONNECT_CLOSE:
        player->SetLoginStatus(false);
        break;
    default:
        player->OnMessage(packet);
        break;
    }

    return 0;
}

void PlayerMgr::RemovePlayer(PlayerInterface * player)
{
    DLOG(INFO) << "RunFastGameMgr::RemovePlayer, mid:=" << player->GetUID();

    player->Serialize(false);
    if (PlayerInterface::IsRobot(player) == false)
    {
        DCHECK(players_.find(player->GetUID()) != players_.end());
        DLOG(INFO) << "RunFastGameMgr::RemovePlayer()";
        players_.erase(player->GetUID());
    }
    player->Destroy();
}

PlayerInterface *PlayerMgr::GetPlayerInterface(const uid_type mid)
{
    std::map<uid_type, PlayerInterface * >::iterator it = players_.find(mid);
    if (it != players_.end())
    {
        DCHECK_NOTNULL(it->second);
        return it->second;
    }
    return nullptr;
}

int32_t PlayerMgr::OnConnectClose(PlayerInterface *player, assistx2::Stream *stream)
{
    DLOG(INFO) << "RunFastGameMgr::OnConnectClose, mid:=" << player->GetUID();

    RunFastTracer::getInstance()->UpdateLoginPlayer(player->GetLoginSource(), player->GetUID(),false);

    player->SetConnectStatus(false);
    player->SetLoginStatus(false);

    if (player->GetRoomObject())
    {
        player->GetRoomObject()->OnMessage(player, stream);
    }
    else
    {
        RemovePlayer(player);
    }

    return 0;
}

int32_t PlayerMgr::OnLogin(const uid_type uid, PlayerInterface *&player, const int32_t login_source)
{
    DCHECK(players_.find(uid) == players_.end());

    std::int32_t result = -1;

    player = PlayerInterface::CreateRealPlayer(uid);
    if (player != nullptr)
    {
        if (player->Serialize(true) != 0)
        {
            player->Destroy();
            player = nullptr;
            return -3;
        }
        player->SetLoginStatus(true);
        player->SetLoginSource(login_source);
        players_.insert(std::make_pair(uid, player));
        result = 0;
    }
    else
    {
        result = -2;
    }
    return result;
}

void PlayerMgr::SendReConnect(const int32_t &mid, const int32_t &roomid, const std::string &roomtype)
{
    DLOG(INFO) << "RunFastGameMgr::SendReConnect()->  mid:" << mid << " roomtype:" << roomtype;

    assistx2::Stream stream(Texas::SERVER_REQUEST_RECONNECT);
    stream.Write(mid);
    stream.Write(roomid);
    stream.Write(roomtype);
    stream.End();

    gatewayconnector_.SendTo(stream.GetNativeStream());
}

void PlayerMgr::OnHeartBeat(PlayerInterface *player, assistx2::Stream *packet)
{

}

int32_t PlayerMgr::GetPlayerRoom(assistx2::Stream *packet)
{
    const std::int32_t mid = packet->Read<std::int32_t>();//自己
    const std::int32_t target = packet->Read<std::int32_t>();//对方
    DLOG(INFO) << "CLEINT_REQUEST_GET_PLAYER_ROOM mid:=" << mid << ",  target:=" << target;

    std::int32_t room_id = 0;
    auto pPlayer = GetPlayerInterface(target);
    if (pPlayer != nullptr && pPlayer->GetRoomObject() != nullptr)
    {
        room_id = pPlayer->GetRoomObject()->GetID();
    }
    packet->Flush();
    packet->Write(mid);
    packet->Write(room_id != 0 ? 0 : -1);
    packet->Write(room_id);
    packet->End();
    gatewayconnector_.SendTo(packet->GetNativeStream());

    return 0;
}

void PlayerMgr::OnRouteMatchServerResult(PlayerInterface *player, assistx2::Stream *packet)
{
    auto mid = player->GetUID();
    auto res = packet->Read<std::int32_t>();

    DLOG(INFO) << "OnRouteMatchServerResult mid:=" << mid << "res:=" << res;

    if (res == -1)
    {
        assistx2::Stream stream(RunFast::SERVER_ENTER_MATCH_HALL);
        stream.Write(mid);
        stream.Write(RunFast::ErrorCode::ERROR_MATCH_SERVER_CLOSED);
        stream.End();

        gatewayconnector_.SendTo(stream.GetNativeStream());

        auto palyer = GetPlayerInterface(mid);
        if (palyer != nullptr)
        {
            palyer->SetMatchProxy(nullptr);
        }
    }
}

