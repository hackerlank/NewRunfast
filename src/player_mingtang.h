#ifndef PLAYER_MINGTANG_H
#define PLAYER_MINGTANG_H

struct PlayerMingTang
{
    std::uint32_t spring_times = 0;           //�������

    std::uint32_t bomb_times = 0;         //ը���ܴ���
    std::uint32_t bomb_times_one = 0;     //1��ը���Ĵ�����
    std::uint32_t bomb_times_two = 0;     //2��ը���Ĵ�����
    std::uint32_t bomb_times_n = 0;       //2������ը��������
};

#endif // PLAYER_MINGTANG_H
