#include "utilities_test.h"
#include "cgt_basics.h"
#include "utilities.h"
#include <cstdint>
#include <sstream>
#include <tuple>

using namespace std;

namespace {
void test_size_in_bits()
{
    assert(size_in_bits<int>() == size_in_bits(int(0)));
    assert(size_in_bits<int>() == sizeof(int) * CHAR_BIT);

    assert(size_in_bits<uint64_t>() == size_in_bits(uint64_t(0)));
    assert(size_in_bits<uint64_t>() == sizeof(uint64_t) * CHAR_BIT);

    assert(size_in_bits<uint8_t>() == size_in_bits(uint8_t(0)));
    assert(size_in_bits<uint8_t>() == sizeof(uint8_t) * CHAR_BIT);
}

void test_print_bits()
{
    typedef tuple<uint8_t, string> test_case_t;
    /*
       input
       expected output
    */

    // clang-format off
    vector<test_case_t> test_cases
    {
        {0, "00000000"},
        {1, "00000001"},
        {2, "00000010"},
        {3, "00000011"},
        {4, "00000100"},
        {5, "00000101"},
        {6, "00000110"},
        {7, "00000111"},
        {8, "00001000"},
        {254, "11111110"},
        {255, "11111111"},
    };
    // clang-format on

    for (const test_case_t& test : test_cases)
    {
        const uint8_t& input = get<0>(test);
        const string& expected = get<1>(test);

        stringstream stream;
        print_bits(stream, input);
        assert(stream.str() == expected);
    }
}

void test_split_string()
{
    typedef tuple<string, vector<string>> test_case_t;
    /*
        input
        expected output
    */

    // clang-format off
    vector<test_case_t> test_cases
    {
        {"", {}},
        {" ", {}},
        {"abc", {"abc"}},
        {"\t \n a\n b   c de  ", {"a", "b", "c", "de"}}
    };
    // clang-format on

    for (const test_case_t& test : test_cases)
    {
        const string& str = get<0>(test);
        const vector<string>& expected = get<1>(test);

        vector<string> result = split_string(str);
        assert(result == expected);
    }
}

void test_is_int()
{
    typedef tuple<string, bool> test_case_t;
    /*
       input
       expected output
    */

    // clang-format off
    vector<test_case_t> test_cases
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
    // clang-format on

    for (const test_case_t& test : test_cases)
    {
        const string& str = get<0>(test);
        const bool& expected = get<1>(test);

        assert(is_int(str) == expected);
    }
}

void test_string_starts_ends_with()
{
    typedef tuple<string, string, bool, bool> test_case_t;
    /*
        string
        word
        expected string_starts_with() result
        expected string_ends_with() result
    */

    // clang-format off
    vector<test_case_t> test_cases
    {
        {"abcd", "abcd", true, true},

        {"abcd", "", true, true},
        {"", "", true, true},
        {"", "a", false, false},

        {"abcd ", "abcd", true, false},
        {" abcd", "abcd", false, true},
        {" abcd ", "abcd", false, false},

        {"abc", "abcd", false, false},
        {"abc", "b", false, false},
        {"abc", "d", false, false},
    };
    // clang-format on

    for (const test_case_t& test : test_cases)
    {
        const string& str = get<0>(test);
        const string& word = get<1>(test);
        const bool& exp_starts_with = get<2>(test);
        const bool& exp_ends_with = get<3>(test);

        assert(string_starts_with(str, word) == exp_starts_with);
        assert(string_ends_with(str, word) == exp_ends_with);
    }
}

void test_is_power_of_2()
{
    typedef tuple<uint8_t, bool> test_case_t;
    /*
       input
       expected result
    */

    // clang-format off
    vector<test_case_t> test_cases
    {
        {0, false},
        {1, true},
        {2, true},
        {3, false},
        {4, true},
        {5, false},
        {6, false},
        {7, false},
        {8, true},
        {63, false},
        {64, true},
    };
    // clang-format on

    for (const test_case_t& test : test_cases)
    {
        const uint8_t& input = get<0>(test);
        const bool& expected = get<1>(test);

        assert(!is_power_of_2(-input));

        uint8_t as_u8 = input;
        int8_t as_s8 = (int8_t) input;
        assert(is_power_of_2(as_u8) == expected);
        assert(is_power_of_2(as_s8) == expected);

        uint16_t as_u16 = input;
        int16_t as_s16 = input;
        assert(is_power_of_2(as_u16) == expected);
        assert(is_power_of_2(as_s16) == expected);

        uint32_t as_u32 = input;
        int32_t as_s32 = input;
        assert(is_power_of_2(as_u32) == expected);
        assert(is_power_of_2(as_s32) == expected);

        uint64_t as_u64 = input;
        int64_t as_s64 = input;
        assert(is_power_of_2(as_u64) == expected);
        assert(is_power_of_2(as_s64) == expected);
    }
}

void test_relation_from_search_results()
{
    typedef tuple<bool, bool, bool, bool, relation> test_case_t;

    // clang-format off
    vector<test_case_t> test_cases
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
    // clang-format on

    for (const test_case_t& test : test_cases)
    {
        const bool& le_known = get<0>(test);
        const bool& is_le = get<1>(test);
        const bool& ge_known = get<2>(test);
        const bool& is_ge = get<3>(test);
        const relation& expected = get<4>(test);

        assert(relation_from_search_results(le_known, is_le, ge_known, is_ge) ==
               expected);
    }
}
} // namespace

void utilities_test_all()
{
    test_size_in_bits();
    test_print_bits();
    test_split_string();
    test_is_int();
    test_string_starts_ends_with();
    test_is_power_of_2();
    test_relation_from_search_results();
}
