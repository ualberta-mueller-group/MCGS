#include "nogo_1xn.h"

#include "strip.h"

nogo_1xn::nogo_1xn(std::string game_as_string) :
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
    const move mc = move_stack().back();
    const int to = cgt_move::decode(mc);
    game::undo_move();
    const bw player = cgt_move::get_color(mc);
    assert(at(to) == player);
    replace(to, EMPTY);
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
    
    std::vector<int> _board = _game.get_board();
    int num = (int)_board.size();

    _board[p] = to_play();

    bool has_liberty = _board[0] == EMPTY;
    for (int i = 1; i < num; i++) {
        if (_board[i] == EMPTY) {
            has_liberty = true;
        }
        else if (_board[i] != _board[i-1] && _board[i-1] != EMPTY) {
            if (has_liberty) {
                has_liberty = false;
            }
            else {
                return false;
            }
        }
    }
    if (!has_liberty)   // last block is not empty and has no liberty
        return false;

    _board[p] = EMPTY;

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