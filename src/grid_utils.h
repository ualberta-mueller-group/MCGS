#pragma once

#include <array>
#include "grid.h"

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
    bool get_neighbor(int_pair& neighbor, grid_dir direction) const;

    // mutators
    bool move(grid_dir direction);
    bool increment_position();

    // static utility functions
    static bool coord_in_shape(const int_pair& coord, const int_pair& shape);
    static bool point_in_shape(int point, const int_pair& shape);

    static int coord_to_point(const int_pair& coord, const int_pair& shape);
    static int_pair point_to_coord(int point, const int_pair& shape);

    static bool get_neighbor(int_pair& neighbor, const int_pair& coord, grid_dir direction, const int_pair& shape);

private:
    int_pair _shape;
    int_pair _coord;
};


