#ifndef PLAYER_MINGTANG_H
#define PLAYER_MINGTANG_H

struct PlayerMingTang
{
    std::uint32_t spring_times = 0;           //春天次数

    std::uint32_t bomb_times = 0;         //炸弹总次数
    std::uint32_t bomb_times_one = 0;     //1个炸弹的次数；
    std::uint32_t bomb_times_two = 0;     //2个炸弹的次数；
    std::uint32_t bomb_times_n = 0;       //2个以上炸弹次数；
};

#endif // PLAYER_MINGTANG_H
