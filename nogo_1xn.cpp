//---------------------------------------------------------------------------
// Implementation of NoGo on a 1-dimensional 1xn board
//---------------------------------------------------------------------------
#include "nogo_1xn.h"

#include "cgt_basics.h"
#include "game.h"
#include "strip.h"
#include <memory>

using std::string, std::pair, std::unique_ptr;

nogo_1xn::nogo_1xn(const vector<int>& board) :
    strip(board)
{ }

nogo_1xn::nogo_1xn(string game_as_string) :
    strip(game_as_string)
{ }

void nogo_1xn::play(const move& m, bw to_play)
{
    const int to = m;
    assert(at(to) == EMPTY);
    game::play(m, to_play);
    replace(to, to_play);
}

void nogo_1xn::undo_move()
{
    const move mc = last_move();
    const int to = cgt_move::decode(mc);
    game::undo_move();
    const bw player = cgt_move::get_color(mc);
    assert(at(to) == player);
    replace(to, EMPTY);
}

/*
   implements "xo split" from
   Henry's paper
   

TODO: block simplification

*/
split_result nogo_1xn::split() const
{
    // NOTE: don't use checked_is_color here -- it accesses the board before block simplification


    string board = board_as_string();
    int N = board.size();
    // should do block simplification right here

    vector<pair<int, int>> subgame_ranges;

    int subgame_start = 0;

    for (int i = 0; i < N; i++)
    {
        const int color = clobber_char_to_color(board[i]);

        const bool prev_in_range = i - 1 >= 0;
        const int color_prev = prev_in_range ? clobber_char_to_color(board[i - 1]) : EMPTY;

        if ( color != EMPTY
            && prev_in_range 
            && color_prev == opponent(color)
            )
        {
            // found an XO or OX split
            subgame_ranges.push_back({subgame_start, (i - 1) - subgame_start + 1});
            subgame_start = i;
        }
    }

    // remainder at end
    if (N > 0)
    {
        subgame_ranges.push_back({subgame_start, (N - 1) - subgame_start + 1});
    }

    if (subgame_ranges.size() == 1)
    {
        return split_result(); // no split
    } else
    {
        split_result result = split_result(vector<game*>());

        for (const pair<int, int>& range : subgame_ranges)
        {
            game* g = new nogo_1xn(board.substr(range.first, range.second));
            result->push_back(g);
        }

        return result;
    }

}

game* nogo_1xn::inverse() const
{
    return new nogo_1xn(inverse_board());
}

std::ostream& operator<<(std::ostream& out, const nogo_1xn& g)
{
    out << g.board_as_string();
    return out;
}
//---------------------------------------------------------------------------

class nogo_1xn_move_generator : public move_generator
{
public:
    nogo_1xn_move_generator(const nogo_1xn& game, bw to_play);
    void operator++();
    operator bool() const;
    move gen_move() const;
private:
    int at(int p) const {return _game.at(p); }
    bool is_legal(int p) const;

    void find_next_move();
    const nogo_1xn& _game;
    int _current; // current stone location to test
};

inline nogo_1xn_move_generator::nogo_1xn_move_generator(const nogo_1xn& game, bw to_play) :
    move_generator(to_play),
    _game(game),
    _current(0)
{
    if (_game.size() > 0 && !is_legal(_current))
        find_next_move();
}

void nogo_1xn_move_generator::operator++()
{
    find_next_move();
}

inline void nogo_1xn_move_generator::find_next_move()
{
    _current++;

    int num = (int)_game.size();
    while (_current < num && !is_legal(_current)) {
        _current++;
    }
}

inline bool nogo_1xn_move_generator::is_legal(int p) const
{
    if (at(p) != EMPTY)
        return false;

    int num = _game.size();

    int current, previous = (p == 0) ? to_play() : _game.at(0);
    bool has_liberty = previous == EMPTY;
    for (int i = 1; i < num; i++) {
        current = (p == i) ? to_play() : _game.at(i);

        if (current == EMPTY) {
            has_liberty = true;
        }
        else if (current != previous && previous != EMPTY) {
            if (has_liberty) {
                has_liberty = false;
            }
            else {
                return false;
            }
        }

        previous = current;
    }
    if (!has_liberty)   // last block is not empty and has no liberty
        return false;

    return true;
}

nogo_1xn_move_generator::operator bool() const
{
    return _current < _game.size();
}

move nogo_1xn_move_generator::gen_move() const
{
    assert(operator bool());
    return _current;
}
//---------------------------------------------------------------------------

move_generator* nogo_1xn::create_move_generator(bw to_play) const
{
    return new nogo_1xn_move_generator(*this, to_play);
}
//---------------------------------------------------------------------------
