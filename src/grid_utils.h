#pragma once

/*
    Helpers for grids. Defines grid_location
*/

#include <array>
#include "grid.h"
#include <ostream>
#include <cassert>

enum grid_dir
{
    GRID_DIR_UP = 0,
    GRID_DIR_UP_RIGHT,
    GRID_DIR_RIGHT,
    GRID_DIR_DOWN_RIGHT,
    GRID_DIR_DOWN,
    GRID_DIR_DOWN_LEFT,
    GRID_DIR_LEFT,
    GRID_DIR_UP_LEFT,
    GRID_DIR_NO_DIRECTION,
};

static constexpr std::array<int_pair, 9> _GRID_DISPLACEMENTS
{
    {
        {-1, 0},
        {-1, 1},
        {0, 1},
        {1, 1},
        {1, 0},
        {1, -1},
        {0, -1},
        {-1, -1},
        {0, 0},
    }
};

static_assert(_GRID_DISPLACEMENTS[GRID_DIR_UP] == int_pair(-1, 0));
static_assert(_GRID_DISPLACEMENTS[GRID_DIR_UP_RIGHT] == int_pair(-1, 1));
static_assert(_GRID_DISPLACEMENTS[GRID_DIR_RIGHT] == int_pair(0, 1));
static_assert(_GRID_DISPLACEMENTS[GRID_DIR_DOWN_RIGHT] == int_pair(1, 1));
static_assert(_GRID_DISPLACEMENTS[GRID_DIR_DOWN] == int_pair(1, 0));
static_assert(_GRID_DISPLACEMENTS[GRID_DIR_DOWN_LEFT] == int_pair(1, -1));
static_assert(_GRID_DISPLACEMENTS[GRID_DIR_LEFT] == int_pair(0, -1));
static_assert(_GRID_DISPLACEMENTS[GRID_DIR_UP_LEFT] == int_pair(-1, -1));
static_assert(_GRID_DISPLACEMENTS[GRID_DIR_NO_DIRECTION] == int_pair(0, 0));

static constexpr std::array<grid_dir, 4> GRID_DIRS_CARDINAL
{
    GRID_DIR_UP,
    GRID_DIR_RIGHT,
    GRID_DIR_DOWN,
    GRID_DIR_LEFT,
};

static constexpr std::array<grid_dir, 4> GRID_DIRS_DIAGONAL
{
    GRID_DIR_UP_RIGHT,
    GRID_DIR_DOWN_RIGHT,
    GRID_DIR_DOWN_LEFT,
    GRID_DIR_UP_LEFT
};

static constexpr std::array<grid_dir, 8> GRID_DIRS_ALL
{
    GRID_DIR_UP,
    GRID_DIR_UP_RIGHT,
    GRID_DIR_RIGHT,
    GRID_DIR_DOWN_RIGHT,
    GRID_DIR_DOWN,
    GRID_DIR_DOWN_LEFT,
    GRID_DIR_LEFT,
    GRID_DIR_UP_LEFT
};

class grid_location
{
public:
    // constructors
    grid_location(const int_pair& shape);
    grid_location(const int_pair& shape, const int_pair& coord);
    grid_location(const int_pair& shape, int point);

    // getters
    const int_pair& get_shape() const;
    const int_pair& get_coord() const;
    int get_point() const;

    // setters
    void set_shape(const int_pair& shape);
    void set_coord(const int_pair& coord);
    void set_point(int point);

    // utility
    bool get_neighbor_coord(int_pair& neighbor_coord, grid_dir direction) const;
    bool get_neighbor_point(int& neighbor_point, grid_dir direction) const;

    // mutators
    bool move(grid_dir direction);
    bool increment_position();

    // static utility functions
    static bool coord_in_shape(const int_pair& coord, const int_pair& shape);
    static bool point_in_shape(int point, const int_pair& shape);

    static int coord_to_point(const int_pair& coord, const int_pair& shape);
    static int_pair point_to_coord(int point, const int_pair& shape);

    static bool get_neighbor_coord(int_pair& neighbor_coord, const int_pair& coord, grid_dir direction, const int_pair& shape);
    static bool get_neighbor_point(int& neighbor_point, const int_pair& coord, grid_dir direction, const int_pair& shape);


private:
    int_pair _shape;
    int_pair _coord;
};

std::ostream& operator<<(std::ostream& os, const int_pair& pr);


////////////////////////////////////////////////////////////
inline grid_location::grid_location(const int_pair& shape)
    : _shape(shape),
    _coord(0, 0)
{
    // TODO 0 size grid is awkward...
    assert(_shape.first > 0 && _shape.second > 0);
}

inline grid_location::grid_location(const int_pair& shape, const int_pair& coord)
    : _shape(shape),
    _coord(coord)
{
    // TODO 0 size grid is awkward...
    assert(_shape.first > 0 && _shape.second > 0);
    assert(coord_in_shape(_coord, _shape));
}

inline grid_location::grid_location(const int_pair& shape, int point)
    : _shape(shape),
    _coord(point_to_coord(point, shape))
{
    // TODO 0 size grid is awkward...
    assert(_shape.first > 0 && _shape.second > 0);
    assert(coord_in_shape(_coord, _shape));
}

inline const int_pair& grid_location::get_shape() const
{
    return _shape;
}

inline const int_pair& grid_location::get_coord() const
{
    return _coord;
}

inline int grid_location::get_point() const
{
    return coord_to_point(_coord, _shape);
}

inline void grid_location::set_shape(const int_pair& shape)
{
    _shape = shape;
    assert(coord_in_shape(_coord, _shape));
}

inline void grid_location::set_coord(const int_pair& coord)
{
    _coord = coord;
    assert(coord_in_shape(_coord, _shape));
}

inline void grid_location::set_point(int point)
{
    _coord = point_to_coord(point, _shape);
    assert(coord_in_shape(_coord, _shape));
}

inline bool grid_location::get_neighbor_coord(int_pair& neighbor_coord, grid_dir direction) const
{
    return get_neighbor_coord(neighbor_coord, _coord, direction, _shape);
}

inline bool grid_location::get_neighbor_point(int& neighbor_point, grid_dir direction) const
{
    return get_neighbor_point(neighbor_point, _coord, direction, _shape);
}

inline bool grid_location::move(grid_dir direction)
{
    return get_neighbor_coord(_coord, _coord, direction, _shape);
}

inline bool grid_location::coord_in_shape(const int_pair& coord, const int_pair& shape)
{
    return                                 //
        (coord.first >= 0) &&              //
        (coord.first < shape.first) &&    //
        (coord.second >= 0) &&             //
        (coord.second < shape.second);    //
}

inline bool grid_location::point_in_shape(int point, const int_pair& shape)
{
    const int grid_size = shape.first * shape.second;
    return (point >= 0) && (point < grid_size);
}

inline int grid_location::coord_to_point(const int_pair& coord, const int_pair& shape)
{
    assert(coord_in_shape(coord, shape));
    return coord.first * shape.second + coord.second;
}

inline int_pair grid_location::point_to_coord(int point, const int_pair& shape)
{
    assert(point_in_shape(point, shape));
    int r = point / shape.second;
    int c = point % shape.second;
    return {r, c};
}

