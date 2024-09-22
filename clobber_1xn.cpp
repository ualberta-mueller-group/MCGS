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

move_generator* clobber_1xn::create_mg() const
{
    return nullptr;
}

std::ostream& operator<<(std::ostream& out, const clobber_1xn& g)
{
    out << g.board_as_string();
    return out;
}
