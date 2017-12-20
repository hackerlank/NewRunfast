#ifndef _GAMELOGIC_H
#define _GAMELOGIC_H

#include "card_interface.h"

enum Ranking
{
  INVALID_RANKING = -1,
  NIN_None,
  NIN_Ace,
  NIN_Two,
  NIN_Three,
  NIN_Four,
  NIN_Five,
  NIN_Six,
  NIN_Seven,
  NIN_Eight,
  NIN_Nine,
  NIN_Nin,						//ţţ
  NIN_WHITE,					//��Ƥţ //white
  NIN_BOMB,					//����
  NIN_FULL,						//��ţ, �廨ţ
  NIN_SMALL					//��Сţ
};

class HandStrength
{
  friend class PokerStyleGameLogicImpl;
  friend class MaJiangrStyleGameLogicImpl;

private:
  bool operator < (const HandStrength &c) const = delete;
  bool operator == (const HandStrength &c) const = delete;

public:
  HandStrength() :id_(0), ranking_(NIN_None)
  {

  }

  Ranking getRanking() const { return ranking_; };

  void setId(int rid) { id_ = rid; };
  int getId() const { return id_; };

  void Clear()
  {
    id_ = 0;
    ranking_ = NIN_None;
    cards_.clear();
  }

  const Cards & GetRank()const
  {
    return cards_;
  }

private:
  int id_;

  Ranking ranking_;

  Cards cards_;
};

class Referee
{
protected:
  Referee() {}
  virtual ~Referee() {}

public:
  virtual void getStrength(const Cards & allcards, HandStrength & strength) const = 0;

  virtual bool getStrength(const Cards & holdcards, const Cards & commcards, HandStrength & strength) const = 0;

  // 0 , ���ڣ� -1�� С�ڣ� 1 ����
  virtual int Compare(const HandStrength & left, const HandStrength & right) const = 0;
};

bool CompareAdaptation(Referee * referee, const HandStrength & left, const HandStrength & right);

//GameLogic
Referee * CreateReferee(char room_type);


void SortByShowCards(const HandStrength & hs, Cards & cards);


#endif /* _GAMELOGIC_H */
