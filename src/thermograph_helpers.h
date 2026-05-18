#pragma once

#include "ThGraph.h"
#include "hashing.h"

// reimplemented because `ThGraph.h`'s implementation prints newlines
inline void print_thermograph(std::ostream& os, const ThGraph& graph)
{
    os << "Left Scaffold: " << *graph.Sc(SG_BLACK);
    os << " Right Scaffold: " << *graph.Sc(SG_WHITE);
}

bool thermograph_bends_out_below_zero(const ThGraph& graph, bool left);

hash_t get_thermograph_hash(const ThGraph& graph);

bool game_is_small_from_thermograph(const ThGraph& graph);
