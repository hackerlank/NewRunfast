#ifndef _XPOKER_DB_DATALAYER_H_
#define _XPOKER_DB_DATALAYER_H_

#include <set>
#include <memory>

#include <cpp_redis.h>


#include <assistx2/singleton.h>
#include <assistx2/connector.h>
#include <assistx2/memcached_wrapper.h>

#include "xpoker.h"
#include "data_def.h"
#include "membergame.pb.h"
#include "memberfides.pb.h"
#include "membercommongame.pb.h"

class Database;

class DataLayer : public Singleton<DataLayer>
{
public:
	DataLayer(void);
	virtual ~DataLayer(void);

	std::int32_t Init(boost::asio::io_service * engine);

public:
	std::int32_t UpdateGameData(uid_type mid, 	std::int32_t win_count, std::int32_t round_count,  std::string handstrength, const chips_type maxwin);
	std::int32_t UpdateRunFastScore(uid_type mid,std::int32_t score);
  std::int32_t SetRunFastGameRecord(std::int32_t roomid, int32_t num_of_games,
    int32_t player1_mid, int32_t player2_mid, int32_t player3_mid, const std::string& data_game, int32_t winner);
	//金币支付接口, incr 可正，可负 扣錢為正， 加錢為負
	std::int32_t Pay(const uid_type mid, const chips_type incr, chips_type &  amount, chips_type& real_delta, bool bForce = false);

    std::int32_t PayProps(const uid_type mid, std::int32_t pcate,std::int32_t pframe, std::int32_t num, bool isPay = true);
    std::int32_t AddProps(const uid_type mid, std::int32_t pcate, std::int32_t pframe, std::int32_t num);

    void AddRoomInfo(const std::int32_t roomid,const std::string& info);
    void RemoveRoomInfo(const std::int32_t roomid);
    bool GetRoomInfo(const std::int32_t roomid,std::string& info);

    void AddRoomRecordInfo(const std::int32_t roomid, const std::string& info);
    void RemoveRoomRecordInfo(const std::int32_t roomid);
    bool GetRoomRecordInfo(const std::int32_t roomid, std::string& info);

    void AddPlayerStatus(const uid_type mid, const std::int32_t game_session);
    void RemovePlayerStatus(const uid_type mid);
	
    void UpdateRoomsList(const std::string & type, const std::string & info, const std::int32_t sid);
	void UpdateRoom(std::int32_t roomid, const std::string & info);

    void UpdateCreatorList(const std::string& list);
    bool GetCreatorList(std::string& list);

    void UpdateCreatorRoomList(const uid_type mid,const std::string& list);
    bool GetCreatorRoomList(const uid_type mid,std::string& list);
    void DeleteCreatorRoomList(const uid_type mid);

    void UpdateRoomData(const uid_type mid, std::int32_t roomid, const std::string& list);
    bool GetRoomData(const uid_type mid, std::int32_t roomid, std::string& list);
    void DeleteRoomData(const uid_type mid, std::int32_t roomid);

    bool GetRopen(const uid_type mid,std::string& data);

    void removeKey(const std::string& key) const;

    std::int32_t GetUserProps(const uid_type uid, Props_type & props);

	std::int32_t GetFriends(const uid_type mid, std::set<uid_type> & friends);

	std::int32_t GetPokerFriends(const uid_type mid, std::set<uid_type> & friends);

	std::int32_t DeletePokerFriend(const uid_type mid, const uid_type fmid);

	std::int32_t AddPokerFriend(const uid_type mid, const uid_type fmid);

	std::int32_t GetLevelCfg(LevelItems_type & cfg);

	std::int32_t GetRank(const uid_type mid, std::int32_t & win_rank, std::int32_t & riches_rank);

	std::int32_t UpdateAwardPool(chips_type award, chips_type & award_pool);

	void GetAward(std::int32_t thousand, chips_type & award, chips_type & award_pool, chips_type & old_award_pool);

	void LockGold(const uid_type mid);

	void UnlockGold(const uid_type mid);

	std::int32_t GetRoomGiftCfg( RoomGiftCfg & cfgs );

	std::int32_t GetLoginIP( uid_type mid, std::string & login_ip );

	void GetDayData( const uid_type mid, std::string & data);

	void UpdateDayData( const uid_type mid, const std::string & data );

    std::int32_t RedisSet(const std::string &  key, const std::string & value);

    std::int32_t RedisGet(const std::string &  key, std::string & value);

    std::int32_t RedisGetKeys(std::vector<std::string > &  key, std::vector<std::string > & value);

    std::int32_t RedisDel(const std::string &  key);

    std::int32_t RedisIncr(const std::string &  key, const std::int32_t incr, std::int32_t & amount);

	std::int32_t IncrPoints(std::int32_t mid, const std::int32_t incr, std::int32_t & value);

	std::int32_t GetPoints(std::int32_t mid, std::string & value);

	std::int32_t StartGoldStat(chips_type carry_gold);

	std::int32_t GetPlayerGameInfo(uid_type mid, MemberGame & info, bool forcedflush = false);

	std::int32_t GetCommonGameInfo(uid_type mid, MemberCommonGame & info, bool forcedflush = false);

	std::int32_t GetCommonGameInfoFromDB(uid_type mid, json_spirit::Value & array);

	std::int32_t GetPlayerBaseInfo(uid_type mid, std::string  & json_str, MemberFides & info);

    void set_player_gps(const uid_type mid, const std::string& data);
    std::string player_gps(const uid_type mid);

private:
	std::int32_t InitMemcachedAgent();

	std::int32_t GetUserPropsFromCache(const uid_type mid, Props_type & props);

	std::int32_t GetUserPropsFromDB(const uid_type uid, Props_type & props);


	std::int32_t GetMemberGameFromDB(uid_type mid, json_spirit::Value & array);

	std::int32_t GetGameInfoFromCache(uid_type mid, MemberGame & info);

	std::int32_t SyncGameDataToCache(const uid_type mid, const json_spirit::Value & json_value);

	std::int32_t GetCommonGameInfoFromCache(uid_type mid, MemberCommonGame & info);

	std::int32_t SetCommonGameInfoToCache(const uid_type mid, const json_spirit::Value & json_value);

    
private:
	Database * databaseconnector_ = nullptr;

	cpp_redis::Requestor<cpp_redis::REQUESTOR > * redisconnector_ = nullptr;

	boost::asio::io_service * engine_ = nullptr;

	std::shared_ptr< IMemcacheHandler > memcached_agent_;

	std::string cfg_database_;

	std::string local_config_;

	std::string db_prefix_;
};

#endif //_XPOKER_DB_DATALAYER_H_


