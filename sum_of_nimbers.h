//---------------------------------------------------------------------------
// Sum of nimbers utilities
// Compute nim sum for sumgame where all subgames are
// of type nimber
//---------------------------------------------------------------------------

#pragma once

#include <vector>
#include "sumgame.h"

std::vector<int> get_nim_heaps(const sumgame& s);

bool static_solve(const sumgame& s);
