#pragma once

////////////////////////////////////////////////// enums
enum command_type_enum
{
    COMMAND_TYPE_SOLVE_BW = 0,
    COMMAND_TYPE_SOLVE_N,
    COMMAND_TYPE_WINNING_MOVES,
};

enum test_case_status_enum
{
    TEST_CASE_STATUS_TIMEOUT = 0,
    TEST_CASE_STATUS_FAIL,
    TEST_CASE_STATUS_PASS,
    TEST_CASE_STATUS_COMPLETED,
};
