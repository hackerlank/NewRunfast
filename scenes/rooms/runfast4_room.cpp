#include "runfast4_room.h"
#include "poker_cmd.h"

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#include <assistx2/json_wrapper.h>
#include <json_spirit_writer_template.h>
#include <json_spirit_reader_template.h>
#include "player_interface.h"
#include "version.h"
#include "run4_referee.h"
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

extern bool g_server_stopped;
extern std::int32_t g_game_session;

RunFast4Room::RunFast4Room(const boost::int32_t roomid, const runfastroomcfg & cfg) :
    PrivateRoom(roomid, cfg),
    rfReferee_(nullptr),
    card_count_(13)
{
    rfReferee_ = new Run4Referee;
    CHECK_NOTNULL(rfReferee_);
}

RunFast4Room::~RunFast4Room()
{
    if (rfReferee_ != nullptr)
    {
        delete rfReferee_;
        rfReferee_ = nullptr;
    }
}

std::int32_t RunFast4Room::OnMessage(PlayerInterface * player, assistx2::Stream * packet)
{
    const short cmd = packet->GetCmd();

    DLOG(INFO) << "RunFast4Room::OnMessage() cmd:=" << cmd
        << "mid:= " << player->GetUID();

    switch (cmd)
    {
    case RunFast::CLIENT_REQUEST_PLAY:
        OnPlay(player, packet);
        return 0;
    default:
        break;
    }

    
    return PrivateRoom::OnMessage(player,packet);
}

void RunFast4Room::OnTimer(boost::shared_ptr<assistx2::timer2::TimerContext > context)
{
    PrivateRoom::OnTimer(context);

    EventTimerContext *  ptr = dynamic_cast<EventTimerContext *>(context.get());

    if (state_ != PLAYING)
    {
        return;
    }

    //游戏中生效
    switch (ptr->type_)
    {
    case RoomTimerContext::START_TIMER:
        OnDeal();
        break;
    case RoomTimerContext::TIME_END_PLAY:
        OnEndPlay(ptr->seat_);
        break;
    case RoomTimerContext::TIME_GAME_OVER:
        OnGameOver(ptr->seat_->no_);
        break;
    case RoomTimerContext::DELAY_START:
        BroadCastNextPlayer(ptr->seat_->no_);
        break;
     case RoomTimerContext::PLAY_NIL:
         OnPlayedNil(ptr->seat_);
         break;
    default:
        break;
    }
}

void RunFast4Room::SendTableSnapShot(PlayerInterface * player)
{
    PrivateRoom::SendTableSnapShot(player);
    
    if (state_ == RoomInterface::PLAYING)
    {
        BroadCastNextPlayer(activeplayer_,player);
    }
}

void RunFast4Room::OnStartGame()
{
    //test
    if (static_cast<std::int32_t>(players_.size()) < table_->GetSeatCount())
    {
        return;
    }

    for (auto iter : players_)
    {
        auto seat = GetSeat(iter.second->GetSeat());
        if (seat->ready_ == false)
        {
            return;
        }
    }
    DLOG(INFO) << "RunFast4Room::OnStartGame mid:=" << GetID();

    assistx2::Stream stream(Texas::SERVER_BROADCAST_GAMESTART);
    if (num_of_games_ > 0)
    {
        stream.Write(0);
        stream.Write(num_of_games_);
        stream.Write(rfroomcfg_.ju);
        stream.End();
    }
    else
    {
        stream.Write(RunFast::ErrorCode::ERROR_ROOM_COUNT_NULL);
        stream.End();
        return;
    }

    BroadCast(stream);

    state_ = PLAYING;
    activeplayer_ = Table::INVALID_SEAT;
    if (room_state_ == 0)
    {
        room_state_ = 1;
    }
   
    for (auto seat = table_->begin(); seat != table_->end(); ++seat)
    {
        if (seat->user_ != nullptr)
        {
            seat->sitdown_time_ = time(nullptr);
            seat->holecards_.clear();
            seat->playedCard_ = nullptr;
            seat->is_tuoguan_ = false;
            seat->ready_ = false;
            seat->is_zanli_ = false;
            seat->bomb_ = 0;
            seat->score_ = 0;
            seat->bomb_score_ = 0;
            seat->status_ = ((seat->status_ & ~Seat::PLAYER_STATUS_WAITING) | Seat::PLAYER_STATUS_PLAYING);
            DataLayer::getInstance()->AddPlayerStatus(seat->user_->GetUID(), g_game_session);
            RunFastTracer::getInstance()->RemoveAccount(seat->user_->GetUID());
            RunFastTracer::getInstance()->UpdatePlayingPlayer(seat->user_->GetLoginSource(), seat->user_->GetUID());
            RunFastTracer::getInstance()->RemoveRoomMessage(seat->user_->GetUID(), RunFast::SERVER_BROADCAST_ACCOUNTS_EX);
        }
    }

    if (isRobotMode_ == false)
    {
        auto seat = GetSeat(table_->begin()->no_);
        DCHECK(seat != nullptr);

        auto info = DataCenter::getInstance()->MakeRoomInfo(num_of_games_, owner_,play_type_,operation_,
            0, create_time_,table_->GetSeatCount(), proxy_mid_,table_);

        DataLayer::getInstance()->AddRoomInfo(GetID(), info);
    }

    CancelTimer(RoomTimerContext::TIME_DISBAND_ROOM);
    NewTimer(10, RoomTimerContext::START_TIMER, nullptr);
}

void RunFast4Room::OnDeal()
{
    DLOG(INFO) << "RunFast4Room::OnDeal() start";
    if (play_type_ == 1)
    {
        rfReferee_->SetType(PLAY_TYPE::TYPE_TWO);
        card_count_ = 27;//两副牌
    }
    else
    {
        rfReferee_->SetType(PLAY_TYPE::TYPE_DEFAULT);
        card_count_ = 13;//一副牌
    }

    card_generator_->Reset(0, card_count_);
    //DCHECK(card_generator_->count() == play_type_*3);
    //首次出牌人座位号
    auto play_seat = Table::INVALID_SEAT;

    //auto each_count = card_generator_->count() / 3;
    for (auto seat = table_->begin(); seat != table_->end(); ++seat)
    {
        assistx2::Stream stream(Texas::SERVER_PUSH_HOLD_CARDS);
        stream.Write(card_count_);
        for (std::int32_t i = 0; i < card_count_; ++i)
        {
            auto card = card_generator_->Pop();
            seat->holecards_.push_back(card);
            stream.Write(card->getName());
            if (card->getFace() == CardInterface::Three &&
                card->getSuit() == CardInterface::Spades)
            {
                play_seat = seat->no_;
                DLOG(INFO) << "RunFast4Room::OnDeal() first seat:=" << play_seat;
            }
        }
        stream.Write(seat->no_);
        stream.End();

        DCHECK(seat->user_ != nullptr);
        SendTo(seat->user_, stream);
    }

    if (winner_ != nullptr)
    {
        DLOG(INFO) << "RunFast4Room::OnDeal() winner:=" << winner_->GetSeat();
        play_seat = winner_->GetSeat();
        DCHECK_NE(play_seat, Table::INVALID_SEAT);
    }
    else
    {
        assistx2::Stream stream(RunFast::SERVER_NOTIFY_FIRST_PLAY);
        stream.Write(play_seat);
        stream.End();
        BroadCast(stream);
    }

    if (play_seat == Table::INVALID_SEAT)
    {
        //auto owner_obj = GetPlayer(owner_);
        //DCHECK(owner_obj != nullptr);
        play_seat = table_->begin()->no_;
        DCHECK(play_seat != Table::INVALID_SEAT);
    }

    RunFastTracer::getInstance()->OnGameStart(this);

    NewTimer(1000, RoomTimerContext::DELAY_START, GetSeat(play_seat));
    //BroadCastNextPlayer(play_seat);
}

void RunFast4Room::OnPlay(PlayerInterface * player, assistx2::Stream * packet)
{
    Cards cards;
    auto count = packet->Read<std::int32_t>();
    auto seatno = player->GetSeat();

    DCHECK_NE(seatno, Table::INVALID_SEAT);

    if (count > card_count_ || count < 0)
    {
        return SendPlayResult(player, seatno, -1);
    }

    if (activeplayer_ != seatno)
    {
        return SendPlayResult(player, seatno, -3);
    }

    DLOG(INFO) << "RunFast4Room::OnPlay() mid:=" << player->GetUID()
        << " Cards count:=" << count;

    auto seat = GetSeat(seatno);

    DCHECK(seat != nullptr);

    //auto prev_seat = prevSeat(seatno);
    auto next_seat = nextSeat(seatno);
    auto prev_played_seat = GetPrevPlayedSeat(seatno);

    if (count == 0)
    {
        seat->playedCard_ = nullptr;
        if (prev_played_seat == nullptr)
        {
            DLOG(ERROR) << "played cards size = 0 and prev_seat->playedCard_ == nullptr"
                << "&& next_seat->playedCard_ == nullptr  mid:=" << player->GetUID();
            return SendPlayResult(player, seatno, -1);
        }
        else
        {
            auto cards = rfReferee_->autoPlay(*prev_played_seat->playedCard_.get(), seat->holecards_, next_seat->holecards_.size());
            if (cards.size() > 0)
            {
                DLOG(ERROR) << "played cards size = 0 and It has cards that can be played in handcards !"
                    << " mid: = " << player->GetUID();
                return SendPlayResult(player, seatno, -1);
            }
        }
    }
    else
    {
        for (int32_t i = 0; i < count; ++i)
        {
            auto card_name = packet->Read<std::string>();
            auto card = CardFactory::MakePokerCard(card_name);

            if (card == nullptr)
            {
                DLOG(ERROR) << "RunFast4Room::OnPlay(): card is error card_name:="<< card_name << " mid: = " << player->GetUID();
                return SendPlayResult(player, seatno, -1);
            }
            cards.push_back(card);
        }
        DLOG(INFO) << "RunFast4Room::OnPlay()---->PlayedCards:=" << cards << " mid: = " << player->GetUID();

        if (!rfReferee_->IsInHandCard(cards, seat->holecards_))
        {
            DLOG(ERROR) << "RunFast4Room::OnPlay(): PlayedCards not in handcards" << " mid: = " << player->GetUID();
            return SendPlayResult(player, seatno, -1);
        }

        if (count == 1 && next_seat->holecards_.size() == 1)
        {
            if (!rfReferee_->isMaxCard(cards, seat->holecards_))
            {
                DLOG(ERROR) << "next seat only 1 card,please play max card" << " mid: = " << player->GetUID();
                return SendPlayResult(player, seatno, -1);
            }
        }
        int iCount = 0;
        CardInterface::Face firstFace;
        auto type = rfReferee_->getCardType(cards, iCount, firstFace);
        if (type == CardType::TYPE_INVALID)
        {
            DLOG(ERROR) << "CardType == TYPE_INVALID" << " mid: = " << player->GetUID();
            return SendPlayResult(player, seatno, -1);
        }
        DLOG(INFO) << "getCardType:=" << cards << ",iCount:=" << iCount << ",firstFace:=" << firstFace
            << ",type:=" << type;
        seat->playedCard_ = std::make_shared<PlayedCard>(seatno, cards, type, iCount, firstFace);
        if (prev_played_seat == nullptr)
        {
            if (winner_ == nullptr && ((operation_ & 0x02) == 0x02) && rfroomcfg_.ju == num_of_games_)
            {
                if (rfReferee_->IsHaveHei3(seat->holecards_) == true)
                {
                    if (rfReferee_->IsHaveHei3(cards) == false)
                    {
                        return SendPlayResult(player, seatno, -2);
                    }
                }
            }
        }
        else
        {
            if (seat->playedCard_->getType() != TYPE_BOMB &&
                prev_played_seat->playedCard_->getType() != TYPE_BOMB)
            {
                if (seat->playedCard_->getType() != prev_played_seat->playedCard_->getType() ||
                    seat->playedCard_->getCards().size() != prev_played_seat->playedCard_->getCards().size())
                {
                    seat->playedCard_ = nullptr;
                    DLOG(ERROR) << "prev.CardType != now.CardType" << " mid: = " << player->GetUID();
                    return SendPlayResult(player, seatno, -1);
                }
            }
            auto res = rfReferee_->Compare(*seat->playedCard_.get(), *prev_played_seat->playedCard_.get());
            if (res != 1)
            {
                seat->playedCard_ = nullptr;
                DLOG(ERROR) << "prev.CardType > now.CardType" << " mid: = " << player->GetUID();
                return SendPlayResult(player, seatno, -1);
            }
        }
    }
    CancelTimer(RoomTimerContext::TIME_OUT_PLAY);
    CancelTimer(RoomTimerContext::PLAY_NIL,seat);

    RemoveCards(seat, cards);
    SendPlayResult(player, seatno, 0);
    BroadCastPlayedCard(seatno, cards);
    if (seat->holecards_.size() == 0)
    {
        NewTimer(1, RoomTimerContext::TIME_GAME_OVER, seat);
    }
    else
    {
        BroadCastNextPlayer(nextSeat(seatno)->no_);
    }
}

void RunFast4Room::OnGameOver(std::int32_t seatno)
{
    DLOG(INFO) << "RunFast4Room::OnGameOver() seatno" << seatno;

    DCHECK_NE(seatno, Table::INVALID_SEAT);

    num_of_games_ -= 1;

    auto winseat = GetSeat(seatno);

    assistx2::Stream stream(RunFast::SERVER_BROADCAST_GAMEOVER);
    stream.Write(table_->GetPlayerCount());
    for (Table::Iterator seat = table_->begin(); seat != table_->end(); ++seat)
    {
        stream.Write(seat->no_);
        stream.Write(static_cast<std::int32_t>(seat->holecards_.size()));
        for (auto iter : seat->holecards_)
        {
            stream.Write(iter->getName());
        }
    }
    stream.Write(num_of_games_);
    stream.End();

    BroadCast(stream);

    for (Table::Iterator seat = table_->begin(); seat != table_->end(); ++seat)
    {
        if (seat->no_ == seatno)
        {
            continue;
        }
        auto score = static_cast<std::int32_t>(seat->holecards_.size());
        if (score == 1)
        {
            score = 0;
        }
        else if(score == card_count_)
        {
            score = score*bet_ * 2;
            RunFastTracer::getInstance()->WriteChuntianBomb(winseat->user_->GetUID(), 'D', Mingtang::CHUNTIAN);//名堂统计
            DataManager::getInstance()->MingTang(winseat->user_, Mingtang::CHUNTIAN, this);//名堂上报数据中心
        }
        else
        {
            score = score*bet_;
        }
        winseat->score_ += score;
        seat->score_ -= score;
    }

	PrivateRoom::OnGameOver(seatno);
    OnAccountEvent(seatno);
    OnDataRecord(seatno);

    winner_ = winseat->user_;
    state_ = WAITING;

    activeplayer_ = Table::INVALID_SEAT;

    if (isRobotMode_ == false)
    {

        auto info = DataCenter::getInstance()->MakeRoomInfo(num_of_games_, owner_, play_type_,operation_,
            winner_->GetUID(), create_time_, table_->GetSeatCount(), proxy_mid_,table_);

        DataLayer::getInstance()->AddRoomInfo(GetID(), info);
    }

    for (Table::Iterator seat = table_->begin(); seat != table_->end(); ++seat)
    {
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
    
    if (num_of_games_ > 0)
    {
        return;
    }

    type_ = 1;

    OnRoomAccount();
	PrivateRoom::OnRoomAccount();

    Disband(nullptr);
}

void RunFast4Room::OnForceGameOver()
{
    DLOG(INFO) << "RunFast4Room::OnForceGameOver() ";

    assistx2::Stream stream(RunFast::SERVER_BROADCAST_FORCE_GAMEOVER);
    stream.Write(table_->GetPlayerCount());
    for (Table::Iterator seat = table_->begin(); seat != table_->end(); ++seat)
    {
        seat->room_score_ += seat->score_ + seat->bomb_score_;
        stream.Write(seat->no_);
        stream.Write(static_cast<std::int32_t>(seat->holecards_.size()));
        for (auto iter : seat->holecards_)
        {
            stream.Write(iter->getName());
        }
    }
    stream.End();

    BroadCast(stream);
    OnDataRecord(0);

    state_ = WAITING;
    activeplayer_ = Table::INVALID_SEAT;

    for (Table::Iterator seat = table_->begin(); seat != table_->end(); ++seat)
    {
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
}

void RunFast4Room::OnDataRecord(int32_t winner)
{
    std::string strGameData = "";
    std::string strRoomData = "";

    for (Table::Iterator seat = table_->begin(); seat != table_->end(); ++seat)
    {
        auto isWin = false;
        if (seat->no_ == winner)
        {
            isWin = true;
        }
        auto score = seat->score_ + seat->bomb_score_;

        game_data_mgr_.UpdateGameData(seat->no_, score, seat->bomb_, isWin);
        auto game_data = game_data_mgr_.GetGameData(seat->no_);
        strGameData += game_data_mgr_.MakeGameData(seat->user_->GetUID(),seat->score_, game_data.sum_scroe, seat->bomb_score_);
        strRoomData += game_data_mgr_.MakeRoomData(seat->user_->GetUID(), game_data);
    }

    std::stringstream uid;
    uid << GetID();
    uid << "_";
    uid << create_time_;

    if (isRobotMode_ == false)
    {
        DataLayer::getInstance()->AddRoomRecordInfo(GetID(), strRoomData);
    }
    
    RunFastTracer::getInstance()->OnGameOver(this);

    if (winner != 0)
    {
        winner = GetSeat(winner)->user_->GetUID();
    }
    
    RunFastTracer::getInstance()->WirteGameRecordSub(uid.str(), rfroomcfg_.ju - num_of_games_, strGameData, winner,this);
}

void RunFast4Room::SendPlayResult(PlayerInterface * player, std::int32_t seatno, std::int32_t err)
{
    assistx2::Stream stream(RunFast::SERVER_RESPONSE_PLAY);
    stream.Write(seatno);
    stream.Write(err);
    stream.End();

    SendTo(player, stream);
}

void RunFast4Room::BroadCastPlayedCard(std::int32_t seatno, const Cards& cards, bool isend)
{
    IsPlayed = true;
    int count = 0;
    CardInterface::Face firstface;
    auto seat = GetSeat(seatno);

    assistx2::Stream stream(RunFast::SERVER_BROADCAST_PLAYED_CARD);
    stream.Write(seatno);
    stream.Write(static_cast<std::int32_t>(cards.size()));

    auto type = rfReferee_->getCardType(cards, count, firstface);
	/*if (TYPE_INVALID == type)
	{
		rfReferee_->IsEndfly_EndThreeZone(cards, firstface, type, count);
	}*/
    auto new_cards = rfReferee_->SortCard(cards, type, count, firstface);

    for (auto iter : new_cards)
    {
        stream.Write(iter->getName());
    }
    stream.Write(static_cast<std::int32_t>(type));
    stream.Write(static_cast<std::int32_t>(seat->holecards_.size()));

    stream.End();

    BroadCast(stream);

    if (type == TYPE_BOMB)
    {
        if (isend == false)
        {
            bomb_info_.clear();
            bomb_info_.push_back(seatno);
        }
        else
        {
            bomb_info_.clear();
            OnBombAccountEvent(seatno);
        }
    }
    else if (bomb_info_.size() > 0)
    {
        bomb_info_.push_back(seatno);
        if (static_cast<std::int32_t>(bomb_info_.size()) == table_->GetSeatCount())
        {
            OnBombAccountEvent(bomb_info_[0]);
            bomb_info_.clear();
        }
    }

    RunFastTracer::getInstance()->WritePlayRecord(GetID(), seat->user_->GetUID(), seatno, new_cards, seat->holecards_, type);
}

void RunFast4Room::BroadCastNextPlayer(std::int32_t seatno, PlayerInterface * player)
{
    DLOG(INFO) << "RunFast4Room::BroadCastNextPlayer() seatno:=" << seatno;
    if (seatno == Table::INVALID_SEAT)
    {
        return;
    }

    activeplayer_ = seatno;

    auto isyaobuqi = false;
    if (OnNextSeatPlay(seatno, isyaobuqi) == true)
    {
        return;
    }

    std::int32_t isOneCard = 0;
    auto next_seat = nextSeat(seatno);
    if (next_seat->holecards_.size() == 1)
    {
        isOneCard = 1;
    }

    if (player == nullptr)
    {
        GetSeat(seatno)->start_time_ = time(nullptr);
        if (isyaobuqi == true)
        {
            GetSeat(seatno)->start_time_ = +18;
        }
    }
   
    auto prev_played_seat = GetPrevPlayedSeat(seatno);

    //服务器广播出牌人
    assistx2::Stream stream(RunFast::SERVER_BROADCAST_NEXT_PLAYER);
    stream.Write(seatno);
    stream.Write(isOneCard);//下家是否报单;
    stream.Write(static_cast<std::int32_t>(isyaobuqi));//下家是否要不起;
    if (prev_played_seat != nullptr)
    {
        stream.Write(prev_played_seat->no_);
        auto played_card = prev_played_seat->playedCard_;
        if (played_card != nullptr)
        {
            auto played_cards = played_card->getCards();
            stream.Write(static_cast<std::int32_t>(played_cards.size()));
            for (auto iter : played_cards)
            {
                stream.Write(iter->getName());
            }
            stream.Write(static_cast<std::int32_t>(played_card->getType()));
        }
        else
        {
            stream.Write(0);
        }
    }
    else
    {
        stream.Write(seatno);
        stream.Write(0);
    }
    stream.End();

    if (player == nullptr)
    {
        BroadCast(stream);
    }
    else
    {
        SendTo(player, stream);
    }
}

void RunFast4Room::OnBombAccountEvent(int32_t seatno)
{
    auto winseat = GetSeat(seatno);
    DCHECK(winseat != nullptr);
    
    winseat->bomb_ += 1;

    auto winscore = 0;
    auto score = 10 * bet_;
    for (Table::Iterator seat = table_->begin(); seat != table_->end(); ++seat)
    {
        if (seat->no_ == seatno)
        {
            continue;
        }
        winscore += score;
        winseat->bomb_score_ += score;
        seat->bomb_score_ -= score;
    }

    DLOG(INFO) << "RunFastRoom::OnBombAccountEvent() seatno:=" << seatno << ",winscore:=" << winscore;

    assistx2::Stream stream(RunFast::SERVER_BROADCAST_BOMB_ACCOUNTS);
    stream.Write(table_->GetPlayerCount());
    for (Table::Iterator seat = table_->begin(); seat != table_->end(); ++seat)
    {
        stream.Write(seat->no_);
        if (seat->no_ == seatno)
        {
            stream.Write(static_cast<std::int32_t>(winscore));
        }
        else
        {
            stream.Write(static_cast<std::int32_t>(-score));
        }
        std::int64_t sum_socre = seat->room_score_ + seat->bomb_score_;
        stream.Write(sum_socre);
    }
    stream.End();

    BroadCast(stream);
    RunFastTracer::getInstance()->WriteChuntianBomb(winseat->user_->GetUID(), 'D', Mingtang::BOMB);//名堂统计
    DataManager::getInstance()->MingTang(winseat->user_, Mingtang::BOMB, this);//名堂上报数据中心
}

void RunFast4Room::OnAccountEvent(int32_t seatno)
{
    DLOG(INFO) << "RunFast4Room::OnAccountEvent() seatno:=" << seatno;

    auto seat = GetSeat(seatno);
    DCHECK(seat != nullptr);

    OnPiaoScore();

    //新协议
    assistx2::Stream stream_ex(RunFast::SERVER_BROADCAST_ACCOUNTS_EX);
    stream_ex.Write(static_cast<std::int32_t>(owner_));
    stream_ex.Write(seat->user_->GetUID());
    stream_ex.Write(num_of_games_);
    stream_ex.Write(rfroomcfg_.ju);
    stream_ex.Write(table_->GetPlayerCount());
    for (Table::Iterator p = table_->begin(); p != table_->end(); ++p)
    {
        stream_ex.Write(p->no_);
        stream_ex.Write(p->user_->GetUID());
        stream_ex.Write(p->user_->getRoleInfo().name());
        stream_ex.Write(p->score_);
        stream_ex.Write(p->score_ + p->bomb_score_);
        stream_ex.Write(p->bomb_);
        stream_ex.Write(p->room_score_);
        stream_ex.Write(static_cast<std::int32_t>(p->holecards_.size()));
        stream_ex.Write(p->bei_count_);
        stream_ex.Write(static_cast<std::int32_t>( (static_cast<std::int32_t>(p->holecards_.size()) == card_count_)));
        stream_ex.Write(std::string(""));
    }
    stream_ex.Write(GetID());
    stream_ex.Write(static_cast<std::int32_t>(time(nullptr)));
    stream_ex.Write(play_type_);
    stream_ex.Write(operation_);
    stream_ex.Write(rfroomcfg_.type);
    stream_ex.End();

    for (auto iter : players_)
    {
        RunFastTracer::getInstance()->SaveRoomMessage(iter.first, stream_ex);
    }

    RunFastTracer::getInstance()->OnGameAccount(GetID(), stream_ex);

    BroadCast(stream_ex);
}

void RunFast4Room::OnRoomAccount(bool isreturn)
{
    if (state_ == PLAYING)
    {
        OnForceGameOver();
    }

    //新协议
    std::stringstream ss;
    assistx2::Stream stream_ex(RunFast::SERVER_BROADCAST_ROOM_ACCOUNTS_EX);
    stream_ex.Write(static_cast<std::int32_t>(owner_));
    stream_ex.Write(table_->GetPlayerCount());
    for (Table::Iterator seat = table_->begin(); seat != table_->end(); ++seat)
    {
        if (seat->user_ != nullptr)
        {
            auto game_data = game_data_mgr_.GetGameData(seat->no_);
            stream_ex.Write(seat->user_->GetUID());
            stream_ex.Write(seat->user_->getRoleInfo().name());
            stream_ex.Write(seat->user_->getRoleInfo().icon());
            stream_ex.Write(game_data.max_score);
            stream_ex.Write(game_data.bomb_count);
            stream_ex.Write(game_data.win_count);
            stream_ex.Write(game_data.lost_count);
            stream_ex.Write(game_data.sum_scroe);
            stream_ex.Write(std::string(""));
            ss << game_data.sum_scroe << ";";
        }
        seat->room_score_ = 0;
    }
    stream_ex.Write(GetID());
    stream_ex.Write(static_cast<std::int32_t>(time(nullptr)));
    stream_ex.Write(play_type_);
    stream_ex.Write(operation_);
    stream_ex.Write(rfroomcfg_.ju);
    stream_ex.Write(rfroomcfg_.type);
    stream_ex.End();

    for (auto iter : players_)
    {
        RunFastTracer::getInstance()->SaveRoomMessage(iter.first, stream_ex);
    }

    BroadCast(stream_ex);

    std::stringstream uid;
    uid << GetID() << "_" << create_time_;

    auto master = owner_;
    if (proxy_mid_ != 0)
    {
        master = proxy_mid_;
    }

    auto pay = rfroomcfg_.cost;
    auto isCreateInFreeTime = ConfigMgr::getInstance()->IsInFreeTime(rfroomcfg_.type, create_time_);
    if (isCreateInFreeTime == true)
    {
        pay = 0;
    }
    else if (isreturn == true)
    {
        pay = -1;
    }

    RunFastTracer::getInstance()->WirteGameRecord(GetID(), rfroomcfg_.ju - num_of_games_,
        table_,ss.str(), uid.str(), master, pay, rfroomcfg_.type);

    game_data_mgr_.ClearAll();
}

bool RunFast4Room::OnNextSeatPlay(std::int32_t seatno,bool& isyaobuqi)
{
    if (seatno == Table::INVALID_SEAT)
    {
        return false;
    }
    auto seat = GetSeat(seatno);
    auto next_seat = nextSeat(seatno);
    auto prev_played_seat = GetPrevPlayedSeat(seatno);
    
    Cards cards;
    if (prev_played_seat == nullptr)
    {
        cards = rfReferee_->autoPlay(seat->holecards_);
    }
    else
    {
        cards = rfReferee_->autoPlay(*prev_played_seat->playedCard_.get(), seat->holecards_, next_seat->holecards_.size());
    }
    if (cards.size() == seat->holecards_.size())
    {
        NewTimer(1000, RoomTimerContext::TIME_END_PLAY, seat);
        return true;
    }
    if (cards.size() == 0)
    {
        isyaobuqi = true;
        auto deltime = 2000;
        if ((operation_ & 0x08) == 0x08)
        {
            deltime = 1000;
        }
        NewTimer(deltime, RoomTimerContext::PLAY_NIL, seat);
        return false;
    }
  
    return false;
}

void RunFast4Room::OnEndPlay(Seat* seat)
{
    Cards cards(seat->holecards_);
    RemoveCards(seat, cards);
    BroadCastPlayedCard(seat->no_, cards,true);
    
    NewTimer(1, RoomTimerContext::TIME_GAME_OVER, seat);
}

void RunFast4Room::OnPlayedNil(Seat* seat)
{
    if (seat->no_ != activeplayer_)
    {
        return;
    }

    seat->playedCard_ = nullptr;
    
    Cards cards;
    BroadCastPlayedCard(seat->no_, cards);
    BroadCastNextPlayer(nextSeat(seat->no_)->no_);
}
