#pragma once

#include <vector>
#include <string>

#include "cgt_basics.h"
#include "sumgame.h"

std::vector<std::string> get_winning_moves_for_player(
    sumgame& sum, bw player);
