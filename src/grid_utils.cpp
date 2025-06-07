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

////////////////////////////////////////////////// grid_generator functions
void grid_generator::operator++()
{
    assert(*this);
    if (!_increment_board())
    {
        _increment_dimensions();
        _board = get_empty_board(_current_rows, _current_cols);
    }
}

std::string grid_generator::get_empty_board(size_t rows, size_t cols)
{
    assert(rows >= 0 && cols >= 0);

    std::string board;
    if (rows == 0 || cols == 0)
        return board;

    board.reserve(rows * cols + (rows - 1));

    for (size_t r = 0; r < rows; r++)
    {
        for (size_t c = 0; c < cols; c++)
            board.push_back('.');

        if (r + 1 < rows)
            board.push_back('|');
    }

    return board;
}

bool grid_generator::_increment_board()
{
    assert(*this);

    bool carry = true;
    for (auto it = _board.rbegin(); it != _board.rend(); it++)
    {
        if (!carry)
            break;

        if (*it == '|')
            continue;

        carry = !increment_char(*it);
    }

    return !carry;
}

void grid_generator::_increment_dimensions()
{
    assert(*this);

    if (_has_zero_area())
    {
        _current_rows = 1;
        _current_cols = 1;
        return;
    }

    _current_cols++;

    if (_current_cols > _n_cols)
    {
        _current_rows++;
        _current_cols = 1;
    }
}
