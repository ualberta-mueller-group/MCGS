#pragma once

#include <ostream>

#include "ThGraph.h"
#include "bounds.h"
#include "hashing.h"

// Reimplemented because `ThGraph.h`'s implementation prints newlines
void print_thermograph(std::ostream& os, const ThGraph& graph);

/*
    Get specified rational bound along BOUND_SCALE_DYADIC_RATIONAL, where the
    scale index represents a multiple of 1/8.

    For a game G and the returned left bound B, G <= B.
*/
bound_t read_rational_bound_from_thermograph(const ThGraph& graph, bool left);

// True IFF immediately below T=0, the graph points "outward" as T decreases
bool thermograph_bends_out_below_zero(const ThGraph& graph, bool left);

// Computes a "local hash" of a graph
hash_t get_thermograph_hash(const ThGraph& graph);

// NOTE: false if the game is 0!
bool game_is_small_from_thermograph(const ThGraph& graph);

// Result is on BOUND_SCALE_DYADIC_RATIONAL. Bounds may be relaxed due to
// imprecision resulting from the scale's resolution
game_bounds* make_rational_bounds_from_thermograph(const ThGraph& graph);
