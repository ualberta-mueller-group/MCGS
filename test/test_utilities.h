//---------------------------------------------------------------------------
// Utility functions for unit tests
//---------------------------------------------------------------------------

#ifndef test_utilities_H
#define test_utilities_H

#include "cgt_move.h"
#include "game.h"

inline void assert_move(move_generator& mg, int mv)
{ 
    const move m = mg.gen_move();
    assert(m == mv);
}

inline void assert_two_part_move(move_generator& mg, int from, int to)
{ 
    move m = mg.gen_move();
    assert(from == cgt_move::from(m));
    assert(to == cgt_move::to(m));
}

inline void assert_solve(game& pos, bw to_play, bool expected_result)
{
    alternating_move_game g(pos, to_play);
    const bool result = g.solve();
    assert(result == expected_result);
}

#endif // test_utilities_H
