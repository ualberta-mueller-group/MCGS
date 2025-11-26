#pragma once

#include <utility>
#include <vector>

#include "int_pair.h"
#include "hashing.h"
#include "grid_hash.h"

typedef std::pair<std::vector<int>, int_pair> grid_pair_t;

grid_pair_t rotate_grid_pair(const grid_pair_t& grid_pair);
grid_pair_t transpose_grid_pair(const grid_pair_t& grid_pair);
hash_t get_grid_pair_hash(const grid_pair_t& grid_pair, grid_hash& gh);

