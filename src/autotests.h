/*
    Invoked by ./MCGS --run-tests
*/
#pragma once
#include "test_filter.h"
#include <string>

void run_autotests(const std::string& test_directory,
                   const std::string& outfile_name,
                   unsigned long long test_timeout,
                   test_filter_enum filter_type);
