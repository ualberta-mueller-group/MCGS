/*
    Initializes global variables/state.

    Must be called near the start of the program.

    Global/static variables must be initialized through some "init" function
    called from mcgs_init_all(), to avoid "static initialization order fiasco"
    problems.
*/
#pragma once
#include "cli_options.h"

void mcgs_init_1();

void mcgs_init_2(const cli_options& opts);
void mcgs_init_2();
