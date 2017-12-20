#include "RunFastRobot.h"
#include "PokerCmd.h"
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#include <glog/stl_logging.h>
#include "Table.h"
#include "RoomInterface.h"
#include "PlayLogic.h"
#include "TimerHelper.h"

class RunFastRobotImpl
{
public:
    RunFastRobotImpl(RunFastRobot* owner);
    ~RunFastRobotImpl();

    bool Init();

    std::int32_t PrevSeat(std::int32_t seatno);
    std::int32_t NextSeat(std::int32_t seatno);

    void OnEnterRoom(assistx2::Stream *packet);
    void OnGameStart(assistx2::Stream *packet);
    void OnPlayedCard(assistx2::Stream *packet);
    void OnPlay();
    void OnGameOver();
    void OnPlayCard(RoomInterface*room,Cards cards);
    void OnReady();
public:
    RunFastRobot* owner_;
    bool destroy_;
    boost::function<void(PlayerInterface *) > leave_func_;
    Cards cards_;
    std::int32_t my_seat_ = 0;
    std::shared_ptr<PlayedCard> prev_cards_ = nullptr;
    std::shared_ptr<PlayedCard> next_cards_ = nullptr;
    std::shared_ptr< PlayLogic > logic_ = nullptr;
    bool isBaodan = false;
    bool isReady = false;
};


RunFastRobot::RunFastRobot(int32_t mid, DelaySendTo delay_send_helper, boost::function<void(PlayerInterface *) > & kickout_callback) :
    Player(mid), 
    pImpl_(new RunFastRobotImpl(this))
{
    type_ = ROBOT_TYPE;
    pImpl_->destroy_ = false;
    pImpl_->leave_func_ = kickout_callback;
    pImpl_->logic_ = std::make_shared<PlayLogic>();

    DCHECK(pImpl_->Init() == true);
}

RunFastRobot::~RunFastRobot(void)
{


}

std::int32_t RunFastRobot::Serialize(bool loadorsave)
{
    if (GetSeat() != Table::INVALID_SEAT)
    {
        PlayerBase::SitUp();
    }
    pImpl_->isReady = false;

    if (loadorsave == true)
    {
        pImpl_->destroy_ = false;
        SetLoginStatus(true);
    }
    else
    {

        riches_rank_ = 0;
        win_rank_ = 0;
        cards_.clear();
    }

    return 0;
}

void RunFastRobot::SendTo(const assistx2::Stream & packet)
{
    if (pImpl_->destroy_ == true)
    {
        return;
    }

    GlobalTimerProxy::getInstance()->NewTimer(
        std::bind(&RunFastRobot::OnMessage, this, packet), 1);
}

void RunFastRobot::OnMessage(assistx2::Stream clone)
{
    const short cmd = clone.GetCmd();
    switch (cmd)
    {
    case Texas::SERVER_RESPONSE_ENTER_ROOM:
        pImpl_->OnEnterRoom(&clone);
        break;
    case Texas::SERVER_PUSH_HOLD_CARDS:
        pImpl_->OnGameStart(&clone);
        break;
    case RunFast::SERVER_BROADCAST_PLAYED_CARD:
        pImpl_->OnPlayedCard(&clone);
        break;
    case 1072:
    {
        /*auto seatno =*/ clone.Read<std::int32_t>();
        auto err = clone.Read<std::int32_t>();
        DLOG(INFO) << "RunFastRobot::Play mid:=" << GetUID()
            << ",err:=" << err;
    }
    break;
    case RunFast::SERVER_BROADCAST_NEXT_PLAYER:
    {
        auto seatno = clone.Read<std::int32_t>();
        auto value = clone.Read<std::int32_t>();
        auto  isyaobuqi = clone.Read<std::int32_t>();

        pImpl_->isBaodan = static_cast<bool>(value);

        if (seatno == pImpl_->my_seat_)
        {
            if (isyaobuqi == 1)
            {
                return;
            }
            else
            {
                pImpl_->OnPlay();
            }
        }
    }
    break;
    case RunFast::SERVER_BROADCAST_GAMEOVER:
        pImpl_->OnGameOver();
        break;
    default:
        break;
    }
}

void RunFastRobot::Destroy()
{
    DLOG(INFO) << "RunFastRobot::Destroy mid:=" << GetUID();
    DCHECK(pImpl_->destroy_ == false);
    pImpl_->destroy_ = true;

    pImpl_->leave_func_(this);
    SetLoginStatus(false);
    pImpl_->isReady = false;
}

void RunFastRobot::SetRoomObject(RoomInterface * roomobject)
{
    PlayerBase::SetRoomObject(roomobject);
}

RunFastRobotImpl::RunFastRobotImpl(RunFastRobot* owner):
 owner_(owner)
{
}

RunFastRobotImpl::~RunFastRobotImpl()
{
}

void RunFastRobotImpl::OnEnterRoom(assistx2::Stream *packet)
{
    auto err = packet->Read<std::int32_t>();
    auto version = packet->Read<std::int32_t>();
    if (err == 0)
    {
        GlobalTimerProxy::getInstance()->NewTimer(std::bind(&RunFastRobotImpl::OnReady,
            this), 2);
    }
    else
    {
        owner_->Destroy();
    }
    DLOG(INFO) << "RunFastRobot::OnEnterRoom(): robot:=" << owner_->GetUID()
        << " err:=" << err << " version:=" << version;
}

void RunFastRobotImpl::OnGameStart(assistx2::Stream *packet)
{
    cards_.clear();
    my_seat_ = 0;
    prev_cards_ = nullptr;
    next_cards_ = nullptr;
    isBaodan = false;
    isReady = false;

    auto count = packet->Read<std::int32_t>();
    //auto count = 16;
    for (auto i = 0; i < count; ++i)
    {
        auto card_str = packet->Read<std::string>();
        auto card = CardFactory::MakePokerCard(card_str);
        cards_.push_back(card);
    }
    my_seat_ = packet->Read<std::int32_t>();
}

void RunFastRobotImpl::OnPlayedCard(assistx2::Stream *packet)
{
    auto seatno = packet->Read<std::int32_t>();
    auto card_size = packet->Read<std::int32_t>();
    if (card_size == 0)
    {
        if (seatno == PrevSeat(my_seat_))
        {
            prev_cards_ = nullptr;
        }
        else
        {
            next_cards_ = nullptr;
        }
        return;
    }
    Cards cards;
    for (std::int32_t i = 0; i < card_size; ++i)
    {
        auto card_str = packet->Read<std::string>();
        auto card = CardFactory::MakePokerCard(card_str);
        cards.push_back(card);
    }
   
    if (seatno == my_seat_)
    {
        for (auto iter : cards)
        {
            auto it = find(cards_.begin(), cards_.end(), iter);
            if (it != cards_.end())
            {
                cards_.erase(it);
            }
            else
            {
                DLOG(ERROR) << "RunFastRobotImpl::OnPlay():"
                    << "The Card Face: = " << iter->getFace()
                    << " Suit:= " << iter->getSuit()
                    << "not in holecards";
            }
        }
        return;
    }
  
    std::int32_t count = 0;
    CardInterface::Face firstface = CardInterface::FirstFace;
    auto res = logic_->getCardType(cards, count, firstface);

    if (seatno == PrevSeat(my_seat_))
    {
        prev_cards_ = std::make_shared<PlayedCard>(seatno,cards,res,count,firstface);
    }
    else
    {
        next_cards_ = std::make_shared<PlayedCard>(seatno, cards, res, count, firstface);
    }
}

void RunFastRobotImpl::OnPlay()
{
    Cards cards;
    std::int32_t nszie = 2;
    if (isBaodan)
    {
        nszie = 1;
    }
    if (prev_cards_ == nullptr &&
        next_cards_ == nullptr)
    {
//         for (auto iter : cards_)
//         {
//             if (iter->getFace() == CardInterface::Face::Three&&
//                 iter->getSuit() == CardInterface::Suit::Spades)
//             {
//                 cards.push_back(iter);
//                 break;
//             }
//         }
//        if (cards.size() == 0)
//       {
            cards = logic_->autoPlay_AI(cards_, nszie);
 //       }
    }
    else if (prev_cards_ == nullptr)
    {
        DLOG(INFO) << "RunFastRobotImpl::OnPlay() next_cards" << next_cards_->getCards();
        cards = logic_->autoPlay(*next_cards_.get(), cards_, nszie);
    }
    else
    {
        DLOG(INFO) << "RunFastRobotImpl::OnPlay() next_cards" << prev_cards_->getCards();
        cards = logic_->autoPlay(*prev_cards_.get(), cards_, nszie);
    }
    auto room = owner_->GetRoomObject();
    if (room != nullptr)
    {
        if (cards.size() != 0)
        {
            if (room->IsRobotRoom())
            {
                OnPlayCard(room, cards);
            }
            else
            {
                auto time = rand() % 3 + 1;
                GlobalTimerProxy::getInstance()->NewTimer(std::bind(&RunFastRobotImpl::OnPlayCard,
                    this, room, cards), time);
            }
        }
        else
        {
            //OnPlayCard(room, cards);
        }
    }
}

void RunFastRobotImpl::OnPlayCard(RoomInterface*room, Cards cards)
{
    assistx2::Stream stream(RunFast::CLIENT_REQUEST_PLAY);
    stream.Write(static_cast<std::int32_t>(cards.size()));
    for (auto iter : cards)
    {
        stream.Write(iter->getName());
    }
    stream.End();

    room->OnMessage(owner_, &stream);

    DLOG(INFO) << "RunFastRobotImpl::OnPlay() cards" << cards;
}

void RunFastRobotImpl::OnGameOver()
{
    cards_.clear();
    my_seat_ = 0;
    prev_cards_ = nullptr;
    next_cards_ = nullptr;
    isBaodan = false;
    isReady = false;

    auto rand_time = rand() % 3 + 8;
    GlobalTimerProxy::getInstance()->NewTimer(std::bind(&RunFastRobotImpl::OnReady,
        this), rand_time);
}

std::int32_t RunFastRobotImpl::PrevSeat(std::int32_t seatno)
{
    if (seatno > 1)
    {
        return seatno - 1;
    }
    else
    {
        return 3;
    }
}

std::int32_t RunFastRobotImpl::NextSeat(std::int32_t seatno)
{
    if (seatno < 3)
    {
        return seatno + 1;
    }
    else
    {
        return 1;
    }
}

void RunFastRobotImpl::OnReady()
{
    if (isReady == false)
    {
        auto room = owner_->GetRoomObject();
        if (room != nullptr)
        {
            assistx2::Stream stream(Texas::CLIENT_REQUEST_SITDOWN);
            stream.End();
            room->OnMessage(owner_, &stream);
            isReady = true;
            DLOG(INFO) << "RunFastRobotImpl::OnReady() mid:=" << owner_->GetUID();
        }
    }
}

bool RunFastRobotImpl::Init()
{
    auto mid = rand() % 2000 + 801;
    std::string tmp;
    if (DataLayer::getInstance()->GetPlayerBaseInfo(mid, tmp, owner_->roleinfo_) != 0)
    {
        DLOG(INFO) << "GetPlayerBaseInfo failed mid:=" << mid;
        return false;
    }

    if (DataLayer::getInstance()->GetCommonGameInfo(mid, owner_->game_base_, true) != 0)
    {
        DLOG(INFO) << "GetCommonGameInfo failed mid:=" << mid;
        return false;
    }

//    if (DataLayer::getInstance()->GetPlayerGameInfo(mid, owner_->gameinfo_) != 0)
//    {
//        DLOG(INFO) << "GetPlayerGameInfo failed mid:=" << mid;
//        return false;
//    }

    owner_->roleinfo_.set_mid(mid);

    return true;
}
