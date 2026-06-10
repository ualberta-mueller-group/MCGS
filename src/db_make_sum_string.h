/*
    Used when DB_INCLUDE_STRINGS is defined, to put text representations of
    sums into DB entries, for use with --dump-db.
*/
#pragma once

#include <sstream>
#include <string>

#include "sumgame.h"

inline std::string db_make_sum_string(const sumgame& sum)
{
    std::stringstream str;
    sum.print_sorted(str);
    return str.str();
}
