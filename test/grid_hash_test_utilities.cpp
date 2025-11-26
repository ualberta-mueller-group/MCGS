#pragma once

#include "grid_hash_test_utilities.h"

#include <vector>

#include "int_pair.h"
#include "hashing.h"
#include "grid_hash.h"

using namespace std;

grid_pair_t rotate_grid_pair(const grid_pair_t& grid_pair)
{
    const std::vector<int>& vec = grid_pair.first;
    const int_pair& shape = grid_pair.second;

    const int_pair shape_rotated(shape.second, shape.first);

    return {grid::rotate_90_board(vec, shape), shape_rotated};
}

grid_pair_t transpose_grid_pair(const grid_pair_t& grid_pair)
{
    const vector<int>& vec = grid_pair.first;
    const int_pair& shape = grid_pair.second;

    const int_pair shape_rotated(shape.second, shape.first);

    return {grid::transpose_board(vec, shape), shape_rotated};
}

hash_t get_grid_pair_hash(const grid_pair_t& grid_pair, grid_hash& gh)
{
    gh.init_from_board_and_type(grid_pair.first, grid_pair.second, 0);
    return gh.get_value();
}
