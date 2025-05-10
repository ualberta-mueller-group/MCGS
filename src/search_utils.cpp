#include "search_utils.h"

#include <chrono>
#include <ratio>
#include "throw_assert.h"
#include "impartial_sumgame.h"

using namespace std;

namespace {
test_status_t get_test_status(const search_value* real_value, const search_value* expected_value)
{
    assert(real_value != nullptr);

    if (real_value->value_type == SEARCH_VALUE_TYPE_NONE)
        return TEST_STATUS_TIMEOUT;

    if (expected_value == nullptr)
        return TEST_STATUS_COMPLETE;

    return (*real_value == *expected_value) ? TEST_STATUS_PASS : TEST_STATUS_FAIL;
}
} // namespace

//////////////////////////////////////////////////
search_result search_partizan(const sumgame& sum,
    unsigned long long timeout,
    const search_value* expected_value)
{
    search_result result;
    result.player = sum.to_play();

    chrono::time_point start = chrono::high_resolution_clock::now();
    std::optional<solve_result> sr = sum.solve_with_timeout(timeout);
    chrono::time_point end = chrono::high_resolution_clock::now();
    chrono::duration<double, std::milli> duration = end - start;
    result.duration = duration.count();

    if (sr.has_value())
    {
        result.value.value_type = SEARCH_VALUE_TYPE_WINLOSS;
        result.value.value_win = sr->win;
    }
    else
        result.value.value_type = SEARCH_VALUE_TYPE_NONE;

    result.status = get_test_status(&result.value, expected_value);

    return result;
}

search_result search_impartial(const sumgame& sum,
    unsigned long long timeout,
    const search_value* expected_value)
{
    search_result result;
    result.player = EMPTY;

    chrono::time_point start = chrono::high_resolution_clock::now();
    int nim_value = search_sumgame(sum);
    chrono::time_point end = chrono::high_resolution_clock::now();
    chrono::duration<double, std::milli> duration = end - start;
    result.duration = duration.count();

    result.value.value_type = SEARCH_VALUE_TYPE_NIMBER;
    result.value.value_nimber = nim_value;

    result.status = get_test_status(&result.value, expected_value);

    return result;
}
