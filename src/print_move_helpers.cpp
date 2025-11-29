#include "print_move_helpers.h"

#include <ostream>
#include <string>

#include "cgt_move.h"
#include "grid_location.h"
#include "int_pair.h"

void print_move1_as_points(std::ostream& str, const move& m)
{
    str << 1 + cgt_move::move1_get_part_1(m);
}

void print_move2_as_points(std::ostream& str, const move& m)
{
    str << 1 + cgt_move::move2_get_part_1(m);
    str << '-';
    str << 1 + cgt_move::move2_get_part_2(m);
}

void print_move3_as_points(std::ostream& str, const move& m)
{
    str << 1 + cgt_move::move3_get_part_1(m);
    str << '-';
    str << 1 + cgt_move::move3_get_part_2(m);
    str << '-';
    str << 1 + cgt_move::move3_get_part_3(m);
}

void print_move4_as_points(std::ostream& str, const move& m)
{
    str << 1 + cgt_move::move4_get_part_1(m);
    str << '-';
    str << 1 + cgt_move::move4_get_part_2(m);
    str << '-';
    str << 1 + cgt_move::move4_get_part_3(m);
    str << '-';
    str << 1 + cgt_move::move4_get_part_4(m);
}

void print_move6_as_points(std::ostream& str, const move& m)
{
    str << 1 + cgt_move::move6_get_part_1(m);
    str << '-';
    str << 1 + cgt_move::move6_get_part_2(m);
    str << '-';
    str << 1 + cgt_move::move6_get_part_3(m);
    str << '-';
    str << 1 + cgt_move::move6_get_part_4(m);
    str << '-';
    str << 1 + cgt_move::move6_get_part_5(m);
    str << '-';
    str << 1 + cgt_move::move6_get_part_6(m);
}

void print_move2_as_coords(std::ostream& str, const move& m,
                           const int_pair& shape)
{
    str << grid_coord_as_chesslike_string(cgt_move::move2_get_coord_1(m));
}

void print_move4_as_coords(std::ostream& str, const move& m,
                           const int_pair& shape)
{
    str << grid_coord_as_chesslike_string(cgt_move::move4_get_coord_1(m));
    str << '-';
    str << grid_coord_as_chesslike_string(cgt_move::move4_get_coord_2(m));
}

void print_move6_as_coords(std::ostream& str, const move& m,
                           const int_pair& shape)
{
    str << grid_coord_as_chesslike_string(cgt_move::move6_get_coord_1(m));
    str << '-';
    str << grid_coord_as_chesslike_string(cgt_move::move6_get_coord_2(m));
    str << '-';
    str << grid_coord_as_chesslike_string(cgt_move::move6_get_coord_3(m));
}

std::string grid_coord_as_chesslike_string(const int_pair& coord)
{
    std::string result;
    result.reserve(2);

    result += col_idx_to_chesslike_char(coord.second);

    // From 0-based internal to 1-based external coordinates
    result += std::to_string(coord.first + 1);

    return result;
}

std::string grid_point_as_chesslike_string(int point, const int_pair& shape)
{
    const int_pair coord = grid_location::point_to_coord(point, shape);
    return grid_coord_as_chesslike_string(coord);
}

