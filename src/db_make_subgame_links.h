#pragma once

#include "sumgame.h"
#include "database.h"

void db_make_subgame_links(const sumgame& sum, db_entry_partisan& entry,
                           database& db);
