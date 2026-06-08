#include "grid_hash_test_utilities.h"

#include <vector>
#include <optional>
#include <string>
#include <cassert>

#include "int_pair.h"
#include "hashing.h"
#include "grid_hash.h"
#include "grid.h"

using namespace std;

////////////////////////////////////////////////// Helpers
namespace {
void generate_all_grid_orientations(vector<grid_pair_t>& grid_vec)
{
    // Must have 0 degree grid
    assert(grid_vec.size() == 1);

    // IMPORTANT: Ensure vector doesn't resize and invalidate these references
    grid_vec.reserve(8);
    assert(grid_vec.capacity() == 8);

    // 0
    const grid_pair_t& grid0 = grid_vec.back();

    // 0t
    grid_vec.emplace_back(transpose_grid_pair(grid0));

    // 90
    const grid_pair_t& grid90 = grid_vec.emplace_back(rotate_grid_pair(grid0));

    // 90t
    grid_vec.emplace_back(transpose_grid_pair(grid90));

    // 180
    const grid_pair_t& grid180 = grid_vec.emplace_back(rotate_grid_pair(grid90));

    // 180t
    grid_vec.emplace_back(transpose_grid_pair(grid180));

    // 270
    const grid_pair_t& grid270 = grid_vec.emplace_back(rotate_grid_pair(grid180));

    // 270t
    grid_vec.emplace_back(transpose_grid_pair(grid270));

    assert(grid_vec.capacity() == 8);
}
} // namespace

////////////////////////////////////////////////// Exported functions
std::vector<grid_pair_t> string_to_all_grid_orientations(
    const std::string& board_as_string, grid_type_enum grid_type)
{
    vector<grid_pair_t> grid_vec;

    switch (grid_type)
    {
        case GRID_TYPE_COLOR:
        {
            grid_vec.emplace_back(string_to_grid(board_as_string));
            break;
        }
        case GRID_TYPE_NUMBER:
        {
            grid_vec.emplace_back(string_to_int_grid(board_as_string));
            break;
        }
        default:
            assert(false);
    }

    generate_all_grid_orientations(grid_vec);
    return grid_vec;
}

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

int_pair find_color_in_grid_pair(const grid_pair_t& gp, int color)
{
    const vector<int>& board = gp.first;
    const int_pair& shape = gp.second;

    optional<int_pair> found_coord;

    for (int r = 0; r < shape.first; r++)
    {
        for (int c = 0; c < shape.second; c++)
        {
            const int_pair coord(r, c);

            const int idx = grid_location::coord_to_point(coord, shape);
            if (board[idx] == color)
            {
                assert(!found_coord);
                found_coord = coord;
            }
        }
    }

    assert(found_coord.has_value());
    return *found_coord;
}
