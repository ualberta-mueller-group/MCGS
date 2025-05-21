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

// IWYU pragma: begin_exports
#include "sumgame.h"
#include <optional>
// IWYU pragma: end_exports

// solve sumgame s - compute its nim_value
int search_impartial_sumgame(const sumgame& s);

std::optional<int> search_impartial_sumgame_with_timeout(
    const sumgame& s, unsigned long long timeout);

void init_impartial_sumgame_ttable(size_t idx_bits);
