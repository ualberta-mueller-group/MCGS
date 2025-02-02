//---------------------------------------------------------------------------
// A game board which is a 1-dimensional strip
//---------------------------------------------------------------------------
#include "strip.h"

#include "cgt_basics.h"
//---------------------------------------------------------------------------

using std::vector;

int clobber_char_to_color(char c)
{
    if (c == 'X')
        return BLACK;
    else if (c == 'O')
        return WHITE;
    else if (c == '.')
        return EMPTY;
    else assert(false);

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

void assert_is_clobber_char(char c)
{
    assert(c == 'X' || c == 'O' || c == '.');
}



vector<int> string_to_board(const std::string& game_as_string)
{
    vector<int> board;
    for(auto c: game_as_string)
    {
        assert_is_clobber_char(c);
        board.push_back(clobber_char_to_color(c));
    }
    return board;
}

std::string board_to_string(const vector<int>& board)
{
    std::string result;
    for(int p: board)
        result += color_to_clobber_char(p);
    return result;
}

} // namespace
//---------------------------------------------------------------------------

strip::strip(const vector<int>& board) :
    game(),
    _board(board)
{ }

strip::strip(const std::string& game_as_string) :
    strip(string_to_board(game_as_string))
{ }

std::string strip::board_as_string() const
{
    return board_to_string(_board);
}

vector<int> strip::inverse_board() const
{
    vector<int> inv_board(_board);
    for (auto i = 0; i < size(); ++i)
    {
        inv_board[i] = ebw_opponent(_board[i]);
    }
    return inv_board;
}
