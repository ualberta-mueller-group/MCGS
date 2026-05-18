#pragma once

#include "bounds.h"
#include "database.h"
#include "sumgame.h"

std::shared_ptr<game_bounds> db_make_bounds(const database& db, sumgame& sum,
                                            const db_entry_partisan& entry);
