#pragma once
#include <tuple>

#include "bounds.h"
#include "database.h"
#include "sumgame.h"

std::optional<std::tuple<bound_scale, game_bounds_ptr>> db_make_bounds(
    database& db, sumgame& sum);
