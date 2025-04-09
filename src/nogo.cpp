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

//---------------------------------------------------------------------------

class nogo_move_generator : public move_generator
{
public:
    nogo_move_generator(const nogo& game, bw to_play);
    void operator++() override;
    operator bool() const override;
    move gen_move() const override;

private:
    bool _is_legal(int p);
    void _find_next_move();

    bool has_liberty(int p) const;
    std::vector<int> neighbors(int p) const;

    const nogo& _game;
    std::vector<int> _board;
    int _current; // current stone location to test
};

inline nogo_move_generator::nogo_move_generator(const nogo& game,
    bw to_play)
: move_generator(to_play), _game(game), _board(game.get_board()), _current(0)
{
    if (_game.size() > 0 && !_is_legal(_current))
        _find_next_move();
}

void nogo_move_generator::operator++()
{
    _find_next_move();
}

inline void nogo_move_generator::_find_next_move()
{
    _current++;

    int num = (int) _game.size();
    while (_current < num && !_is_legal(_current))
    {
        _current++;
    }
}

inline bool nogo_move_generator::_is_legal(int p)
{
    if (_game.at(p) != EMPTY)
        return false;

    _board[p] = to_play();
    int opp_color = ebw_opponent(to_play());

    if (!has_liberty(p))
    {
        _board[p] = EMPTY;
        return false;
    }

    std::vector<int> nbrs = neighbors(p);
    for (int nbr : nbrs)
    {
        if (_board[nbr] == opp_color && !has_liberty(nbr))
        {
            _board[p] = EMPTY;
            return false;
        }
    }

    _board[p] = EMPTY;
    return true;
}

bool nogo_move_generator::has_liberty(int p) const
{
    int size = _game.size();
    std::vector<bool> markers = std::vector<bool>(size, false);

    int color = _board[p];
    std::vector<int> point_stack = { p };
    markers[p] = true;

    while (!point_stack.empty())
    {
        int p = point_stack.back();
        point_stack.pop_back();

        std::vector<int> nbrs = neighbors(p);
        for (int nbr : nbrs)
        {
            if (_board[nbr] == EMPTY)
                return true;
            if (_board[nbr] == color && !markers[nbr])
            {
                point_stack.push_back(nbr);
                markers[nbr] = true;
            }
        }
    }

    return false;
}

std::vector<int> nogo_move_generator::neighbors(int p) const
{
    int_pair coord = _game.point_to_coord(p);
    int r = coord.first, c = coord.second;

    int_pair shape = _game.shape();
    int n_rows = shape.first, n_cols = shape.second;

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
