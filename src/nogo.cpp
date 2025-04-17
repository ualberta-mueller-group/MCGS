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
    const int to = m;
    assert(at(to) == EMPTY);
    game::play(m, to_play);
    replace(to, to_play);
}

void nogo::undo_move()
{
    const move mc = last_move();
    const int to = cgt_move::decode(mc);
    game::undo_move();
    const bw player = cgt_move::get_color(mc);
    assert(at(to) == player);
    replace(to, EMPTY);
}

split_result nogo::_split_implementation() const
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
bool nogo_rule::is_legal(std::vector<int> board, int_pair shape, int p, int toplay)
{
    if (board[p] != EMPTY)
        return false;

    board[p] = toplay;
    int opp_color = ebw_opponent(toplay);

    if (!nogo_rule::has_liberty(board, shape, p))
    {
        board[p] = EMPTY;
        return false;
    }

    std::vector<int> nbrs = nogo_rule::neighbors(shape, p);
    for (int nbr : nbrs)
    {
        if (board[nbr] == opp_color && !nogo_rule::has_liberty(board, shape, nbr))
        {
            board[p] = EMPTY;
            return false;
        }
    }

    board[p] = EMPTY;
    return true;
}

bool nogo_rule::has_liberty(const std::vector<int>& board, int_pair shape, int p)
{
    int size = (int) board.size();
    std::vector<bool> markers = std::vector<bool>(size, false);

    int color = board[p];
    std::vector<int> point_stack = { p };
    markers[p] = true;

    while (!point_stack.empty())
    {
        int p = point_stack.back();
        point_stack.pop_back();

        std::vector<int> nbrs = nogo_rule::neighbors(shape, p);
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

std::vector<int> nogo_rule::neighbors(int_pair shape, int p)
{
    int n_rows = shape.first;
    int n_cols = shape.second;

    int r = p / shape.second;
    int c = p % shape.second;

    std::vector<int> nbrs;
    if (r > 0)
        nbrs.push_back(p - n_cols); // up
    if (r < n_rows-1)
        nbrs.push_back(p + n_cols); // down
    if (c > 0)
        nbrs.push_back(p - 1);      // left
    if (c < n_cols-1)
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

    const nogo& _game;
    int _current; // current stone location to test
};

inline nogo_move_generator::nogo_move_generator(const nogo& game,
                                                bw to_play)
    : move_generator(to_play), _game(game), _current(0)
{
    if (_game.size() > 0 &&
        !nogo_rule::is_legal(_game.get_board(), _game.shape(), _current, to_play))
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
    while (_current < num &&
        !nogo_rule::is_legal(_game.get_board(), _game.shape(), _current, to_play()))
    {
        _current++;
    }
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
