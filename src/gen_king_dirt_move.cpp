#include "gen_king_dirt_move.h"

#include <ostream>

#include "print_move_helpers.h"
#include "cgt_move.h"
#include "int_pair.h"
#include "cgt_basics.h"

namespace gen_king_dirt_move {
void print_as_coords(std::ostream& str, const move& m, const int_pair& shape,
                     ebw to_play)
{
    assert(is_black_white(to_play));

    int_pair coord1, coord2;
    bool place_stone;
    unpack_coords(m, coord1, coord2, place_stone);

    if (place_stone)
    {
        str << grid_coord_as_chesslike_string(coord1);
        str << '_' << color_to_char(to_play);
    }
    else
    {
        str << grid_coord_as_chesslike_string(coord1);
        str << '-';
        str << grid_coord_as_chesslike_string(coord2);
    }
}
} // namespace gen_king_dirt_move
