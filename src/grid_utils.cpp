#include "grid_utils.h"
#include <iostream>
#include <cassert>
#include <cmath>
#include "grid.h"

using std::cout, std::endl;

bool grid_location::increment_position()
{
    assert(coord_in_shape(_coord, _shape));

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
            return false;
        }
    }

    assert(coord_in_shape(new_coord, _shape));

    _coord = new_coord;
    return true;
}

bool grid_location::get_neighbor(int_pair& neighbor, const int_pair& coord, grid_dir direction, const int_pair& shape)
{
    const int_pair& delta = _GRID_DISPLACEMENTS[direction];
    assert(abs(delta.first) <= 1);
    assert(abs(delta.second) <= 1);

    const int_pair new_coord = {coord.first + delta.first, coord.second + delta.second};

    bool success = coord_in_shape(new_coord, shape);

    if (success)
        neighbor = new_coord;

    return success;
}

bool grid_location::get_neighbor_point(int& neighbor_point, const int_pair& coord, grid_dir direction, const int_pair& shape)
{
    int_pair neighbor;
    bool success = get_neighbor(neighbor, coord, direction, shape);

    if (!success)
        return false;

    neighbor_point = coord_to_point(neighbor, shape);
    return true;
}

std::ostream& operator<<(std::ostream& os, const int_pair& pr)
{
    os << '(' << pr.first << ' ' << pr.second << ')';
    return os;
}
