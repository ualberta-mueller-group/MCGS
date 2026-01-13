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

enum minimax_outcome_enum
{
    MINIMAX_OUTCOME_NONE = 0,
    MINIMAX_OUTCOME_WIN,
    MINIMAX_OUTCOME_LOSS,
};

enum test_case_status_enum
{
    TEST_CASE_STATUS_TIMEOUT = 0,
    TEST_CASE_STATUS_FAIL,
    TEST_CASE_STATUS_PASS,
    TEST_CASE_STATUS_COMPLETED,
};

////////////////////////////////////////////////// enum to string functions
std::string command_type_to_string(command_type_enum command_type);
std::string minimax_outcome_to_string(minimax_outcome_enum minimax_outcome);
std::string test_case_status_to_string(test_case_status_enum test_case_status);

////////////////////////////////////////////////// other utilities
test_case_status_enum evaluate_test_case_status(
    const std::optional<std::string>& result,
    const std::optional<std::string>& expected_result);
