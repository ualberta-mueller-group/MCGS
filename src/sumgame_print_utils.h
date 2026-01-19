#pragma once

#include <string>

#include "sumgame.h"
#include "cgt_basics.h"

std::string sumgame_move_to_string(const sumgame& sum, const sumgame_move& sm,
                                   ebw player, bool with_subgame_idx);
