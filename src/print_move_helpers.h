#pragma once

#include "cgt_move_new.h"
#include <ostream>

void print_move1_as_points(std::ostream& str, const move& m);
void print_move2_as_points(std::ostream& str, const move& m);
void print_move3_as_points(std::ostream& str, const move& m);
void print_move4_as_points(std::ostream& str, const move& m);
void print_move6_as_points(std::ostream& str, const move& m);

// int_pairs are here so we can convert to points later if we want to...
void print_move2_as_coords(std::ostream& str, const move& m,
                           const int_pair& shape);
void print_move4_as_coords(std::ostream& str, const move& m,
                           const int_pair& shape);
void print_move6_as_coords(std::ostream& str, const move& m,
                           const int_pair& shape);

// Grid coord in format such as "g5", with columns a, b..., rows 1, 2...
std::string grid_coord_as_chesslike_string(const int_pair& coord);
std::string grid_point_as_chesslike_string(int point, const int_pair& shape);

// 0 -> 'a', 1 -> 'b', etc.
inline char col_idx_to_chesslike_char(int col_idx)
{
    static_assert('a' < 'z');
    THROW_ASSERT(col_idx >= 0 && col_idx <= ('z' - 'a'));
    // TODO this limit is very small...

    return 'a' + col_idx;
}
