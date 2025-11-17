#pragma once

#include <cassert>
#include <climits>
#include <type_traits>
#include "throw_assert.h"

#define N_BIT_INT_MAX_BITS 31

typedef int move;
static_assert(N_BIT_INT_MAX_BITS < sizeof(int) * CHAR_BIT);
//////////////////////////////////////////////////
enum signed_type_enum
{
    INT_UNSIGNED = 0,
    INT_SIGNED,
};

namespace n_bit_int {

template <int n_bits>
inline constexpr int sign_bit_idx()
{
    static_assert(2 <= n_bits && n_bits <= N_BIT_INT_MAX_BITS);
    return n_bits - 1;
}

template <int n_bits>
inline constexpr int sign_bit_mask()
{
    static_assert(2 <= n_bits && n_bits <= N_BIT_INT_MAX_BITS);
    return 1 << sign_bit_idx<n_bits>();
}


template <int n_bits>
inline constexpr int value_mask()
{
    static_assert(2 <= n_bits && n_bits <= N_BIT_INT_MAX_BITS);
    return ~((unsigned int)(-1) << n_bits);
}

template <int n_bits, signed_type_enum signed_type>
inline constexpr int max_val()
{
    static_assert(2 <= n_bits && n_bits <= N_BIT_INT_MAX_BITS);

    switch (signed_type)
    {
        case INT_UNSIGNED:
            return value_mask<n_bits>();
        case INT_SIGNED:
            return sign_bit_mask<n_bits>() - 1;
    }

}

template <int n_bits, signed_type_enum signed_type>
inline constexpr int min_val()
{
    static_assert(2 <= n_bits && n_bits <= N_BIT_INT_MAX_BITS);

    switch (signed_type)
    {
        case INT_UNSIGNED:
            return 0;
        case INT_SIGNED:
            return -max_val<n_bits, signed_type>() - 1;
    }

}

template <int n_bits, signed_type_enum signed_type>
inline constexpr int shrink_int_to_n_bits(int val)
{
    static_assert(2 <= n_bits && n_bits <= N_BIT_INT_MAX_BITS);

    assert(                                        //
        (min_val<n_bits, signed_type>()) <= val && //
        val <= (max_val<n_bits, signed_type>())    //
    );                                             //

    return val & value_mask<n_bits>();
}

template <int n_bits, signed_type_enum signed_type>
inline constexpr int expand_int_from_n_bits(int n_bit_val)
{
    static_assert(2 <= n_bits && n_bits <= N_BIT_INT_MAX_BITS);

    assert(n_bit_val == (n_bit_val & value_mask<n_bits>()));

    if constexpr (signed_type == INT_SIGNED)
    {
        const bool is_negative = (n_bit_val & sign_bit_mask<n_bits>()) != 0;

        if (is_negative)
            n_bit_val |= ~value_mask<n_bits>();
    }

    return n_bit_val;
}

} // namespace n_bit_int

namespace cgt_move_new {


template <int n_bits, signed_type_enum signed_type, int n_previous_bits>
inline void move_n_set_part(move& m, int value)
{
    static_assert(n_previous_bits >= 0);

    static_assert(2 <= n_bits &&                                 //
                  n_bits <= N_BIT_INT_MAX_BITS &&                //
                  n_bits + n_previous_bits <= N_BIT_INT_MAX_BITS //
    );

    m &= ~(n_bit_int::value_mask<n_bits>() << n_previous_bits);

    m |= (n_bit_int::shrink_int_to_n_bits<n_bits, signed_type>(value)
          << n_previous_bits);
}

template <int n_bits, signed_type_enum signed_type, int n_previous_bits>
inline int move_n_get_part(const move& m)
{
    static_assert(n_previous_bits >= 0);

    static_assert(2 <= n_bits &&                                 //
                  n_bits <= N_BIT_INT_MAX_BITS &&                //
                  n_bits + n_previous_bits <= N_BIT_INT_MAX_BITS //
    );

    using move_unsigned_t = std::make_unsigned_t<move>;
    const move_unsigned_t& m_unsigned =
        reinterpret_cast<const move_unsigned_t&>(m);

    const int n_bit_value =
        (m_unsigned >> n_previous_bits) & n_bit_int::value_mask<n_bits>();

    return n_bit_int::expand_int_from_n_bits<n_bits, signed_type>(n_bit_value);
}

////////////////////////////////////////////////// move1 (i31)
//////////////////////////////////////// move1 setters
inline void move1_set_part_1(move& m, int part1)
{
    move_n_set_part<31, INT_SIGNED, 0>(m, part1);
}

//////////////////////////////////////// move1 getters
inline int move1_get_part_1(const move& m)
{
    return move_n_get_part<31, INT_SIGNED, 0>(m);
}

//////////////////////////////////////// move1 helpers
inline move move1_create(int part1)
{
    move m = 0;
    move1_set_part_1(m, part1);
    return m;
}

inline void move1_unpack(const move& m, int& part1)
{
    part1 = move1_get_part_1(m);
}

////////////////////////////////////////////////// move2 (i16, u15)
//////////////////////////////////////// move2 setters
inline void move2_set_part_1(move& m, int part1)
{
    move_n_set_part<16, INT_SIGNED, 0>(m, part1);
}

inline void move2_set_part_2(move& m, int part2)
{
    move_n_set_part<15, INT_UNSIGNED, 16>(m, part2);
}

//////////////////////////////////////// move2 getters
inline int move2_get_part_1(const move& m)
{
    return move_n_get_part<16, INT_SIGNED, 0>(m);
}

inline int move2_get_part_2(const move& m)
{
    return move_n_get_part<15, INT_UNSIGNED, 16>(m);
}

//////////////////////////////////////// move2 helpers
inline move move2_create(int part1, int part2)
{
    move m = 0;
    move2_set_part_1(m, part1);
    move2_set_part_2(m, part2);
    return m;
}

inline void move2_unpack(const move& m, int& part1, int& part2)
{
    part1 = move2_get_part_1(m);
    part2 = move2_get_part_2(m);
}

////////////////////////////////////////////////// move3 (i11, u10, u10)
//////////////////////////////////////// move3 setters
inline void move3_set_part_1(move& m, int part1)
{
    move_n_set_part<11, INT_SIGNED, 0>(m, part1);
}

inline void move3_set_part_2(move& m, int part2)
{
    move_n_set_part<10, INT_UNSIGNED, 11>(m, part2);
}

inline void move3_set_part_3(move& m, int part3)
{
    move_n_set_part<10, INT_UNSIGNED, 11 + 10>(m, part3);
}

//////////////////////////////////////// move3 getters
inline int move3_get_part_1(const move& m)
{
    return move_n_get_part<11, INT_SIGNED, 0>(m);
}

inline int move3_get_part_2(const move& m)
{
    return move_n_get_part<10, INT_UNSIGNED, 11>(m);
}

inline int move3_get_part_3(const move& m)
{
    return move_n_get_part<10, INT_UNSIGNED, 11 + 10>(m);
}

//////////////////////////////////////// move3 helpers
inline move move3_create(int part1, int part2, int part3)
{
    move m = 0;
    move3_set_part_1(m, part1);
    move3_set_part_2(m, part2);
    move3_set_part_3(m, part3);
    return m;
}

inline void move3_unpack(const move& m, int& part1, int& part2, int& part3)
{
    part1 = move3_get_part_1(m);
    part2 = move3_get_part_2(m);
    part3 = move3_get_part_3(m);
}

////////////////////////////////////////////////// move4 (i8, i8, i8, u7)
//////////////////////////////////////// move4 setters
inline void move4_set_part_1(move& m, int part1)
{
    move_n_set_part<8, INT_SIGNED, 0>(m, part1);
}

inline void move4_set_part_2(move& m, int part2)
{
    move_n_set_part<8, INT_SIGNED, 8>(m, part2);
}

inline void move4_set_part_3(move& m, int part3)
{

    move_n_set_part<8, INT_SIGNED, 8 + 8>(m, part3);
}

inline void move4_set_part_4(move& m, int part4)
{
    move_n_set_part<7, INT_UNSIGNED, 8 + 8 + 8>(m, part4);
}

//////////////////////////////////////// move4 getters
inline int move4_get_part_1(const move& m)
{
    return move_n_get_part<8, INT_SIGNED, 0>(m);
}

inline int move4_get_part_2(const move& m)
{
    return move_n_get_part<8, INT_SIGNED, 8>(m);
}

inline int move4_get_part_3(const move& m)
{
    return move_n_get_part<8, INT_SIGNED, 8 + 8>(m);
}

inline int move4_get_part_4(const move& m)
{
    return move_n_get_part<7, INT_UNSIGNED, 8 + 8 + 8>(m);
}

//////////////////////////////////////// move4 helpers
inline move move4_create(int part1, int part2, int part3, int part4)
{
    move m = 0;
    move4_set_part_1(m, part1);
    move4_set_part_2(m, part2);
    move4_set_part_3(m, part3);
    move4_set_part_4(m, part4);
    return m;
}


inline void move4_unpack(const move& m, int& part1, int& part2, int& part3,
                         int& part4)
{
    part1 = move4_get_part_1(m);
    part2 = move4_get_part_2(m);
    part3 = move4_get_part_3(m);
    part4 = move4_get_part_4(m);
}

////////////////////////////////////////////////// move6 (i6, u5, ..., u5)
//////////////////////////////////////// move6 setters
inline void move6_set_part_1(move& m, int part1)
{
    move_n_set_part<6, INT_SIGNED, 0>(m, part1);
}

inline void move6_set_part_2(move& m, int part2)
{
    move_n_set_part<5, INT_UNSIGNED, 6>(m, part2);
}

inline void move6_set_part_3(move& m, int part3)
{
    move_n_set_part<5, INT_UNSIGNED, 6 + 5>(m, part3);
}

inline void move6_set_part_4(move& m, int part4)
{
    move_n_set_part<5, INT_UNSIGNED, 6 + 5 + 5>(m, part4);
}

inline void move6_set_part_5(move& m, int part5)
{
    move_n_set_part<5, INT_UNSIGNED, 6 + 5 + 5 + 5>(m, part5);
}

inline void move6_set_part_6(move& m, int part6)
{
    move_n_set_part<5, INT_UNSIGNED, 6 + 5 + 5 + 5 + 5>(m, part6);
}

//////////////////////////////////////// move6 getters
inline int move6_get_part_1(const move& m)
{
    return move_n_get_part<6, INT_SIGNED, 0>(m);
}

inline int move6_get_part_2(const move& m)
{
    return move_n_get_part<5, INT_UNSIGNED, 6>(m);
}

inline int move6_get_part_3(const move& m)
{
    return move_n_get_part<5, INT_UNSIGNED, 6 + 5>(m);
}

inline int move6_get_part_4(const move& m)
{
    return move_n_get_part<5, INT_UNSIGNED, 6 + 5 + 5>(m);
}

inline int move6_get_part_5(const move& m)
{

    return move_n_get_part<5, INT_UNSIGNED, 6 + 5 + 5 + 5>(m);
}

inline int move6_get_part_6(const move& m)
{
    return move_n_get_part<5, INT_UNSIGNED, 6 + 5 + 5 + 5 + 5>(m);
}

//////////////////////////////////////// move6 helpers
inline move move6_create(int part1, int part2, int part3, int part4, int part5,
                         int part6)
{
    move m = 0;
    move6_set_part_1(m, part1);
    move6_set_part_2(m, part2);
    move6_set_part_3(m, part3);
    move6_set_part_4(m, part4);
    move6_set_part_5(m, part5);
    move6_set_part_6(m, part6);
    return m;
}


inline void move6_unpack(const move& m, int& part1, int& part2, int& part3,
                         int& part4, int& part5, int& part6)
{
    part1 = move6_get_part_1(m);
    part2 = move6_get_part_2(m);
    part3 = move6_get_part_3(m);
    part4 = move6_get_part_4(m);
    part5 = move6_get_part_5(m);
    part6 = move6_get_part_6(m);
}


} // namespace cgt_move_new



//////////////////////////////////////////////////
void test_new_move_stuff();
