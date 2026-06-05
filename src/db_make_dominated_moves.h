#pragma once

#include "database.h"
#include "sumgame.h"

void db_make_dominated_moves(const sumgame& sum, db_entry_partisan& entry, database& db);
