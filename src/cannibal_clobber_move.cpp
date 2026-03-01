#include "cannibal_clobber_move.h"

#include <ostream>

#include "print_move_helpers.h"
#include "cgt_move.h"
#include "int_pair.h"
#include "cgt_basics.h"

namespace cannibal_clobber_move {
void print_as_coords(std::ostream& str, const move& m, const int_pair& shape)
{
    int_pair coord1, coord2;
    bw target_color_ignored;
    unpack_coords(m, coord1, coord2, target_color_ignored);

    str << grid_coord_as_chesslike_string(coord1);
    str << '-';
    str << grid_coord_as_chesslike_string(coord2);
}
} // namespace cannibal_clobber_move
