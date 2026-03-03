#pragma once

#include "sumgame.h"
#include "dominated_moves.h"

std::shared_ptr<dominated_moves_t> db_make_dominated_moves(const sumgame& sum);
