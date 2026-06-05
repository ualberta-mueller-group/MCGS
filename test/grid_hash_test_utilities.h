#pragma once

#include <utility>
#include <vector>

#include "grid.h"
#include "int_pair.h"
#include "hashing.h"
#include "grid_hash.h"

typedef std::pair<std::vector<int>, int_pair> grid_pair_t;

std::vector<grid_pair_t> string_to_all_grid_orientations(
    const std::string& board_as_string, grid_type_enum grid_type);

grid_pair_t rotate_grid_pair(const grid_pair_t& grid_pair);
grid_pair_t transpose_grid_pair(const grid_pair_t& grid_pair);
hash_t get_grid_pair_hash(const grid_pair_t& grid_pair, grid_hash& gh);

// Assumes color only occurs once
int_pair find_color_in_grid_pair(const grid_pair_t& gp, int color);


