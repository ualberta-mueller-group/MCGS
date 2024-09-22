//---------------------------------------------------------------------------
// Move utilities
// Two part move representation
// Print move coordinate(s)
//---------------------------------------------------------------------------

#ifndef cgt_move_H
#define cgt_move_H

#include <vector>
#include "cgt_basics.h"

namespace cgt_move {

const int MOVE_MAX_SIZE = (1 << 16);
static_assert(sizeof(int) >= 4);

inline move two_part_move(int first, int second)
{
    assert(first < MOVE_MAX_SIZE);
    assert(second < MOVE_MAX_SIZE);
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

#endif // cgt_two_part_move_H
