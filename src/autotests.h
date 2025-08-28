/*
    Invoked by ./MCGS --run-tests
*/
#pragma once
#include <string>

void run_autotests(const std::string& test_directory,
                   const std::string& outfile_name,
                   unsigned long long test_timeout);

void run_autotests_stdin(const std::string& outfile_name,
                         unsigned long long test_timeout);
