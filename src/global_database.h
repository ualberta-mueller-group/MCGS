/*
    Globally accessible instance of the database class, available after
    mcgs_init_all() runs
*/
#pragma once

// IWYU pragma: begin_exports
#include "database.h"
// IWYU pragma: end_exports

////////////////////////////////////////////////// global database functions
database& get_global_database();

void init_global_database();
