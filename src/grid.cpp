//---------------------------------------------------------------------------
// A game board which is a 2-dimensional grid
//---------------------------------------------------------------------------
#include "grid.h"

#include <cassert>
#include <cstdlib>
#include <utility>
#include <vector>
#include "cgt_basics.h"
#include "grid_location.h"
#include "parsing_utilities.h"
#include "strip.h"
#include "throw_assert.h"
#include "utilities.h"
#include "warn_default.h"

//---------------------------------------------------------------------------

namespace {
const int ROW_SEP = 4; // row separator

void check_is_valid_char(char c)
{
    THROW_ASSERT(c == 'X' || c == 'O' || c == '.' || c == '#' || c == '|');
}

int char_to_color(char c)
{
    if (c == 'X')
        return BLACK;
    else if (c == 'O')
        return WHITE;
    else if (c == '.')
        return EMPTY;
    else if (c == '#')
        return BORDER;
    else if (c == '|')
        return ROW_SEP;
    else
        assert(false);

    exit(-1);
    return -1;
}

int color_to_char(int color)
{
    static char clobber_char[] = {'X', 'O', '.', '#', '|'};

    assert_range(color, BLACK, ROW_SEP + 1);
    return clobber_char[color];
}

std::pair<std::vector<int>, int_pair> string_to_board(
    const std::string& game_as_string)
{
    std::vector<int> board;
    int n_rows = 0, n_cols = 0, counter = 0;
    for (const char& c : game_as_string)
    {
        check_is_valid_char(c);
        int color = char_to_color(c);
        if (color == ROW_SEP)
        {
            n_rows++;
            if (n_cols == 0)
                n_cols = counter;
            else
                THROW_ASSERT(n_cols == counter);
            counter = 0;
        }
        else
        {
            board.push_back(color);
            counter++;
        }
    }
    n_rows++;
    if (n_cols == 0)
        n_cols = counter;
    else
        THROW_ASSERT(n_cols == counter);

    int_pair shape = {n_rows, n_cols};
    return {board, shape};
}

std::string board_to_string(const std::vector<int>& board, const int_pair shape)
{
    std::string result;
    int n_cols = shape.second;
    for (int r = 0; r < shape.first; r++)
    {
        for (int c = 0; c < shape.second; c++)
        {
            result += color_to_char(board[r * n_cols + c]);
        }
        if (r != shape.first - 1)
            result += '|';
    }
    return result;
}

inline char coord(int value) // 0 -> 'a', 1 -> 'b', etc.
{
    return 'a' + value;
}

} // namespace

// TODO make it faster?
std::pair<std::vector<int>, int_pair> string_to_int_grid(
    const std::string& game_as_string)
{
    // Declare first to allow copy elision?
    std::pair<std::vector<int>, int_pair> ret_pair;

    std::vector<int>& board = ret_pair.first;
    int_pair& shape = ret_pair.second;

    const std::string SEP_STRING = std::string(1, color_to_char(ROW_SEP));

    int n_rows = 0;
    int n_cols = 0;
    int counter = 0;
    bool prev_was_control = false; // previous token was ',' or '|'

    std::vector<std::string> tokens = get_string_tokens(game_as_string, {',', '|'});
    const size_t N_TOKENS = tokens.size();

    for (size_t idx = 0; idx < N_TOKENS; idx++)
    {
        const std::string& token = tokens[idx];

        if (token == ",") // comma
        {
            THROW_ASSERT(!prev_was_control);
            prev_was_control = true;
            continue;
        }

        if (token == SEP_STRING) // ROW_SEP
        {
            THROW_ASSERT(!prev_was_control);
            prev_was_control = true;

            n_rows++;
            if (n_cols == 0)
                n_cols = counter;
            else
                THROW_ASSERT(n_cols == counter);
            counter = 0;
        }
        else // integer
        {
            prev_was_control = false;

            THROW_ASSERT(is_int(token));
            const int val = atoi(token.c_str());

            board.push_back(val);
            counter++;
        }
    }

    n_rows++;
    if (n_cols == 0)
        n_cols = counter;
    else
        THROW_ASSERT(n_cols == counter);

    shape = {n_rows, n_cols};
    return ret_pair;
}


//---------------------------------------------------------------------------

grid::grid(int n_rows, int n_cols) : game()
{
    _shape.first = n_rows;
    _shape.second = n_cols;
    _board = std::vector<int>(n_rows * n_cols, EMPTY);
}

grid::grid(const std::vector<int>& board, int_pair shape)
    : game(), _board(board), _shape(shape)
{
    _check_legal();
}

grid::grid(const std::pair<std::vector<int>, int_pair>& board_pair)
    : game(),
    _board(board_pair.first),
    _shape(board_pair.second)
{
    assert(_shape.first >= 0 &&  //
           _shape.second >= 0 && //
           _board.size() ==
               static_cast<unsigned int>(_shape.first * _shape.second) //
    );
}

grid::grid(const std::string& game_as_string) : game()
{
    std::pair<std::vector<int>, int_pair> board_shape =
        string_to_board(game_as_string);
    _board = board_shape.first;
    _shape = board_shape.second;
}

std::string grid::board_as_string() const
{
    return board_to_string(_board, _shape);
}

const std::vector<int>& grid::board_const() const
{
    return _board;
}

void grid::_init_hash(local_hash& hash) const
{
    WARN_DEFAULT_IMPL();
    hash.toggle_value(0, _shape.first);
    hash.toggle_value(1, _shape.second);

    const size_t N = this->size();

    for (size_t i = 0; i < N; i++)
        hash.toggle_value(i + 2, this->at(i));
}

relation grid::_order_impl(const game* rhs) const
{
    WARN_DEFAULT_IMPL();
    assert(game_type() == rhs->game_type());

    const grid* other = reinterpret_cast<const grid*>(rhs);
    assert(dynamic_cast<const grid*>(rhs) == other);

    return _compare_grids(*this, *other);
}

relation grid::_compare_grids(const grid& g1, const grid& g2)
{
    const int_pair& shape1 = g1._shape;
    const int_pair& shape2 = g2._shape;

    if (shape1.first != shape2.first)
        return shape1.first < shape2.first ? REL_LESS : REL_GREATER;

    if (shape1.second != shape2.second)
        return shape1.second < shape2.second ? REL_LESS : REL_GREATER;

    const int size1 = g1.size();
    const int size2 = g2.size();
    assert(size1 == size2);

    assert(size1 >= 0 && size2 >= 0);
    assert((unsigned int) size1 == g1._board.size());
    assert((unsigned int) size2 == g2._board.size());

    for (int i = 0; i < size1; i++)
    {
        const int& val1 = g1._board[i];
        const int& val2 = g2._board[i];

        if (val1 != val2)
            return val1 < val2 ? REL_LESS : REL_GREATER;
    }

    return REL_EQUAL;
}

void grid::_check_legal() const
{
    return;
    // TODO resolve int/stone board problem

    /*
    for (const int& x : _board)
        THROW_ASSERT(x == BLACK ||  //
                     x == WHITE ||  //
                     x == EMPTY ||  //
                     x == BORDER || //
                     x == ROW_SEP); //
    */
}

std::vector<int> grid::inverse_board() const
{
    std::vector<int> new_board = _board;
    for (int& p : new_board)
    {
        if (p != BORDER)
            p = ebw_opponent(p);
    }
    return new_board;
}

std::vector<int> grid::get_transpose_board(int_pair& new_shape)
{
    const int_pair& s = shape();

    new_shape.first = s.second;
    new_shape.second = s.first;

    return transpose_board(board_const(), s);
}

// TODO unit test this
std::vector<int> grid::transpose_board(const std::vector<int>& board,
                                       const int_pair& shape)
{
    std::vector<int> new_board;

    const size_t board_n = board.size();
    new_board.reserve(board_n);

    const int n_rows = shape.first;
    const int n_cols = shape.second;

    for (int col = 0; col < n_cols; col++)
    {
        int idx = col;

        for (int row = 0; row < n_rows; row++)
        {
            new_board.push_back(board[idx]);
            idx += n_cols;
        }
    }

    return new_board;
}

std::string grid::point_coord_as_string(int point) const
{
    std::string result;
    int_pair rc = grid_location::point_to_coord(point, shape());
    result += coord(rc.second);
    // From 0-based internal to 1-based external coordinates
    result += std::to_string(rc.first + 1); 
    return result;
}
