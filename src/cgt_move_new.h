/*
    Tools for encoding and decoding moves. Defines various N part moves.

    The bit fields of an N part move are specified by a "layout struct". For
    example, move2_layout specifies a move type containing a 16 bit signed
    value, and a 15 bit unsigned value.

    NOTE: Many values in this file should be constexprs, don't change them to
          const! This file doesn't seem to increase compile time measurably,
          compared to the old one.
*/
#pragma once

#include <cassert>
#include <array>
#include <tuple>
#include <type_traits>
#include "n_bit_int.h"

typedef int move;

namespace cgt_move_new {

////////////////////////////////////////////////// bit fields of N part moves
struct move1_layout
{
    static constexpr std::array<std::pair<int, signed_type_enum>, 1> LAYOUT =
    {{
        {31, INT_SIGNED},
    }};
};

struct move2_layout
{
    static constexpr std::array<std::pair<int, signed_type_enum>, 2> LAYOUT =
    {{
        {16, INT_SIGNED},
        {15, INT_UNSIGNED},
    }};
};

struct move3_layout
{
    static constexpr std::array<std::pair<int, signed_type_enum>, 3> LAYOUT =
    {{
        {11, INT_SIGNED},
        {10, INT_UNSIGNED},
        {10, INT_UNSIGNED},
    }};
};

struct move4_layout
{
    static constexpr std::array<std::pair<int, signed_type_enum>, 4> LAYOUT =
    {{
        {8, INT_SIGNED},
        {8, INT_SIGNED},
        {8, INT_SIGNED},
        {7, INT_UNSIGNED},
    }};
};

struct move6_layout
{
    static constexpr std::array<std::pair<int, signed_type_enum>, 6> LAYOUT =
    {{
        {6, INT_SIGNED},
        {5, INT_UNSIGNED},
        {5, INT_UNSIGNED},
        {5, INT_UNSIGNED},
        {5, INT_UNSIGNED},
        {5, INT_UNSIGNED},
    }};
};

/*
   Move parts must have bit fields whose sizes are in the interval
   [2, N_BIT_INT_MAX_BITS] and whose sum is also in the interval.
*/
template <class Move_Layout_T>
constexpr bool move_layout_is_legal()
{
    int total_bits = 0;

    for (const std::pair<int, signed_type_enum>& p : Move_Layout_T::LAYOUT)
    {
        if (!(2 <= p.first && p.first <= N_BIT_INT_MAX_BITS))
            return false;

        total_bits += p.first;
    }

    return (2 <= total_bits && total_bits <= N_BIT_INT_MAX_BITS);
}


////////////////////////////////////////////////// implementation details

/*
   Given a move layout struct and the (1-indexed) move part number,
   return a tuple containing the part's:

   - Bit field width
   - Left shift amount (cumulative sum of previous parts' widths)
   - Sign type (INT_UNSIGNED or INT_SIGNED)
*/
template <class Move_Layout_T, unsigned int move_part_number>
constexpr std::tuple<int, int, signed_type_enum>
get_move_part_width_shift_and_sign_type()
{
    static_assert(move_layout_is_legal<Move_Layout_T>());
    static_assert(1 <= move_part_number &&
                  move_part_number <= Move_Layout_T::LAYOUT.size());

    int total_shift = 0;

    for (unsigned int i = 1; i < move_part_number; i++)
    {
        const std::pair<int, signed_type_enum> p = Move_Layout_T::LAYOUT[i - 1];
        total_shift += p.first;
    }

    const std::pair<int, signed_type_enum> p =
        Move_Layout_T::LAYOUT[move_part_number - 1];

    return {p.first, total_shift, p.second};
}

/*
   Store an integral value inside of the move part specified by the
   move layout struct and (1-indexed) move part number.

   This function converts the given value to an N bit int.
*/
template <class Move_Layout_T, unsigned int move_part_number>
inline void move_n_set_part(move& m, int value)
{
    static_assert(move_layout_is_legal<Move_Layout_T>());
    static_assert(1 <= move_part_number &&
                  move_part_number <= Move_Layout_T::LAYOUT.size());

    constexpr std::tuple<int, int, signed_type_enum> WIDTH_SHIFT_AND_SIGN_TYPE =
        get_move_part_width_shift_and_sign_type<Move_Layout_T,
                                                move_part_number>();

    constexpr int WIDTH = std::get<0>(WIDTH_SHIFT_AND_SIGN_TYPE);
    constexpr int SHIFT = std::get<1>(WIDTH_SHIFT_AND_SIGN_TYPE);
    constexpr signed_type_enum SIGN_TYPE =
        std::get<2>(WIDTH_SHIFT_AND_SIGN_TYPE);

    static_assert(2 <= WIDTH &&                       //
                  SHIFT >= 0 &&                       //
                  WIDTH <= N_BIT_INT_MAX_BITS &&      //
                  WIDTH + SHIFT <= N_BIT_INT_MAX_BITS //
    );

    // Remove previous N bit int from the move
    m &= ~(n_bit_int::value_mask<WIDTH>() << SHIFT);

    // Insert the new one after shrinking it
    m |= (n_bit_int::shrink_int_to_n_bits<WIDTH, SIGN_TYPE>(value)
          << SHIFT);
}

/*
   Get an integral value from the move part specified by the move layout
   struct and (1-indexed) move part number.

   This function converts the N bit int stored in the move back to a regular
   C++ int.
*/
template <class Move_Layout_T, unsigned int move_part_number>
inline int move_n_get_part(const move& m)
{
    static_assert(move_layout_is_legal<Move_Layout_T>());
    static_assert(1 <= move_part_number &&
                  move_part_number <= Move_Layout_T::LAYOUT.size());

    constexpr std::tuple<int, int, signed_type_enum> WIDTH_SHIFT_AND_SIGN_TYPE =
        get_move_part_width_shift_and_sign_type<Move_Layout_T,
                                                move_part_number>();

    constexpr int WIDTH = std::get<0>(WIDTH_SHIFT_AND_SIGN_TYPE);
    constexpr int SHIFT = std::get<1>(WIDTH_SHIFT_AND_SIGN_TYPE);
    constexpr signed_type_enum SIGN_TYPE =
        std::get<2>(WIDTH_SHIFT_AND_SIGN_TYPE);

    static_assert(2 <= WIDTH &&                       //
                  SHIFT >= 0 &&                       //
                  WIDTH <= N_BIT_INT_MAX_BITS &&      //
                  WIDTH + SHIFT <= N_BIT_INT_MAX_BITS //
    );

    // Right shift of signed value is undefined. Treat the move as unsigned.
    using move_unsigned_t = std::make_unsigned_t<move>;
    const move_unsigned_t& m_unsigned =
        reinterpret_cast<const move_unsigned_t&>(m);

    // Extract the N bit int from the move
    const int n_bit_value =
        (m_unsigned >> SHIFT) & n_bit_int::value_mask<WIDTH>();

    // Convert it to a regular int and return it
    return n_bit_int::expand_int_from_n_bits<WIDTH, SIGN_TYPE>(n_bit_value);
}


//////////////////////////////////////////////////
/*
   N part move functions follow below. Provides implementations for
   1, 2, 3, 4, and 6 part moves. See the move layout structs earlier in this
   file for details on move part sizes and signedness.
*/


////////////////////////////////////////////////// move1 (i31)
//////////////////////////////////////// move1 setters
inline void move1_set_part_1(move& m, int part1)
{
    move_n_set_part<move1_layout, 1>(m, part1);
}

//////////////////////////////////////// move1 getters
inline int move1_get_part_1(const move& m)
{
    return move_n_get_part<move1_layout, 1>(m);
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
    move_n_set_part<move2_layout, 1>(m, part1);
}

inline void move2_set_part_2(move& m, int part2)
{
    move_n_set_part<move2_layout, 2>(m, part2);
}

//////////////////////////////////////// move2 getters
inline int move2_get_part_1(const move& m)
{
    return move_n_get_part<move2_layout, 1>(m);
}

inline int move2_get_part_2(const move& m)
{
    return move_n_get_part<move2_layout, 2>(m);
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
    move_n_set_part<move3_layout, 1>(m, part1);
}

inline void move3_set_part_2(move& m, int part2)
{
    move_n_set_part<move3_layout, 2>(m, part2);
}

inline void move3_set_part_3(move& m, int part3)
{
    move_n_set_part<move3_layout, 3>(m, part3);
}

//////////////////////////////////////// move3 getters
inline int move3_get_part_1(const move& m)
{
    return move_n_get_part<move3_layout, 1>(m);
}

inline int move3_get_part_2(const move& m)
{
    return move_n_get_part<move3_layout, 2>(m);
}

inline int move3_get_part_3(const move& m)
{
    return move_n_get_part<move3_layout, 3>(m);
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
    move_n_set_part<move4_layout, 1>(m, part1);
}

inline void move4_set_part_2(move& m, int part2)
{
    move_n_set_part<move4_layout, 2>(m, part2);
}

inline void move4_set_part_3(move& m, int part3)
{
    move_n_set_part<move4_layout, 3>(m, part3);
}

inline void move4_set_part_4(move& m, int part4)
{
    move_n_set_part<move4_layout, 4>(m, part4);
}

//////////////////////////////////////// move4 getters
inline int move4_get_part_1(const move& m)
{
    return move_n_get_part<move4_layout, 1>(m);
}

inline int move4_get_part_2(const move& m)
{
    return move_n_get_part<move4_layout, 2>(m);
}

inline int move4_get_part_3(const move& m)
{
    return move_n_get_part<move4_layout, 3>(m);
}

inline int move4_get_part_4(const move& m)
{
    return move_n_get_part<move4_layout, 4>(m);
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
    move_n_set_part<move6_layout, 1>(m, part1);
}

inline void move6_set_part_2(move& m, int part2)
{
    move_n_set_part<move6_layout, 2>(m, part2);
}

inline void move6_set_part_3(move& m, int part3)
{
    move_n_set_part<move6_layout, 3>(m, part3);
}

inline void move6_set_part_4(move& m, int part4)
{
    move_n_set_part<move6_layout, 4>(m, part4);
}

inline void move6_set_part_5(move& m, int part5)
{
    move_n_set_part<move6_layout, 5>(m, part5);
}

inline void move6_set_part_6(move& m, int part6)
{
    move_n_set_part<move6_layout, 6>(m, part6);
}

//////////////////////////////////////// move6 getters
inline int move6_get_part_1(const move& m)
{
    return move_n_get_part<move6_layout, 1>(m);
}

inline int move6_get_part_2(const move& m)
{
    return move_n_get_part<move6_layout, 2>(m);
}

inline int move6_get_part_3(const move& m)
{
    return move_n_get_part<move6_layout, 3>(m);
}

inline int move6_get_part_4(const move& m)
{
    return move_n_get_part<move6_layout, 4>(m);
}

inline int move6_get_part_5(const move& m)
{
    return move_n_get_part<move6_layout, 5>(m);
}

inline int move6_get_part_6(const move& m)
{
    return move_n_get_part<move6_layout, 6>(m);
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
