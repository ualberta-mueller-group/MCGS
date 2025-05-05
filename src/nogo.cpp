//---------------------------------------------------------------------------
// Implementation of NoGo on a 2-dimensional rectangular board
//---------------------------------------------------------------------------
#include "nogo.h"

#include "cgt_basics.h"
#include "game.h"
#include "grid.h"
#include <memory>

//////////////////////////////////////// nogo
nogo::nogo(std::string game_as_string) : grid(game_as_string)
{
}

nogo::nogo(const std::vector<int>& board, int_pair shape)
    : grid(board, shape)
{
}

void nogo::play(const move& m, bw to_play)
{
    game::play(m, to_play);

    const int to = m;
    assert(at(to) == EMPTY);
    replace(to, to_play);
}

void nogo::undo_move()
{
    const move mc = last_move();
    game::undo_move();

    const int to = cgt_move::decode(mc);
    const bw player = cgt_move::get_color(mc);
    assert(at(to) == player);
    replace(to, EMPTY);
}

split_result nogo::_split_impl() const
{
    return split_result();  // no split
}

game* nogo::inverse() const
{
    return new nogo(inverse_board(), shape());
}

std::ostream& operator<<(std::ostream& out, const nogo& g)
{
    out << g.board_as_string();
    return out;
}

//////////////////////////////////////// nogo_rule
bool nogo_rule::is_legal(nogo_board nboard, int p, int toplay)
{
    std::vector<int>& board = nboard.board;

    if (board[p] != EMPTY)
        return false;

    board[p] = toplay;
    int opp_color = 1 - toplay;

    if (!nogo_rule::has_liberty(nboard, p))
    {
        board[p] = EMPTY;
        return false;
    }

    std::vector<int> nbrs = nogo_rule::neighbors(nboard, p);
    for (int nbr : nbrs)
    {
        if (board[nbr] == opp_color && !nogo_rule::has_liberty(nboard, nbr))
        {
            board[p] = EMPTY;
            return false;
        }
    }

    board[p] = EMPTY;
    return true;
}

bool nogo_rule::has_liberty(const nogo_board& nboard, int p)
{
    const std::vector<int>& board = nboard.board;

    int size = (int) board.size();
    std::vector<bool> markers = std::vector<bool>(size, false);

    int color = board[p];
    std::vector<int> point_stack = { p };
    markers[p] = true;

    while (!point_stack.empty())
    {
        int p = point_stack.back();
        point_stack.pop_back();

        std::vector<int> nbrs = nogo_rule::neighbors(nboard, p);
        for (int nbr : nbrs)
        {
            if (board[nbr] == EMPTY)
                return true;
            if (board[nbr] == color && !markers[nbr])
            {
                point_stack.push_back(nbr);
                markers[nbr] = true;
            }
        }
    }

    return false;
}

std::vector<int> nogo_rule::neighbors(const nogo_board& nboard, int p)
{
    const std::vector<int> board = nboard.board;
    int n_rows = nboard.shape.first;
    int n_cols = nboard.shape.second;

    int r = p / nboard.shape.second;
    int c = p % nboard.shape.second;

    std::vector<int> nbrs;
    if (r > 0 && board[p - n_cols] != BORDER)
        nbrs.push_back(p - n_cols); // up
    if (r < n_rows-1 && board[p + n_cols] != BORDER)
        nbrs.push_back(p + n_cols); // down
    if (c > 0 && board[p - 1] != BORDER)
        nbrs.push_back(p - 1);      // left
    if (c < n_cols-1 && board[p + 1] != BORDER)
        nbrs.push_back(p + 1);      // right
    
    return nbrs;
}

//---------------------------------------------------------------------------

class nogo_move_generator : public move_generator
{
public:
    nogo_move_generator(const nogo& game, bw to_play);
    void operator++() override;
    operator bool() const override;
    move gen_move() const override;

private:
    void _find_next_move();
    bool _is_legal();

    const nogo& _game;
    int _current; // current stone location to test
};

inline nogo_move_generator::nogo_move_generator(const nogo& game,
                                                bw to_play)
    : move_generator(to_play), _game(game), _current(0)
{
    if (_game.size() > 0 && !_is_legal())
    {
        _find_next_move();
    }
}

void nogo_move_generator::operator++()
{
    _find_next_move();
}

inline void nogo_move_generator::_find_next_move()
{
    _current++;

    int num = (int) _game.size();
    while (_current < num && !_is_legal())
    {
        _current++;
    }
}

inline bool nogo_move_generator::_is_legal()
{
    return nogo_rule::is_legal({_game.board(), _game.shape()}, _current, to_play());
}

nogo_move_generator::operator bool() const
{
    return _current < _game.size();
}

move nogo_move_generator::gen_move() const
{
    assert(operator bool());
    return _current;
}

//---------------------------------------------------------------------------

move_generator* nogo::create_move_generator(bw to_play) const
{
    return new nogo_move_generator(*this, to_play);
}

//---------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& os, const nogo_board& nboard)
{
    const char COLOR2CHAR[] = {'X', 'O', '.', '#'};
    int size = (int) nboard.board.size();
    int n_cols = nboard.shape.second;
    std::string sboard;
    for (int i = 0; i < size; i++)
    {
        sboard += COLOR2CHAR[nboard.board[i]];
        sboard += ((i+1) % n_cols == 0) ? '\n' : ' ';
    }
    os << sboard;
    return os;
}
