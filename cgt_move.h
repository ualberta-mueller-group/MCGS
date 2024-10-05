//---------------------------------------------------------------------------
// Move utilities
// encode/decode move + color bit
// Two part move representation
// Print move coordinate(s)
//---------------------------------------------------------------------------

#ifndef cgt_move_H
#define cgt_move_H

#include <vector>
#include "cgt_basics.h"



// Currently all moves must be encoded as int, and decoded from int
// There is no abstract move class here.
typedef int move;

namespace cgt_move {

const int BITS_PER_MOVE_PART = 15;
const int COLOR_BIT = 31;
const int SIGN_BIT = 30;
const unsigned int WHITE_MASK = (1 << COLOR_BIT); // bit 31 is set
const int MOVE_MASK = WHITE_MASK - 1; // bits 0..30 are set
const int MOVE_MAX_SIZE = (1 << BITS_PER_MOVE_PART);
const int MOVE_BITS = MOVE_MAX_SIZE - 1;
static_assert(sizeof(int) >= 4);

inline bw get_color(move m) // BLACK = 0, WHITE = 1
{
    return (m >> COLOR_BIT) & 1;
}

inline int get_sign(move m)
{
    return (m >> SIGN_BIT) ? -1 : 1;
}

inline move decode(move m) // remove color bit
{
    return m & MOVE_MASK;
}

inline move encode(move m, bw color) // add color bit
{
    assert(m < WHITE_MASK);
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

} // namespace cgt_move

#endif // cgt_move_H
