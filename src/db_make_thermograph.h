#pragma once

#include "ThGraph.h"
#include "sumgame.h"
#include "database.h"

ThGraph* db_make_thermograph(database& db, sumgame& sum,
                             const db_gen_options_t& gen_opts);
