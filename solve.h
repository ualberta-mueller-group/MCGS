//---------------------------------------------------------------------------
// Solve combinatorial games - find winner
//---------------------------------------------------------------------------
#ifndef solve_H
#define solve_H

#include "cgt_basics.h"
#include "alternating_move_game.h"

// True means that g.to_play() wins,
// false means g.to_play() loses,
// with best play by both
bool solve(alternating_move_game& g);

#endif // solve_H
