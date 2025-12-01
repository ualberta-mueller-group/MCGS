#include "utilities_test.h"
#include "cgt_basics.h"
#include "utilities.h"
#include <cstdint>
#include <sstream>
#include <cstddef>
#include <tuple>
#include <cassert>
#include <string>
#include <vector>
#include <climits>

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
    /*
       input
       expected output
    */
    typedef tuple<uint8_t, string> test_case_t;

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
    /*
        input
        expected output
    */
    typedef tuple<string, vector<string>> test_case_t;

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
    /*
       input
       expected value
    */
    typedef tuple<string, bool> test_case_t;

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
    /*
        string
        word
        expected string_starts_with() result
        expected string_ends_with() result
    */
    typedef tuple<string, string, bool, bool> test_case_t;

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
    /*
       input
       expected result
    */
    typedef tuple<uint8_t, bool> test_case_t;

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

void test_alternating_mask()
{
    assert(alternating_mask<int8_t>() == 0b01010101);
    assert(alternating_mask<uint8_t>() == 0b01010101);

    assert(alternating_mask<int16_t>() == 0b0101010101010101);
    assert(alternating_mask<uint16_t>() == 0b0101010101010101);

    assert(alternating_mask<int32_t>() == 0b01010101010101010101010101010101);
    assert(alternating_mask<uint32_t>() == 0b01010101010101010101010101010101);

    assert(alternating_mask<int16_t>() == 0b0101010101010101);
    assert(alternating_mask<uint16_t>() == 0b0101010101010101);

    assert(alternating_mask<int64_t>() ==
           0b0101010101010101010101010101010101010101010101010101010101010101);

    assert(alternating_mask<uint64_t>() ==
           0b0101010101010101010101010101010101010101010101010101010101010101);
}

void test_rotate_functions()
{
    /*
        rotate distance
        input
        rotate_left result
        rotate_right result
        rotate_interleaved result
    */
    typedef tuple<int, uint32_t, uint32_t, uint32_t, uint32_t> test_case_t;

    vector<test_case_t> test_cases = {
        {
            404,
            0b10111000010110101111000010001000,
            0b00001000100010111000010110101111,
            0b10101111000010001000101110000101,
            0b10101010000010011000111110000101,
        },
        {
            218,
            0b11011100000000010000001100011100,
            0b01110011011100000000010000001100,
            0b00000000010000001100011100110111,
            0b01010001010100001000011000100110,
        },
        {
            536,
            0b01000011000100110000111000000000,
            0b00000000010000110001001100001110,
            0b00010011000011100000000001000011,
            0b00000010010010110001000100000110,
        },
        {
            460,
            0b11001001110001101000000110000101,
            0b01101000000110000101110010011100,
            0b00011000010111001001110001101000,
            0b01001000000110001101110000111100,
        },
        {
            618,
            0b00010111111100110000010100100100,
            0b11001100000101001001000001011111,
            0b01001001000001011111110011000001,
            0b01001100000101001011100011010101,
        },
        {
            854,
            0b10001011001101110000000000100010,
            0b00001000101000101100110111000000,
            0b11011100000000001000101000101100,
            0b10001000000000001100111101101000,
        },
        {
            729,
            0b11111001111001100001100110001000,
            0b00010001111100111100110000110011,
            0b11110011000011001100010001111100,
            0b01010001101001101100110001110110,
        },
        {
            391,
            0b11010100000110101111100100101011,
            0b00001101011111001001010111101010,
            0b01010111101010000011010111110010,
            0b01011101001010001001010111111010,
        },
        {
            260,
            0b00101111011111111100110011011011,
            0b11110111111111001100110110110010,
            0b10110010111101111111110011001101,
            0b11110111111101101110110110011000,
        },
        {
            548,
            0b00000111101110011110110001100010,
            0b01111011100111101100011000100000,
            0b00100000011110111001111011000110,
            0b01110001001111101100111010000010,
        },
        {
            5,
            0b10101100000111010111000011100011,
            0b10000011101011100001110001110101,
            0b00011101011000001110101110000111,
            0b10010111111010100100100100100101,
        },
        {
            2,
            0b00011001111101100001011001000101,
            0b01100111110110000101100100010100,
            0b01000110011111011000010110010001,
            0b01000111011110001101000110010100,
        },
        {
            22,
            0b10011101000001000000011110110001,
            0b11101100011001110100000100000001,
            0b00010000000111101100011001110100,
            0b01000100010011111100001100100001,
        },
        {
            0,
            0b11001010101001111001101010010001,
            0b11001010101001111001101010010001,
            0b11001010101001111001101010010001,
            0b11001010101001111001101010010001,
        },
    };

    for (const test_case_t& test_case : test_cases)
    {
        const int& distance = get<0>(test_case);
        const uint32_t& input = get<1>(test_case);
        const uint32_t& exp_left = get<2>(test_case);
        const uint32_t& exp_right = get<3>(test_case);
        const uint32_t& exp_interleaved = get<4>(test_case);

        assert(rotate_left(input, distance) == exp_left);
        assert(rotate_right(input, distance) == exp_right);
        assert(rotate_interleaved(input, distance) == exp_interleaved);

        assert(rotate_left((int32_t&) input, distance) == (int32_t&) exp_left);
        assert(rotate_right((int32_t&) input, distance) ==
               (int32_t&) exp_right);
        assert(rotate_interleaved((int32_t&) input, distance) ==
               (int32_t&) exp_interleaved);
    }
}

void test_new_vector_capacity()
{
    /*
        vector access index
        current capacity
        expected new capcity
    */
    typedef tuple<size_t, size_t, size_t> test_case_t;

    // clang-format off
    vector<test_case_t> test_cases =
    {
        {0, 0, 1},
        {3, 0, 4},
        {4, 0, 8},
        {7, 0, 8},
        {63, 0, 64},
        {64, 0, 128},
        {0, 3, 3},
        {3, 3, 6},
        {4, 3, 6},
        {7, 3, 12},
        {2, 1, 4},
        {6, 1, 8},
        {21, 1, 32},
        {40, 1, 64},
        {0, 1, 1},
        {3, 7, 7},
        {6, 8, 8},
        {21, 64, 64},
    };
    // clang-format on

    for (const test_case_t& test_case : test_cases)
    {
        const size_t& index = get<0>(test_case);
        const size_t& capacity = get<1>(test_case);
        const size_t& exp_new_capacity = get<2>(test_case);

        const size_t new_capacity = new_vector_capacity(index, capacity);

        assert(new_capacity == exp_new_capacity);
    }
}

void test_relation_from_search_results()
{
    /*
       le_known
       is_le
       ge_known
       is_ge
       expected relation
    */
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

void test_bools_to_outcome_class()
{
    assert(bools_to_outcome_class(false, false) == outcome_class::P);
    assert(bools_to_outcome_class(false, true) == outcome_class::R);
    assert(bools_to_outcome_class(true, false) == outcome_class::L);
    assert(bools_to_outcome_class(true, true) == outcome_class::N);
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

    test_alternating_mask();
    test_rotate_functions();
    test_new_vector_capacity();
    test_relation_from_search_results();
    test_bools_to_outcome_class();
}
