#include "bit_array_test.h"
#include "bit_array.h"
#include "utilities.h"
#include "test_utilities.h"

#include <sstream>

using namespace std;

namespace {

void test_constructors()
{
    {
        bit_array arr1(0);
        bit_array arr2(0, false);
        bit_array arr3(0, true);

        assert(arr1.size() == 0);
        assert(arr2.size() == 0);
        assert(arr3.size() == 0);
    }
    {
        bit_array arr(4);
        const size_t N = arr.size();
        assert(N == 4);
        for (size_t i = 0; i < N; i++)
            assert(arr.get(i) == false);
    }
    {
        bit_array arr(7, false);
        const size_t N = arr.size();
        assert(N == 7);
        for (size_t i = 0; i < N; i++)
            assert(arr.get(i) == false);
    }
    {
        bit_array arr(9, true);
        const size_t N = arr.size();
        assert(N == 9);
        for (size_t i = 0; i < N; i++)
            assert(arr.get(i) == true);
    }
}

void test_get_set()
{
    bit_array arr(131);
    assert(arr.size() == 131);

    // All empty
    for (size_t i = 0; i < arr.size(); i++)
        assert(arr.get(i) == false);

    // Set some values
    constexpr uint64_t BITS[] = {
        0b1101111010110011011110100110001010101100001111001000010110000110,
        0b1101111010110011011110100110001010101100001111001000010110000110,
    };

    constexpr size_t N_BITS = 128;
    static_assert(N_BITS <= size_in_bits(BITS));

    for (size_t i = 0; i < N_BITS; i++)
    {
        const size_t chunk_idx = i / 64;
        assert(chunk_idx < 2);
        const uint64_t& chunk = BITS[chunk_idx];

        const size_t shift = i % 64;

        bool bit = (chunk >> shift) & 0x1;
        arr.set(i, bit);
    }

    // Now check that the values are correct...
    assert(arr.size() == 131);
    for (size_t i = 0; i < arr.size(); i++)
    {
        if (i >= N_BITS)
        {
            assert(arr.get(i) == false);
            continue;
        }

        const size_t chunk_idx = i / 64;
        assert(chunk_idx < 2);
        const uint64_t& chunk = BITS[chunk_idx];

        const size_t shift = i % 64;

        bool bit = (chunk >> shift) & 0x1;
        assert(arr.get(i) == bit);
    }
}

void test_compare()
{
    vector<bit_array> arrs;

    // 0 sizes
    arrs.emplace_back(0);
    arrs.emplace_back(0, false);
    arrs.emplace_back(0, true);

    {
        const size_t N = arrs.size();
        for (size_t i = 0; i < N; i++)
        {
            const bit_array& arr_i = arrs[i];
            assert(arr_i.compare(arr_i) == BIT_ARRAY_ALL_OVERLAP);

            for (size_t j = 0; j < N; j++)
            {
                const bit_array& arr_j = arrs[j];
                assert(arr_i.compare(arr_j) == BIT_ARRAY_ALL_OVERLAP);
                assert(arr_j.compare(arr_i) == BIT_ARRAY_ALL_OVERLAP);
            }
        }
    }

    // 101
    bit_array arr5_0(8);
    arr5_0.set(0, true);
    arr5_0.set(2, true);

    // 101
    bit_array arr5_1(8);
    arr5_1.set(0, true);
    arr5_1.set(2, true);

    // 001
    bit_array arr1(8);
    arr1.set(0, true);

    // 000
    bit_array arr0(8);

    // 010
    bit_array arr2(8);
    arr2.set(1, true);

    assert(arr5_0.compare(arr5_1) == BIT_ARRAY_ALL_OVERLAP); // 5 5
    assert(arr5_0.compare(arr1) == BIT_ARRAY_SOME_OVERLAP); // 5 1
    assert(arr5_0.compare(arr0) == BIT_ARRAY_NO_OVERLAP); // 5 0
    assert(arr5_0.compare(arr2) == BIT_ARRAY_NO_OVERLAP); // 5 2
    assert(arr0.compare(arr2) == BIT_ARRAY_NO_OVERLAP); // 0 2
}

void test_print()
{
    typedef tuple<uint16_t, string> test_case_t;

    vector<test_case_t> test_cases =
    {
        {0, "[0000000000000000]"},
        {5, "[0000000000000101]"},
        {5396, "[0001010100010100]"},
        {1669, "[0000011010000101]"},
        {4234, "[0001000010001010]"},
    };

    for (const test_case_t& test_case : test_cases)
    {
        const uint16_t val = get<0>(test_case);
        const string& exp = get<1>(test_case);

        bit_array arr(16);
        for (size_t i = 0; i < 16; i++)
            arr.set(i, (val >> i) & 0x1);

        stringstream str;
        str << arr;

        assert(str.str() == exp);
    }
}

void test_exceptions()
{
    bit_array arr0(0);
    ASSERT_DID_THROW(arr0.get(0));
    ASSERT_DID_THROW(arr0.get(1));
    ASSERT_DID_THROW(arr0.get(-1));

    ASSERT_DID_THROW(arr0.set(0, true));
    ASSERT_DID_THROW(arr0.set(1, true));
    ASSERT_DID_THROW(arr0.set(-1, true));

    bit_array arr1(1);
    ASSERT_DID_THROW(arr1.get(1));
    ASSERT_DID_THROW(arr1.get(-1));

    ASSERT_DID_THROW(arr1.set(1, true));
    ASSERT_DID_THROW(arr1.set(-1, true));
}

void test_copy_and_move()
{
    bit_array arr5_1(8);
    arr5_1.set(0, true);
    arr5_1.set(2, true);

    bit_array arr5_2 = arr5_1;
    assert(arr5_2.size() == 8);
    assert(arr5_2.get(0) == true);
    assert(arr5_2.get(1) == false);
    assert(arr5_2.get(2) == true);
    assert(arr5_2.get(3) == false);
    assert(arr5_2.compare(arr5_1) == BIT_ARRAY_ALL_OVERLAP);

    bit_array arr5_3(0);
    arr5_3 = arr5_2;
    assert(arr5_3.size() == 8);
    assert(arr5_3.compare(arr5_1) == BIT_ARRAY_ALL_OVERLAP);

    bit_array arr5_4 = std::move(arr5_3);
    assert(arr5_3.size() == 0);
    assert(arr5_4.compare(arr5_1) == BIT_ARRAY_ALL_OVERLAP);

    bit_array arr5_5(0);
    arr5_5 = std::move(arr5_4);
    assert(arr5_4.size() == 0);
    assert(arr5_5.compare(arr5_1) == BIT_ARRAY_ALL_OVERLAP);
}

} // namespace

void bit_array_test_all()
{
    test_constructors();
    test_get_set();
    test_compare();
    test_print();
    test_exceptions();
    test_copy_and_move();
}
