#include "clobber_1xn.h"

#include "cgt_basics.h"
#include "cgt_move.h"

clobber_1xn::clobber_1xn(std::string game_as_string) :
    strip(game_as_string)
{ }

void clobber_1xn::play(const move& m)
{
    const int player = to_play();
    const int from = cgt_move::from(m);
    const int to = cgt_move::to(m);
    assert(at(from) == player);
    assert(at(to) == opponent());
    game::play(m); // changes to_play
    replace(from, EMPTY);
    replace(to, player);
}

void clobber_1xn::undo_move()
{
    const move m = move_stack().back();
    const int from = cgt_move::from(m);
    const int to = cgt_move::to(m);
    game::undo_move(); // changes to_play back
    const int player = to_play();
    assert(at(from) == EMPTY);
    assert(at(to) == player);
    replace(from, player);
    replace(to, opponent());
}


std::ostream& operator<<(std::ostream& out, const clobber_1xn& g)
{
    out << g.board_as_string();
    return out;
}
//---------------------------------------------------------------------------

class clobber_1xn_move_generator : public move_generator
{
public:
    clobber_1xn_move_generator(const clobber_1xn& game);
    void operator++();
    operator bool() const;
    move gen_move() const;
private:
    int at(int p) const {return _game.at(p); }
    int to_play() const {return _game.to_play(); }
    int opponent() const {return _game.opponent(); }
    bool is_move(int p, int dir) const;
    bool has_move(int p) const;

    void find_next_move();
    const clobber_1xn& _game;
    int _current; // current stone location to test
    int _dir; // +-1
};

inline clobber_1xn_move_generator::clobber_1xn_move_generator(const clobber_1xn& game) :
    _game(game), _current(0), _dir(1)
{
    if (! is_move(_current, _dir))
        find_next_move();
}


void clobber_1xn_move_generator::operator++()
{
    find_next_move();
}

inline bool clobber_1xn_move_generator::is_move(int p, int dir) const
{
    return at(p) == to_play()
        && _game.checked_is_color(p + dir, opponent());
}

bool clobber_1xn_move_generator::has_move(int p) const
{
    assert(at(p) == to_play());
    return is_move(p, 1) || is_move(p, -1);
}

void clobber_1xn_move_generator::find_next_move()
{
    const int num = _game.size();
    
    // try same from, other dir first.
    if (      _dir == 1
           && _current < num
           && at(_current) == to_play()
           && is_move(_current, -1)
       )
       _dir = -1;
    
    else // advance
    {
        ++_current;
        while (   _current < num
               && (  at(_current) != to_play()
                  || ! has_move(_current)
                  )
              )
            ++_current;
        if (_current < num)
        {
            if (is_move(_current, 1))
               _dir = 1;
            else
               _dir = -1;
        }
    }
}

clobber_1xn_move_generator::operator bool() const
{
    return _current < _game.size();
}

move clobber_1xn_move_generator::gen_move() const
{
    assert(operator bool());
    return cgt_move::two_part_move(_current, _current + _dir);
}
//---------------------------------------------------------------------------

move_generator* clobber_1xn::create_mg() const
{
    return new clobber_1xn_move_generator(*this);
}
//---------------------------------------------------------------------------
