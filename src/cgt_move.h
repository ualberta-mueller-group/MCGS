//---------------------------------------------------------------------------
// Move utilities
// encode/decode move + color bit
// Two part move representation
// Print move coordinate(s)
//---------------------------------------------------------------------------

#pragma once

#include <string>
#include "cgt_basics.h"

// Currently all moves must be encoded as int, and decoded from int
// There is no abstract move class here.
typedef int move;
static_assert(sizeof(move) >= 4, "move should be at least 32 bits");

namespace cgt_move {

const int BITS_PER_MOVE_PART = 15;
const int COLOR_BIT = 31;
const int SIGN_BIT = 30;
const unsigned int WHITE_MASK = (1 << COLOR_BIT); // color bit 31 is set
const unsigned int SIGN_MASK = (1 << SIGN_BIT);   // sign bit 30 is set
const int MOVE_MASK = WHITE_MASK - 1;             // bits 0..30 are set
const int MOVE_MAX_SIZE = (1 << BITS_PER_MOVE_PART);
const int MOVE_BITS = MOVE_MAX_SIZE - 1;
static_assert(sizeof(int) >= 4);

inline int get_bit(move m, int bit)
{
    return (m >> bit) & 1;
}

inline int set_bit(move m, int bit)
{
    return m | (1 << bit);
}

inline bw get_color(move m) // BLACK = 0, WHITE = 1
{
    return get_bit(m, COLOR_BIT);
}

inline int get_sign(move m)
{
    return (m & SIGN_MASK) ? -1 : 1;
}

inline move decode(move m) // remove color bit
{
    return m & MOVE_MASK;
}

inline move encode(move m, bw color) // add color bit
{
    // Before casting (to fix compiler warnings), check that casting is OK
    static_assert(sizeof(WHITE_MASK) == sizeof(const unsigned int));
    static_assert(sizeof(m) == sizeof(const unsigned int));
    const unsigned int& m_unsigned = reinterpret_cast<const unsigned int&>(m);

    // assert(m < WHITE_MASK);
    assert(m_unsigned < WHITE_MASK);
    assert_black_white(color);
    return m + color * WHITE_MASK;
}

inline move two_part_move(int first, int second)
{
    assert_range(first, -MOVE_MAX_SIZE, MOVE_MAX_SIZE);
    int sign_bit = 0;
    if (first < 0)
    {
        first = -first;
        sign_bit = 1;
    }
    assert_range(second, 0, MOVE_MAX_SIZE);
    return (sign_bit << SIGN_BIT) + first * MOVE_MAX_SIZE + second;
}

inline int first(move m)
{
    return get_sign(m) * ((m >> BITS_PER_MOVE_PART) & MOVE_BITS);
}

inline int second(move m)
{
    return m & MOVE_BITS;
}

inline int from(move m)
{
    return first(m);
}

inline int to(move m)
{
    return second(m);
}

inline int decode3(move m, int* q, bw* player)
{
    *player = get_color(m);
    *q = second(m);
    return first(m);
}

inline move encode3(int first, int second, bw color)
{
    return encode(two_part_move(first, second), color);
}

std::string print(const move& m);
std::string print_two_part_move(const move& m);

} // namespace cgt_move
