#include "test_case_enums.h"
#include "throw_assert.h"

std::string command_type_to_string(command_type_enum command_type)
{
    switch (command_type)
    {
        case COMMAND_TYPE_SOLVE_BW:
            return "Solve BW";
        case COMMAND_TYPE_SOLVE_N:
            return "Solve N";
        case COMMAND_TYPE_WINNING_MOVES:
            return "Winning Moves";
    }

    THROW_ASSERT(false);
}

std::string test_case_status_to_string(test_case_status_enum test_case_status)
{
    switch (test_case_status)
    {
        case TEST_CASE_STATUS_TIMEOUT:
            return "TIMEOUT";
        case TEST_CASE_STATUS_FAIL:
            return "FAIL";
        case TEST_CASE_STATUS_PASS:
            return "PASS";
        case TEST_CASE_STATUS_COMPLETED:
            return "COMPLETED";
    }

    THROW_ASSERT(false);
}

test_case_status_enum evaluate_test_case_status(
    const std::optional<std::string>& result,
    const std::optional<std::string>& expected_result)
{
    if (!result.has_value())
        return TEST_CASE_STATUS_TIMEOUT;

    if (!expected_result.has_value())
        return TEST_CASE_STATUS_COMPLETED;

    if (result.value() == expected_result.value())
        return TEST_CASE_STATUS_PASS;

    return TEST_CASE_STATUS_FAIL;
}
