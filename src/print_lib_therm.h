#pragma once

#include "ThGraph.h"

// reimplemented because `ThGraph.h`'s implementation prints newlines
inline void print_thermograph(std::ostream& os, const ThGraph& graph)
{
    os << "Left Scaffold: " << *graph.Sc(SG_BLACK);
    os << " Right Scaffold: " << *graph.Sc(SG_WHITE);
}
