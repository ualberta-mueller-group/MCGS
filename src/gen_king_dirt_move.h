#pragma once

#include <cassert>
#include <array>
#include <utility>
#include <ostream>

#include "cgt_basics.h"
#include "cgt_move.h"
#include "n_bit_int.h"
#include "int_pair.h"

namespace gen_king_dirt_move {
struct custom_layout
{
    /*
        From (int_pair)
        To (int_pair)
        Place stone (bool)
    */
    static constexpr std::array<std::pair<int, signed_type_enum>, 5> LAYOUT =
    {{
        {16, INT_UNSIGNED},
        {15, INT_UNSIGNED},
        {16, INT_UNSIGNED},
        {15, INT_UNSIGNED},
        {1, INT_UNSIGNED},
    }};
};

inline move create_from_coords(const int_pair& coord1, const int_pair& coord2,
                               bool place_stone)
{
    move m = 0;
    cgt_move::move_n_set_part<custom_layout, 1>(m, coord1.first);
    cgt_move::move_n_set_part<custom_layout, 2>(m, coord1.second);
    cgt_move::move_n_set_part<custom_layout, 3>(m, coord2.first);
    cgt_move::move_n_set_part<custom_layout, 4>(m, coord2.second);
    cgt_move::move_n_set_part<custom_layout, 5>(m, place_stone);

    assert(                                                                 //
        (place_stone ==                                                     //
         static_cast<bool>(cgt_move::move_n_get_part<custom_layout, 5>(m))) //
    );                                                                      //

    return m;
}

inline void unpack_coords(const move& m, int_pair& coord1, int_pair& coord2,
                          bool& place_stone)
{
    static_assert(
        cgt_move::move_parts_trivially_castable_to_int<custom_layout>());

    coord1.first =
        static_cast<int>(cgt_move::move_n_get_part<custom_layout, 1>(m));
    coord1.second =
        static_cast<int>(cgt_move::move_n_get_part<custom_layout, 2>(m));
    coord2.first =
        static_cast<int>(cgt_move::move_n_get_part<custom_layout, 3>(m));
    coord2.second =
        static_cast<int>(cgt_move::move_n_get_part<custom_layout, 4>(m));
    place_stone =
        static_cast<bool>(cgt_move::move_n_get_part<custom_layout, 5>(m));
}

void print_as_coords(std::ostream& str, const move& m, const int_pair& shape,
                     ebw to_play);

} // namespace gen_king_dirt_move
