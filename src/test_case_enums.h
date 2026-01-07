#pragma once

#include <string>
#include <optional>

////////////////////////////////////////////////// enums
enum command_type_enum
{
    COMMAND_TYPE_SOLVE_BW = 0,
    COMMAND_TYPE_SOLVE_N,
    COMMAND_TYPE_WINNING_MOVES,
};

std::string command_type_to_string(command_type_enum command_type);

enum test_case_status_enum
{
    TEST_CASE_STATUS_TIMEOUT = 0,
    TEST_CASE_STATUS_FAIL,
    TEST_CASE_STATUS_PASS,
    TEST_CASE_STATUS_COMPLETED,
};

std::string test_case_status_to_string(test_case_status_enum test_case_status);

test_case_status_enum evaluate_test_case_status(
    const std::optional<std::string>& result,
    const std::optional<std::string>& expected_result);
