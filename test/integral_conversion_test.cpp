#include "integral_conversion_test.h"

#include <cstdint>
#include <cassert>
#include <limits>
#include <type_traits>

#include "integral_conversion.h"
#include "test_utilities.h"


namespace {

template <class Int_From, class Int_To>
void test_impl()
{
    static_assert(std::is_integral_v<Int_From> && std::is_integral_v<Int_To>);
    static_assert(sizeof(Int_From) < sizeof(int64_t) &&
                  sizeof(Int_To) < sizeof(int64_t));


    constexpr Int_From FROM_MIN = std::numeric_limits<Int_From>::min();
    constexpr Int_From FROM_MAX = std::numeric_limits<Int_From>::max();

    constexpr int64_t TO_MIN = std::numeric_limits<Int_To>::min();
    constexpr int64_t TO_MAX = std::numeric_limits<Int_To>::max();

    Int_From from = FROM_MIN;
    while (1)
    {
        const int64_t from_i64 = from;
        const bool is_safe = (TO_MIN <= from_i64 && from_i64 <= TO_MAX);

        assert(is_safe == static_cast_is_safe<Int_To>(from));

        if (!is_safe)
        {
            ASSERT_DID_THROW(integral_cast_checked<Int_To>(from));
        }
        else
        {
            const Int_To casted = integral_cast_checked<Int_To>(from);
            assert(casted == static_cast<Int_To>(from));
            assert(from == static_cast<Int_From>(casted));
        }

        if (from == FROM_MAX)
            break;
        from++;
    }

}

} // namespace

void integral_conversion_test_all()
{
    // 16 -> 8
    test_impl<int16_t, int8_t>(); // i -> i
    test_impl<int16_t, uint8_t>(); // i -> u
    test_impl<uint16_t, int8_t>(); // u -> i
    test_impl<uint16_t, uint8_t>(); // u -> u

    // 8 -> 16
    test_impl<int8_t, int16_t>(); // i -> i
    test_impl<int8_t, uint16_t>(); // i -> u
    test_impl<uint8_t, int16_t>(); // u -> i
    test_impl<uint8_t, uint16_t>(); // u -> u

    // 8 -> 8
    test_impl<int8_t, int8_t>(); // i -> i
    test_impl<int8_t, uint8_t>(); // i -> u
    test_impl<uint8_t, int8_t>(); // u -> i
    test_impl<uint8_t, uint8_t>(); // u -> u
}
