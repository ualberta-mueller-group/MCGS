#pragma once

#include "cgt_basics.h"
#include "database.h"
#include "sumgame.h"

outcome_class db_make_outcome_class(sumgame& sum, const db_entry_partisan& entry);
