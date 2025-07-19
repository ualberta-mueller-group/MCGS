#pragma once

/*
    Helpers for grids. Defines grid_location and grid_generator
*/

#include <array>
#include "grid.h"
#include "throw_assert.h"
#include <cassert>
#include <cstddef>
#include <string>

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

static constexpr std::array<int_pair, 9> _GRID_DISPLACEMENTS {{
    {-1, 0},
    {-1, 1},
    {0, 1},
    {1, 1},
    {1, 0},
    {1, -1},
    {0, -1},
    {-1, -1},
    {0, 0},
}};

static_assert(_GRID_DISPLACEMENTS[GRID_DIR_UP] == int_pair(-1, 0));
static_assert(_GRID_DISPLACEMENTS[GRID_DIR_UP_RIGHT] == int_pair(-1, 1));
static_assert(_GRID_DISPLACEMENTS[GRID_DIR_RIGHT] == int_pair(0, 1));
static_assert(_GRID_DISPLACEMENTS[GRID_DIR_DOWN_RIGHT] == int_pair(1, 1));
static_assert(_GRID_DISPLACEMENTS[GRID_DIR_DOWN] == int_pair(1, 0));
static_assert(_GRID_DISPLACEMENTS[GRID_DIR_DOWN_LEFT] == int_pair(1, -1));
static_assert(_GRID_DISPLACEMENTS[GRID_DIR_LEFT] == int_pair(0, -1));
static_assert(_GRID_DISPLACEMENTS[GRID_DIR_UP_LEFT] == int_pair(-1, -1));
static_assert(_GRID_DISPLACEMENTS[GRID_DIR_NO_DIRECTION] == int_pair(0, 0));

static constexpr std::array<grid_dir, 4> GRID_DIRS_CARDINAL {
    GRID_DIR_UP,
    GRID_DIR_RIGHT,
    GRID_DIR_DOWN,
    GRID_DIR_LEFT,
};

static constexpr std::array<grid_dir, 4> GRID_DIRS_DIAGONAL {
    GRID_DIR_UP_RIGHT, GRID_DIR_DOWN_RIGHT, GRID_DIR_DOWN_LEFT,
    GRID_DIR_UP_LEFT};

static constexpr std::array<grid_dir, 8> GRID_DIRS_ALL {
    GRID_DIR_UP,   GRID_DIR_UP_RIGHT,  GRID_DIR_RIGHT, GRID_DIR_DOWN_RIGHT,
    GRID_DIR_DOWN, GRID_DIR_DOWN_LEFT, GRID_DIR_LEFT,  GRID_DIR_UP_LEFT};

////////////////////////////////////////////////// class grid_location
class grid_location
{
public:
    // constructors
    grid_location(const int_pair& shape); // default location (0, 0)
    grid_location(const int_pair& shape, const int_pair& coord);
    grid_location(const int_pair& shape, int point);

    // getters
    const int_pair& get_shape() const;
    const int_pair& get_coord() const;
    int get_point() const;
    bool valid() const;
    bool is_empty() const;

    // setters
    void set_shape(const int_pair& shape);
    void set_coord(const int_pair& coord);
    void set_point(int point);

    // utility
    bool get_neighbor_coord(int_pair& neighbor_coord, grid_dir direction) const;
    bool get_neighbor_point(int& neighbor_point, grid_dir direction) const;

    // mutators
    bool move(grid_dir direction);
    void reset_position();
    // precondition: valid() is true. May invalidate position
    void increment_position();

    // static utility functions
    static bool shape_is_empty(const int_pair& shape);
    static bool coord_in_shape(const int_pair& coord, const int_pair& shape);
    static bool point_in_shape(int point, const int_pair& shape);

    static int coord_to_point(const int_pair& coord, const int_pair& shape);
    static int_pair point_to_coord(int point, const int_pair& shape);

    static bool get_neighbor_coord(int_pair& neighbor_coord,
                                   const int_pair& coord, grid_dir direction,
                                   const int_pair& shape);

    static bool get_neighbor_point(int& neighbor_point, const int_pair& coord,
                                   grid_dir direction, const int_pair& shape);

private:
    int_pair _shape;
    int_pair _coord;
};

////////////////////////////////////////////////// class grid_generator
class grid_generator
{
public:
    // strips
    grid_generator(size_t n_cols);

    // grids
    grid_generator(size_t n_rows, size_t n_cols);
    grid_generator(const int_pair& dims);

    operator bool() const;
    const std::string& gen_board() const;

    void operator++();

    static std::string get_empty_board(size_t rows, size_t cols);

    const int_pair get_shape() const;

private:
    bool _increment_board();
    void _increment_dimensions();
    bool _has_zero_area() const;

    const size_t _n_rows;
    const size_t _n_cols;

    size_t _current_rows;
    size_t _current_cols;

    std::string _board;
};

////////////////////////////////////////////////// grid_location implementation
inline grid_location::grid_location(const int_pair& shape)
    : _shape(shape), _coord(0, 0)
{
    THROW_ASSERT_DEBUG(_shape.first >= 0 && _shape.second >= 0);
    THROW_ASSERT_DEBUG(is_empty() || valid());
}

inline grid_location::grid_location(const int_pair& shape,
                                    const int_pair& coord)
    : _shape(shape), _coord(shape_is_empty(shape) ? int_pair(0, 0) : coord)
{
    THROW_ASSERT_DEBUG(_shape.first >= 0 && _shape.second >= 0);
    THROW_ASSERT_DEBUG(is_empty() || valid());
}

inline grid_location::grid_location(const int_pair& shape, int point)
    : _shape(shape),
      _coord(shape_is_empty(shape) ? int_pair(0, 0)
                                   : point_to_coord(point, shape))
{
    THROW_ASSERT_DEBUG(_shape.first >= 0 && _shape.second >= 0);
    THROW_ASSERT_DEBUG(is_empty() || valid());
}

inline const int_pair& grid_location::get_shape() const
{
    return _shape;
}

inline const int_pair& grid_location::get_coord() const
{
    THROW_ASSERT_DEBUG(valid());
    return _coord;
}

inline bool grid_location::valid() const
{
    return coord_in_shape(_coord, _shape);
}

inline bool grid_location::is_empty() const
{
    return shape_is_empty(_shape);
}

inline int grid_location::get_point() const
{
    THROW_ASSERT_DEBUG(valid());
    return coord_to_point(_coord, _shape);
}

inline void grid_location::set_shape(const int_pair& shape)
{
    _shape = shape;
    THROW_ASSERT_DEBUG(valid());
}

inline void grid_location::set_coord(const int_pair& coord)
{
    _coord = coord;
    THROW_ASSERT_DEBUG(valid());
}

inline void grid_location::set_point(int point)
{
    _coord = point_to_coord(point, _shape);
    THROW_ASSERT_DEBUG(valid());
}

inline bool grid_location::get_neighbor_coord(int_pair& neighbor_coord,
                                              grid_dir direction) const
{
    THROW_ASSERT_DEBUG(valid());
    return get_neighbor_coord(neighbor_coord, _coord, direction, _shape);
}

inline bool grid_location::get_neighbor_point(int& neighbor_point,
                                              grid_dir direction) const
{
    THROW_ASSERT_DEBUG(valid());
    return get_neighbor_point(neighbor_point, _coord, direction, _shape);
}

inline bool grid_location::move(grid_dir direction)
{
    THROW_ASSERT_DEBUG(valid());
    return get_neighbor_coord(_coord, _coord, direction, _shape);
}

inline void grid_location::reset_position()
{
    _coord = {0, 0};
    THROW_ASSERT_DEBUG(valid());
}

inline bool grid_location::shape_is_empty(const int_pair& shape)
{
    return (shape.first <= 0) || (shape.second <= 0);
}

inline bool grid_location::coord_in_shape(const int_pair& coord,
                                          const int_pair& shape)
{
    return                             //
        (!shape_is_empty(shape)) &&    //
        (coord.first >= 0) &&          //
        (coord.first < shape.first) && //
        (coord.second >= 0) &&         //
        (coord.second < shape.second); //
}

inline bool grid_location::point_in_shape(int point, const int_pair& shape)
{
    const int grid_size = shape.first * shape.second;
    return !shape_is_empty(shape) && (point >= 0) && (point < grid_size);
}

inline int grid_location::coord_to_point(const int_pair& coord,
                                         const int_pair& shape)
{
    THROW_ASSERT_DEBUG(coord_in_shape(coord, shape));
    return coord.first * shape.second + coord.second;
}

inline int_pair grid_location::point_to_coord(int point, const int_pair& shape)
{
    THROW_ASSERT_DEBUG(point_in_shape(point, shape));
    int r = point / shape.second;
    int c = point % shape.second;
    return {r, c};
}

////////////////////////////////////////////////// grid_generator implementation
inline grid_generator::grid_generator(size_t n_cols)
    : _n_rows(1), _n_cols(n_cols), _current_rows(0), _current_cols(0)
{
    assert(_n_rows >= 0 && _n_cols >= 0);
}

inline grid_generator::grid_generator(size_t n_rows, size_t n_cols)
    : _n_rows(n_rows), _n_cols(n_cols), _current_rows(0), _current_cols(0)
{
    assert(_n_rows >= 0 && _n_cols >= 0);
}

inline grid_generator::grid_generator(const int_pair& dims)
    : _n_rows(dims.first),
      _n_cols(dims.second),
      _current_rows(0),
      _current_cols(0)
{
    assert(_n_rows >= 0 && _n_cols >= 0);
}

inline grid_generator::operator bool() const
{
    return (_current_rows <= _n_rows) && (_current_cols <= _n_cols);
}

inline const std::string& grid_generator::gen_board() const
{
    assert(*this);
    return _board;
}

inline const int_pair grid_generator::get_shape() const
{
    return {_current_rows, _current_cols};
}

inline bool grid_generator::_has_zero_area() const
{
    return (_current_rows <= 0) || (_current_cols <= 0);
}
