#pragma once

#include "ThGraph.h"
#include "sumgame.h"
#include "database.h"
#include <memory> 

extern unsigned int max_thermograph_generation_depth;
std::shared_ptr<ThGraph> db_make_thermograph(database& db, sumgame& sum, unsigned int depth);
