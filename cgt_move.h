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
const int COLOR_BIT = 2 * BITS_PER_MOVE_PART;
const int WHITE_MASK = (1 << COLOR_BIT); // bit 30 is set
const int MOVE_MASK = WHITE_MASK - 1; // bits 0..29 are set
const int UNUSED_BIT = (1 << 31);
const int MOVE_MAX_SIZE = (1 << BITS_PER_MOVE_PART);
static_assert(sizeof(int) >= 4);

inline int get_color(move m) // BLACK = 0, WHITE = 1
{
    return m >> (COLOR_BIT);
}

inline move decode(move m) // remove color bit
{
    return m & MOVE_MASK;
}

inline move encode(move m, int color) // add color bit
{
    assert(m < WHITE_MASK);
    assert_black_white(color);
    return m + color*WHITE_MASK;
}

inline move two_part_move(int first, int second)
{
    assert_range(first, 0, MOVE_MAX_SIZE);
    assert_range(second, 0, MOVE_MAX_SIZE);
    return first * MOVE_MAX_SIZE + second;
}

inline int first(move m)
{
    return m / MOVE_MAX_SIZE;
}

inline int second(move m)
{
    return m % MOVE_MAX_SIZE;
}

inline int from(move m)
{
    return first(m);
}

inline int to(move m)
{
    return second(m);
}

} // namespace cgt_move

#endif // cgt_move_H
