#pragma once

#include "ThGraph.h"
#include "sumgame.h"
#include "database.h"


extern unsigned int max_thermograph_generation_depth;
ThGraph* db_make_thermograph(database& db, sumgame& sum, unsigned int depth);
