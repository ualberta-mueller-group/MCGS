#pragma once

#include "cgt_basics.h"
#include "sumgame.h"
#include <string>
#include "throw_assert.h"

/* TODO (in order)

    - Review this design, it's a bit messy...
        - Do these types need better encapsulation? (esp. the union type?)
    - impartial_sumgame
        - Give search function a better name
        - Give it a timeout


    For "union" type: use an actual union internally, with an enum indicating
        the real type

    The enums should have their own to_string functions

    Input needs to be validated somehow so that file_parser can print
        proper error messages...

*/

enum search_value_type_t
{
    SEARCH_VALUE_TYPE_NONE = 0,
    SEARCH_VALUE_TYPE_WINLOSS,
    SEARCH_VALUE_TYPE_NIMBER,
};

struct search_value
{
    std::string str() const;
    bool operator==(const search_value& rhs) const;
    bool operator!=(const search_value& rhs) const;

    search_value_type_t value_type;
    bool value_win;
    int value_nimber;
};

enum test_status_t
{
    TEST_STATUS_TIMEOUT = 0,
    TEST_STATUS_PASS,
    TEST_STATUS_FAIL,
    TEST_STATUS_COMPLETE,
};

struct search_result
{
public:
    std::string player_string() const;
    std::string status_string() const;
    std::string value_string() const;
    std::string duration_string() const;

    ebw player;
    test_status_t status;
    search_value value;
    double duration;
};


search_result search_partizan(const sumgame& sum,
    unsigned long long timeout = 0, 
    const search_value* expected_value = nullptr);

// TODO
//search_result search_partizan(const std::vector<game*>,
//    bw player,
//    unsigned long long timeout = 0, 
//    const search_value* expected_value = nullptr);

search_result search_impartial(const sumgame& sum,
    unsigned long long timeout = 0, 
    const search_value* expected_value = nullptr);

// TODO
//search_result search_impartial(const sumgame& sum,
//    ebw player,
//    unsigned long long timeout = 0, 
//    const search_value* expected_value = nullptr);
