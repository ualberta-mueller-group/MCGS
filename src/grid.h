//---------------------------------------------------------------------------
// A game board which is a 2-dimensional grid
// Board representation, play and remove stones
//---------------------------------------------------------------------------
#pragma once

// IWYU pragma: begin_exports
#include "grid_location.h"
#include "int_pair.h"
// IWYU pragma: end_exports

#include "cgt_basics.h"
#include "game.h"
#include "safe_arithmetic.h"
#include <vector>
#include <utility>
#include <string>
#include <cassert>
#include <ostream>

//---------------------------------------------------------------------------
// NOTE: subclass should call one of the _is_legal_XYZ() methods


enum grid_type_enum
{
    GRID_TYPE_COLOR = 0,
    GRID_TYPE_NUMBER,
};

class grid : public game
{
public:
    grid(int n_rows, int n_cols, grid_type_enum grid_type);

    // TODO rvalue reference version?
    grid(const std::vector<int>& board, int_pair shape, grid_type_enum grid_type);
    grid(const std::pair<std::vector<int>, int_pair>& board_pair, grid_type_enum grid_type);
    grid(const std::string& game_as_string, grid_type_enum grid_type);

    int size() const;
    int at(int p) const;
    const int_pair& shape() const;

    // is p on board, and of given color?
    bool checked_is_color(int p, int color) const;
    void play_stone(int p, int color);
    void remove_stone(int p);

    // replaces whatever is there.
    // Less checking than play_stone or remove_stone
    void replace_int(int p, int value);
    void replace(int p, int color); // TODO rename to replace_color?
    std::vector<int> inverse_board() const;
    std::vector<int> inverse_number_board() const;
    std::string board_as_string() const;
     // Point in format such as "g5", with rows a,b..., columns 1, 2...
    std::string point_coord_as_string(int p) const;
    std::string board_as_number_string() const;
    const std::vector<int>& board_const() const;

    bool coord_in_bounds(const int_pair& coord) const;

    std::vector<int> board() const;

    std::vector<int> get_transpose_board(int_pair& new_shape);

    static std::vector<int> transpose_board(const std::vector<int>& board,
                                            const int_pair& shape);

    static std::vector<int> rotate_90_board(const std::vector<int>& board,
                                            const int_pair& shape);

protected:
    void _init_hash(local_hash& hash) const override;
    relation _order_impl(const game* rhs) const override;

    static relation _compare_grids(const grid& g1, const grid& g2);


private:
    bool _is_legal_grid();
    bool _is_legal_color_grid();
    bool _is_legal_number_grid();

    // _board is row-major.
    std::vector<int> _board; // todo try char as well.
    int_pair _shape;         // (n_rows, n_cols)

    const grid_type_enum _grid_type;
};

inline int grid::size() const
{
    return _board.size();
}

inline int grid::at(int p) const
{
    assert_range(p, 0, size());
    return _board[p];
}

inline const int_pair& grid::shape() const
{
    return _shape;
}

inline bool grid::checked_is_color(int p, int color) const
{
    assert(_grid_type == GRID_TYPE_COLOR);

    return in_range(p, 0, size()) //
           && _board[p] == color; //
}

inline void grid::play_stone(int p, int color)
{
    assert(_grid_type == GRID_TYPE_COLOR);

    assert_range(p, 0, size());
    assert(is_stone_color(color) && _board[p] == EMPTY);
    _board[p] = color;
}

inline void grid::remove_stone(int p)
{
    assert(_grid_type == GRID_TYPE_COLOR);

    assert_range(p, 0, size());
    assert(is_stone_color(_board[p]));
    _board[p] = EMPTY;
}

inline void grid::replace_int(int p, int value)
{
    assert(_grid_type == GRID_TYPE_NUMBER);

    assert_range(p, 0, size());
    assert(negate_is_safe(value));
    _board[p] = value;
}

inline void grid::replace(int p, int color)
{
    assert(_grid_type == GRID_TYPE_COLOR);
    assert_range(p, 0, size());
    assert(is_empty_or_stone_color(color));
    _board[p] = color;
}

inline bool grid::coord_in_bounds(const int_pair& coord) const
{
    return                              //
        (coord.first >= 0) &&           //
        (coord.first < _shape.first) && //
        (coord.second >= 0) &&          //
        (coord.second < _shape.second); //
}

inline std::vector<int> grid::board() const
{
    return _board;
}

////////////////////////////////////////////////// Helpers
// TODO make these static members of grid...
std::pair<std::vector<int>, int_pair> string_to_grid(
    const std::string& game_as_string);

std::pair<std::vector<int>, int_pair> string_to_int_grid(
    const std::string& game_as_string);
