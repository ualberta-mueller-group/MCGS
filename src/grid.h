//---------------------------------------------------------------------------
// A game board which is a 2-dimensional grid
// Board representation, play and remove stones
//---------------------------------------------------------------------------
#pragma once

#include "cgt_basics.h"
#include "game.h"
#include <vector>
#include <utility>
#include <string>
#include <cassert>
#include <ostream>

//---------------------------------------------------------------------------

// (row, col)
typedef std::pair<int, int> int_pair;

class grid : public game
{
public:
    grid(int n_rows, int n_cols);
    grid(const std::vector<int>& board, int_pair shape);
    grid(const std::string& game_as_string);
    int size() const;
    int at(int p) const;
    const int_pair& shape() const;

    // is p on board, and of given color?
    bool checked_is_color(int p, int color) const;
    void play_stone(int p, int color);
    void remove_stone(int p);

    // replaces whatever is there.
    // Less checking than play_stone or remove_stone
    void replace(int p, int color);
    std::vector<int> inverse_board() const;
    std::string board_as_string() const;
    const std::vector<int>& board_const() const;

    int_pair point_to_coord(int p) const;
    int coord_to_point(int_pair coord) const;

    bool coord_in_bounds(const int_pair& coord) const;

    std::vector<int> board() const;

protected:
    void _init_hash(local_hash& hash) const override;
    relation _order_impl(const game* rhs) const override;

    static relation _compare_grids(const grid& g1, const grid& g2);

private:
    void _check_legal() const;

    // _board is row-major.
    std::vector<int> _board; // todo try char as well.
    int_pair _shape;         // (n_rows, n_cols)
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
    return in_range(p, 0, size()) //
           && _board[p] == color; //
}

inline void grid::play_stone(int p, int color)
{
    assert_range(p, 0, size());
    assert_black_white(color);
    assert(_board[p] == EMPTY);
    _board[p] = color;
}

inline void grid::remove_stone(int p)
{
    assert_range(p, 0, size());
    assert(_board[p] != EMPTY);
    _board[p] = EMPTY;
}

inline void grid::replace(int p, int color)
{
    assert_range(p, 0, size());
    assert_empty_black_white(color);
    _board[p] = color;
}

inline int_pair grid::point_to_coord(int p) const
{
    int r = p / _shape.second;
    int c = p % _shape.second;
    return {r, c};
}

inline int grid::coord_to_point(int_pair coord) const
{
    return coord.first * _shape.second + coord.second;
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

inline std::ostream& operator<<(std::ostream& os, const int_pair& pr)
{
    os << '(' << pr.first << ' ' << pr.second << ')';
    return os;
}

//---------------------------------------------------------------------------
