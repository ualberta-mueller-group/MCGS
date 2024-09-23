//---------------------------------------------------------------------------
// Utility functions for unit tests
//---------------------------------------------------------------------------

#ifndef test_utilities_H
#define test_utilities_H

#include "cgt_move.h"
#include "game.h"

inline void assert_move(move_generator& mg, int from, int to)
{ 
    move m = mg.gen_move();
    assert(from == cgt_move::from(m));
    assert(to == cgt_move::to(m));
}

#endif // test_utilities_H
