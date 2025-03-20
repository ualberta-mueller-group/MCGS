//---------------------------------------------------------------------------
// A game board which is a 1-dimensional strip
//---------------------------------------------------------------------------
#include "strip.h"

#include "cgt_basics.h"
#include "throw_assert.h"

//---------------------------------------------------------------------------

int clobber_char_to_color(char c)
{
    if (c == 'X')
        return BLACK;
    else if (c == 'O')
        return WHITE;
    else if (c == '.')
        return EMPTY;
    else
        assert(false);

    exit(-1);
    return -1;
}

char color_to_clobber_char(int color)
{
    static char clobber_char[] = {'X', 'O', '.'};

    assert_range(color, BLACK, EMPTY + 1);
    return clobber_char[color];
}

namespace {

void check_is_clobber_char(char c)
{
    THROW_ASSERT(c == 'X' || c == 'O' || c == '.');
}

std::vector<int> string_to_board(const std::string& game_as_string)
{
    std::vector<int> board;
    for (auto c : game_as_string)
    {
        check_is_clobber_char(c);
        board.push_back(clobber_char_to_color(c));
    }
    return board;
}

std::string board_to_string(const std::vector<int>& board)
{
    std::string result;
    for (int p : board)
        result += color_to_clobber_char(p);
    return result;
}

template <const bool mirror>
inline std::vector<int> inverse_board_impl(
    const std::vector<int>& original_board)
{
    std::vector<int> new_board(original_board.size());

    const size_t N = original_board.size();
    assert(new_board.size() == N);

    for (size_t i = 0; i < N; i++)
    {
        int x;
        if constexpr (mirror)
            x = original_board[N - 1 - i];
        else
            x = original_board[i];

        new_board[i] = ebw_opponent(x);
    }

    return new_board;
}

} // namespace

//---------------------------------------------------------------------------

strip::strip(const std::vector<int>& board) : game(), _board(board)
{
    _check_legal();
}

strip::strip(const std::string& game_as_string)
    : strip(string_to_board(game_as_string))
{
}

std::string strip::board_as_string() const
{
    return board_to_string(_board);
}

void strip::_check_legal() const
{
    for (const int& x : _board)
        THROW_ASSERT(x == BLACK || x == WHITE || x == EMPTY);
}

std::vector<int> strip::inverse_board() const
{
    return inverse_board_impl<false>(_board);
}

std::vector<int> strip::inverse_mirror_board() const
{
    return inverse_board_impl<true>(_board);
}
