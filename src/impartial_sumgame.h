//---------------------------------------------------------------------------
// Algorithms to solve sums of impartial games.
//
// Uses standard sumgame class, with all subgames of type
// impartial_game
//
// Evaluate each impartial game in sum to a nim_value
//
// At each point in time,
// a sumgame may contain a mix of solved (nim_value known) 
// and unsolved subgames
// All subgames solved so far are combined into a single nim_value

#pragma once

#include "sumgame.h"

// solve sumgame s - compute its nim_value
int search_sumgame(const sumgame& s);
