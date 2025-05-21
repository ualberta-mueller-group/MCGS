#include "search_utils.h"

#include <string>
#include <vector>
#include "sumgame.h"
#include "impartial_sumgame.h"
#include "game.h"
#include "throw_assert.h"
#include <chrono>
#include <ratio>
#include <cassert>
#include <cstdio>
#include <stdexcept>

using namespace std;

////////////////////////////////////////////////// helpers
namespace {

test_status_t compare_search_values(const search_value* found_value,
                                    const search_value* expected_value)
{
    assert(found_value != nullptr);

    if (found_value->type() == SEARCH_VALUE_TYPE_NONE)
        return TEST_STATUS_TIMEOUT;

    if (                                                 //
        expected_value == nullptr ||                     //
        expected_value->type() == SEARCH_VALUE_TYPE_NONE //
        )                                                //
        return TEST_STATUS_COMPLETED;

    THROW_ASSERT(found_value->type() == expected_value->type());
    return *found_value == *expected_value ? TEST_STATUS_PASS
                                           : TEST_STATUS_FAIL;
}

} // namespace

////////////////////////////////////////////////// misc functions
string player_name_bw_imp(ebw to_play)
{
    if (is_black_white(to_play))
        return string(1, color_char(to_play));

    assert(to_play == EMPTY);
    return "IMP"; // impartial
}

////////////////////////////////////////////////// search_value
search_value::search_value()
    : _type(SEARCH_VALUE_TYPE_NONE), _value_win(false), _value_nimber(-1)
{
}

bool search_value::operator==(const search_value& rhs) const
{
    if (type() != rhs.type())
        return false;

    switch (type())
    {
        case SEARCH_VALUE_TYPE_NONE:
            return true;
        case SEARCH_VALUE_TYPE_WINLOSS:
            return win() == rhs.win();
        case SEARCH_VALUE_TYPE_NIMBER:
            return nimber() == rhs.nimber();
    }

    assert(false);
}

bool search_value::operator!=(const search_value& rhs) const
{
    return !(*this == rhs);
}

string search_value::str() const
{
    switch (type())
    {
        case SEARCH_VALUE_TYPE_NONE:
            return "None";
        case SEARCH_VALUE_TYPE_WINLOSS:
            return _value_win ? "Win" : "Loss";
        case SEARCH_VALUE_TYPE_NIMBER:
        {
            assert(_value_nimber >= 0);
            return "*" + to_string(_value_nimber);
        }
    }

    THROW_ASSERT(false);
}

search_value_type_t search_value::type() const
{
    return _type;
}

bool search_value::win() const
{
    THROW_ASSERT(type() == SEARCH_VALUE_TYPE_WINLOSS);
    return _value_win;
}

int search_value::nimber() const
{
    THROW_ASSERT(type() == SEARCH_VALUE_TYPE_NIMBER);
    assert(_value_nimber >= 0);
    return _value_nimber;
}

void search_value::set_none()
{
    _type = SEARCH_VALUE_TYPE_NONE;
}

void search_value::set_win(bool new_win)
{
    _type = SEARCH_VALUE_TYPE_WINLOSS;
    _value_win = new_win;
    _value_nimber = -1;
}

void search_value::set_nimber(int new_nimber)
{
    assert(new_nimber >= 0);

    _type = SEARCH_VALUE_TYPE_NIMBER;
    _value_nimber = new_nimber;
}

////////////////////////////////////////////////// test_status_t
string test_status_to_string(test_status_t status)
{
    switch (status)
    {
        case TEST_STATUS_TIMEOUT:
            return "TIMEOUT";
        case TEST_STATUS_PASS:
            return "PASS";
        case TEST_STATUS_FAIL:
            return "FAIL";
        case TEST_STATUS_COMPLETED:
            return "COMPLETED";
    }

    THROW_ASSERT(false);
}

////////////////////////////////////////////////// search_result
string search_result::player_str() const
{
    return player_name_bw_imp(player);
}

string search_result::value_str() const
{
    return value.str();
}

string search_result::status_str() const
{
    return test_status_to_string(status);
}

string search_result::duration_str() const
{
    const char* format = "%.2f";

    int size = snprintf(nullptr, 0, format, duration) + 1;
    // variable length arrays are not part of the C++ standard; use vector
    vector<char> buffer(size);

    int got_size = snprintf(buffer.data(), size, format, duration);
    assert(size == got_size + 1);

    return string(buffer.data());
}

////////////////////////////////////////////////// search functions
search_result search_partizan(const sumgame& sum,
                              const search_value* expected_value,
                              unsigned long long timeout)
{
    search_result result;

    chrono::time_point start = chrono::high_resolution_clock::now();
    optional<solve_result> sr = sum.solve_with_timeout(timeout);
    chrono::time_point end = chrono::high_resolution_clock::now();

    chrono::duration<double, std::milli> duration = end - start;

    // Assign values to search_result
    result.player = sum.to_play();

    if (sr.has_value())
        result.value.set_win(sr->win);
    else
        result.value.set_none();

    result.status = compare_search_values(&result.value, expected_value);

    result.duration = duration.count();

    return result;
}

search_result search_partizan(const vector<game*>& games, bw to_play,
                              const search_value* expected_value,
                              unsigned long long timeout)
{
    assert(is_black_white(to_play));

    sumgame sum(to_play);

    for (game* g : games)
        sum.add(g);

    return search_partizan(sum, expected_value, timeout);
}

search_result search_impartial(const sumgame& sum,
                               const search_value* expected_value,
                               unsigned long long timeout)
{
    if (!sum.impartial())
        throw std::logic_error("Sum contains partizan games");

    search_result result;

    chrono::time_point start = chrono::high_resolution_clock::now();
    std::optional<int> nim_value =
        search_impartial_sumgame_with_timeout(sum, timeout);
    chrono::time_point end = chrono::high_resolution_clock::now();

    chrono::duration<double, std::milli> duration = end - start;

    // Assign values to search_result
    result.player = EMPTY;

    if (nim_value.has_value())
        result.value.set_nimber(nim_value.value());
    else
        result.value.set_none();

    result.status = compare_search_values(&result.value, expected_value);

    result.duration = duration.count();

    return result;
}

search_result search_impartial(const std::vector<game*>& games,
                               const search_value* expected_value,
                               unsigned long long timeout)
{
    sumgame sum(BLACK);
    for (game* g : games)
        sum.add(g);

    return search_impartial(sum, expected_value, timeout);
}
