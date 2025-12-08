#pragma once

////////////////////////////////////////////////// enums
enum test_case_type_enum
{
    TEST_CASE_TYPE_BW_SOLVE = 0,
    TEST_CASE_TYPE_N_SOLVE,
    TEST_CASE_TYPE_WINNING_MOVES,
};

enum test_case_status_enum
{
    TEST_CASE_STATUS_TIMEOUT = 0,
    TEST_CASE_STATUS_FAIL,
    TEST_CASE_STATUS_PASS,
    TEST_CASE_STATUS_COMPLETED,
};
