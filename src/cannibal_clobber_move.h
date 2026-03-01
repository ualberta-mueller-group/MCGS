#pragma once

#include <cassert>
#include <array>
#include <utility>
#include <ostream>

#include "cgt_basics.h"
#include "cgt_move.h"
#include "n_bit_int.h"
#include "int_pair.h"

/*
   TODO cgt_move.h move_layout_is_legal<...>(...) requires
   the target color to have at least 2 bits when it should only require 1...
*/
namespace cannibal_clobber_move {
struct custom_layout
{
    static constexpr std::array<std::pair<int, signed_type_enum>, 5> LAYOUT =
    {{
        {7, INT_UNSIGNED},
        {7, INT_UNSIGNED},
        {7, INT_UNSIGNED},
        {7, INT_UNSIGNED},

        // should be 1, must be at least 2. Is 3 to evenly distribute bits
        {3, INT_UNSIGNED},
    }};
};

inline move create_from_coords(const int_pair& coord1, const int_pair& coord2,
                               bw target_color)
{
    move m = 0;
    cgt_move::move_n_set_part<custom_layout, 1>(m, coord1.first);
    cgt_move::move_n_set_part<custom_layout, 2>(m, coord1.second);
    cgt_move::move_n_set_part<custom_layout, 3>(m, coord2.first);
    cgt_move::move_n_set_part<custom_layout, 4>(m, coord2.second);

    assert(is_black_white(target_color));
    const int color_enc = (target_color == BLACK) ? 0 : 1;
    cgt_move::move_n_set_part<custom_layout, 5>(m, color_enc);

    return m;
}

inline void unpack_coords(const move& m, int_pair& coord1, int_pair& coord2,
                          bw& target_color)
{
    coord1.first = cgt_move::move_n_get_part<custom_layout, 1>(m);
    coord1.second = cgt_move::move_n_get_part<custom_layout, 2>(m);
    coord2.first = cgt_move::move_n_get_part<custom_layout, 3>(m);
    coord2.second = cgt_move::move_n_get_part<custom_layout, 4>(m);

    const int color_enc = cgt_move::move_n_get_part<custom_layout, 5>(m);
    assert(in_interval(color_enc, 0, 1));
    target_color = (color_enc == 0) ? BLACK : WHITE;
}

void print_as_coords(std::ostream& str, const move& m, const int_pair& shape);

} // namespace cannibal_clobber_move

