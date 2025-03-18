#include "cgt_move.h"
#include <cassert>

namespace cgt_move {

void test_two_part_move()
{
    {
        const int part1 = 123;
        const int part2 = 456;
        const move m = two_part_move(part1, part2);
        assert(first(m) == part1);
        assert(second(m) == part2);
    }
    {
        const int part1 = MOVE_MAX_SIZE - 1;
        const int part2 = MOVE_MAX_SIZE - 1;
        const move m = two_part_move(part1, part2);
        assert(first(m) == part1);
        assert(second(m) == part2);
    }
    {
        const int part1 = 0;
        const int part2 = 0;
        const move m = two_part_move(part1, part2);
        assert(first(m) == part1);
        assert(second(m) == part2);
    }
}

void test_encode_decode()
{
    int mm = MOVE_MAX_SIZE - 1;
    const move m = two_part_move(mm, mm);
    const move m2 = encode(m, BLACK);
    assert(decode(m2) == m);
    assert(get_color(m2) == BLACK);
    const move m3 = encode(m, WHITE);
    assert(decode(m3) == m);
    assert(get_color(m3) == WHITE);
}

void test_encode_decode2()
{
    int mm = (MOVE_MAX_SIZE - 1) / 2;
    const move m = two_part_move(mm, mm);
    const move m2 = encode(m, BLACK);
    assert(decode(m2) == m);
    assert(get_color(m2) == BLACK);
    const move m3 = encode(m, WHITE);
    assert(decode(m3) == m);
    assert(get_color(m3) == WHITE);
}

void test_encode_decode3()
{
    const move m = two_part_move(-5, 8);
    const move m2 = encode(m, BLACK);
    assert(decode(m2) == m);
    assert(get_color(m2) == BLACK);
    const move m3 = encode(m, WHITE);
    assert(decode(m3) == m);
    assert(get_color(m3) == WHITE);
}

void test_encode_decode4()
{
    const move m = two_part_move(1, 2);
    const move m2 = encode(m, BLACK);
    assert(decode(m2) == m);
    assert(get_color(m2) == BLACK);
    const move m3 = encode(m, WHITE);
    assert(decode(m3) == m);
    assert(get_color(m3) == WHITE);
}
} // namespace cgt_move

void cgt_move_test_all()
{
    cgt_move::test_two_part_move();
    cgt_move::test_encode_decode();
    cgt_move::test_encode_decode2();
    cgt_move::test_encode_decode3();
    cgt_move::test_encode_decode4();
}
