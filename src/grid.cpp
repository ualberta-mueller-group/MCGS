//---------------------------------------------------------------------------
// A game board which is a 2-dimensional grid
//---------------------------------------------------------------------------
#include "grid.h"

#include "cgt_basics.h"
#include "throw_assert.h"
#include "strip.h"

//---------------------------------------------------------------------------

namespace {

    void check_is_valid_char(char c)
    {
        THROW_ASSERT(c == 'X' || c == 'O' || c == '.' || c == '|');
    }

    int char_to_color(char c)
    {
        if (c == 'X')
            return BLACK;
        else if (c == 'O')
            return WHITE;
        else if (c == '.')
            return EMPTY;
        else if (c == '|')
            return BORDER;
        else
            assert(false);

        exit(-1);
        return -1;
    }

    int color_to_char(int color)
    {
        static char clobber_char[] = {'X', 'O', '.', '|'};

        assert_range(color, BLACK, BORDER + 1);
        return clobber_char[color];
    }
    
    std::pair<std::vector<int>, int_pair> string_to_board(const std::string& game_as_string)
    {
        std::vector<int> board;
        int n_rows = 0, n_cols = 0, counter = 0;
        for (auto c : game_as_string)
        {
            check_is_valid_char(c);
            int color = char_to_color(c);
            if (color == BORDER)
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
                result += color_to_char(board[r*n_cols+c]);
            }
            if (r != shape.first - 1)
                result += '|';
        }
        return result;
    }
    
} // namespace

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

void grid::_check_legal() const
{
    for (const int& x : _board)
        THROW_ASSERT(x == BLACK || x == WHITE || x == EMPTY);
}

std::vector<int> grid::inverse_board() const
{
    std::vector<int> new_board = _board;
    for (int& p : new_board)
    {
        p = ebw_opponent(p);
    }
    return new_board;
}
