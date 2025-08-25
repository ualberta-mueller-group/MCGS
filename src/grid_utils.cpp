#include "grid_utils.h"
#include <cassert>
#include <cmath>
#include <string>
#include <cstddef>
#include "grid.h"
#include "throw_assert.h"

using namespace std;

////////////////////////////////////////////////// helper functions
namespace {
bool increment_char(char& c)
{
    assert(         //
        c == '.' || //
        c == 'X' || //
        c == 'O'    //
    );              //

    if (c == '.')
        c = 'X';
    else if (c == 'X')
        c = 'O';
    else if (c == 'O')
    {
        c = '.';
        return false;
    }

    return true;
}

} // namespace

////////////////////////////////////////////////// grid_location functions
void grid_location::increment_position()
{
    THROW_ASSERT_DEBUG(valid());

    int_pair new_coord = _coord;
    new_coord.second++;

    if (new_coord.second >= _shape.second)
    {
        assert(!coord_in_shape(new_coord, _shape));

        new_coord.second = 0;
        new_coord.first++;

        if (new_coord.first >= _shape.first)
        {
            assert(!coord_in_shape(new_coord, _shape));
            _coord = new_coord;
            return; // now invalid
        }
    }

    assert(coord_in_shape(new_coord, _shape));

    _coord = new_coord;
}

bool grid_location::get_neighbor_coord(int_pair& neighbor_coord,
                                       const int_pair& coord,
                                       grid_dir direction,
                                       const int_pair& shape)
{
    THROW_ASSERT_DEBUG(coord_in_shape(coord, shape));

    const int_pair& delta = _GRID_DISPLACEMENTS[direction];
    assert(abs(delta.first) <= 1);
    assert(abs(delta.second) <= 1);

    const int_pair new_coord = {coord.first + delta.first,
                                coord.second + delta.second};

    bool success = coord_in_shape(new_coord, shape);

    if (success)
        neighbor_coord = new_coord;

    return success;
}

bool grid_location::get_neighbor_point(int& neighbor_point,
                                       const int_pair& coord,
                                       grid_dir direction,
                                       const int_pair& shape)
{
    THROW_ASSERT_DEBUG(coord_in_shape(coord, shape));

    int_pair neighbor_coord;
    bool success = get_neighbor_coord(neighbor_coord, coord, direction, shape);

    if (!success)
        return false;

    neighbor_point = coord_to_point(neighbor_coord, shape);
    return true;
}

