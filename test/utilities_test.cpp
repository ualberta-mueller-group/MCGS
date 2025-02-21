#include "utilities_test.h"
#include "cgt_basics.h"
#include "utilities.h"
#include "limits"

#include <iostream>
using namespace std;

namespace {

void test_split_string()
{
    vector<tuple<string, vector<string>>> test_cases
    {
        {"", {}},
        {" ", {}},
        {"abc", {"abc"}},
        {"\t \n a\n b   c de  ", {"a", "b", "c", "de"}}
    };

    for (const tuple<string, vector<string>>& test : test_cases)
    {
        const string& str = get<0>(test);
        const vector<string>& expected = get<1>(test);

        vector<string> result = split_string(str);

        assert(result == expected);
    }
}

void test_is_int()
{
    vector<tuple<string, bool>> test_cases
    {
        {"", false},
        {" ", false},

        {"0", true},
        {" 0", false},
        {"0 ", false},

        {"-0", true},
        {" -0", false},
        {"-0 ", false},

        {"+0", false},
        {"+1", false},
        {"--1", false},
        {"5-", false},
        {"2.5", false},

        {"05", true},
        {"-05", true},

        {"21", true},
        {"-25", true},
        {"-6", true},
    };

    for (const tuple<string, bool>& test : test_cases)
    {
        const string& str = get<0>(test);
        const bool& expected = get<1>(test);

        assert(is_int(str) == expected);
    }
}

void test_addition_will_wrap()
{
    const int32_t& min = numeric_limits<int32_t>::min();
    const int32_t& max = numeric_limits<int32_t>::max();

    vector<tuple<int32_t, int32_t, bool>> test_cases
    {
        {min, -1, true},
        {min, 0, false},
        {min, 1, false},

        {max, -1, false},
        {max, 0, false},
        {max, 1, true},

        {max / 2, max / 2, false},
        {min / 2, min / 2, false},

        {min, max, false},
    };

    for (const tuple<int32_t, int32_t, bool>& test : test_cases)
    {
        const int32_t& x1 = get<0>(test);
        const int32_t& x2 = get<1>(test);
        const bool& expected = get<2>(test);

        assert(addition_will_wrap(x1, x2) == expected);
        assert(addition_will_wrap(x2, x1) == expected);
    }
}

void test_relation_from_search_results()
{
    vector<tuple<bool, bool, bool, bool, relation>> test_cases
    {
        {0, 0, 0, 0, REL_UNKNOWN},
        {0, 0, 0, 1, REL_UNKNOWN},
        {0, 0, 1, 0, REL_UNKNOWN},
        {0, 0, 1, 1, REL_GREATER_OR_EQUAL},
        {0, 1, 0, 0, REL_UNKNOWN},
        {0, 1, 0, 1, REL_UNKNOWN},
        {0, 1, 1, 0, REL_UNKNOWN},
        {0, 1, 1, 1, REL_GREATER_OR_EQUAL},
        {1, 0, 0, 0, REL_UNKNOWN},
        {1, 0, 0, 1, REL_UNKNOWN},
        {1, 0, 1, 0, REL_FUZZY},
        {1, 0, 1, 1, REL_GREATER},
        {1, 1, 0, 0, REL_LESS_OR_EQUAL},
        {1, 1, 0, 1, REL_LESS_OR_EQUAL},
        {1, 1, 1, 0, REL_LESS},
        {1, 1, 1, 1, REL_EQUAL},
    };

    for (const tuple<bool, bool, bool, bool, relation>& test : test_cases)
    {
        const bool& le_known = get<0>(test);
        const bool& is_le = get<1>(test);
        const bool& ge_known = get<2>(test);
        const bool& is_ge = get<3>(test);
        const relation& expected = get<4>(test);

        assert(relation_from_search_results(le_known, is_le, ge_known, is_ge) == expected);
    }
}

} // namespace

void utilities_test_all()
{
    test_split_string();
    test_is_int();
    test_addition_will_wrap();
    test_relation_from_search_results();
}
