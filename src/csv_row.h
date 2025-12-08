#pragma once

#include <optional>
#include <string>
#include <cstdint>
#include "test_case_enums.h"

/*
   Comments indicate where each field might be filled in
*/

struct csv_row
{
    std::optional<std::string> file; // autotests.cpp
    std::optional<int> case_number; // autotests.cpp
    std::optional<std::string> games; // test_case construction
    std::optional<std::string> player; // test_case construction
    std::optional<std::string> expected_result; // test_case construction
    std::optional<std::string> result; // _run_impl()
    std::optional<double> time_ms; // _run_impl()
    std::optional<uint64_t> node_count; // _run_impl() (helper)
    std::optional<uint64_t> unique_node_count; // _run_impl() (helper)
    std::optional<test_case_status_enum> status; // _run_impl()
    std::optional<std::string> comments; // test_case construction
    std::optional<command_type_enum> command_type; // test_case construction
};
