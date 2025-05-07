#include "grid_utils.h"

grid_location::grid_location(const int_pair& shape)
    : _shape(shape),
    _coord(0, 0)
{
    // TODO 0 size grid is awkward...
    assert(_shape.first > 0 && _shape.second > 0);
}

grid_location::grid_location(const int_pair& shape, const int_pair& coord)
    : _shape(shape),
    _coord(coord)
{
    // TODO 0 size grid is awkward...
    assert(_shape.first > 0 && _shape.second > 0);
    assert(coord_in_shape(_coord, _shape));
}

grid_location::grid_location(const int_pair& shape, int point)
    : _shape(shape),
    _coord(point_to_coord(point, shape))
{
    // TODO 0 size grid is awkward...
    assert(_shape.first > 0 && _shape.second > 0);
    assert(coord_in_shape(_coord, _shape));
}

const int_pair& grid_location::get_shape() const
{
    return _shape;
}

const int_pair& grid_location::get_coord() const
{
    return _coord;
}

int grid_location::get_point() const
{
    return coord_to_point(_coord, _shape);
}

void grid_location::set_shape(const int_pair& shape)
{
    _shape = shape;
    assert(coord_in_shape(_coord, _shape));
}

void grid_location::set_coord(const int_pair& coord)
{
    _coord = coord;
    assert(coord_in_shape(_coord, _shape));
}

void grid_location::set_point(int point)
{
    _coord = point_to_coord(point, _shape);
    assert(coord_in_shape(_coord, _shape));
}

bool grid_location::get_neighbor(int_pair& neighbor, grid_dir direction) const
{
    return get_neighbor(neighbor, _coord, direction, _shape);
}

bool grid_location::move(grid_dir direction)
{
    return get_neighbor(_coord, _coord, direction, _shape);
}

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

bool grid_location::coord_in_shape(const int_pair& coord, const int_pair& shape)
{
    return                                 //
        (coord.first >= 0) &&              //
        (coord.first < shape.first) &&    //
        (coord.second >= 0) &&             //
        (coord.second < shape.second);    //
}

bool grid_location::point_in_shape(int point, const int_pair& shape)
{
    const int grid_size = shape.first * shape.second;
    return (point >= 0) && (point < grid_size);
}

int grid_location::coord_to_point(const int_pair& coord, const int_pair& shape)
{
    assert(coord_in_shape(coord, shape));
    return coord.first * shape.second + coord.second;
}

int_pair grid_location::point_to_coord(int point, const int_pair& shape)
{
    assert(point_in_shape(point, shape));
    int r = point / shape.second;
    int c = point % shape.second;
    return {r, c};
}

bool grid_location::get_neighbor(int_pair& neighbor, const int_pair& coord, grid_dir direction, const int_pair& shape)
{
    const int_pair& delta = _GRID_DISPLACEMENTS[direction];
    assert(abs(delta.first) <= 1);
    assert(abs(delta.second) <= 1);

    const int_pair new_coord = {coord.first + delta.first, coord.second + delta.second};

    bool success = coord_in_shape(neighbor, shape);

    if (success)
        neighbor = new_coord;

    return success;
}
