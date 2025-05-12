#pragma once
/*
    Utilities for solving both partizan and impartial sums. Used by main.cpp
        and autotests.cpp to run tests.
*/

#include <string>
#include <vector>
#include "sumgame.h"
#include "game.h"

////////////////////////////////////////////////// search_value
enum search_value_type_t
{
    SEARCH_VALUE_TYPE_NONE = 0, // i.e. timeout
    SEARCH_VALUE_TYPE_WINLOSS,
    SEARCH_VALUE_TYPE_NIMBER,
};

class search_value
{
public:
    search_value();

    // getters
    std::string str() const;
    search_value_type_t type() const;

    // value getters (must check type() first)
    bool win() const;
    int nimber() const;

    // setters
    void set_none();
    void set_win(bool new_win);
    void set_nimber(int new_nimber);

private:
    search_value_type_t _type;

    bool _value_win;
    int _value_nimber;
};

////////////////////////////////////////////////// test_status_t
enum test_status_t
{
    TEST_STATUS_TIMEOUT = 0,
    TEST_STATUS_PASS,
    TEST_STATUS_FAIL,
    TEST_STATUS_COMPLETE,
};

std::string test_status_to_string(test_status_t status);

////////////////////////////////////////////////// search_result
struct search_result
{
    std::string player_str() const;
    std::string value_str() const;
    std::string status_str() const;
    std::string duration_str() const;

    ebw player;
    search_value value;
    test_status_t status;
    double duration;
};

////////////////////////////////////////////////// search functions
search_result search_partizan(const sumgame& sum, const search_value* expected_value = nullptr, unsigned long long timeout = 0);
search_result search_partizan(const std::vector<game*>& games, bw to_play, const search_value* expected_value = nullptr, unsigned long long timeout = 0);

