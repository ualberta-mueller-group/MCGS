//---------------------------------------------------------------------------
// alternating_move_game - a game where both players take turns to play
//---------------------------------------------------------------------------
#include "alternating_move_game.h"

#include <memory>

// Solve combinatorial game - find winner
// Game-independent implementation of basic boolean minimax
bool alternating_move_game::solve() const
{
    assert_restore_alternating_game ar(*this);
    alternating_move_game& ag = const_cast<alternating_move_game&>(*this);
    return ag._solve();
}

bool alternating_move_game::_solve()
{
    game& pos = game_pos();
    std::unique_ptr<move_generator> mgp(pos.create_move_generator(to_play()));
    move_generator& mg = *mgp;

    for (; mg; ++mg)
    {
        play(mg.gen_move());
        bool success = false;
        bool found = find_static_winner(success);
        if (!found)
            success = not solve();
        undo_move();
        if (success)
            return true;
    }
    return false;
}

//---------------------------------------------------------------------------
assert_restore_alternating_game::assert_restore_alternating_game(
    const alternating_move_game& game)
    : _game(game), _game_hash(game.game_hash())
{
}

assert_restore_alternating_game::~assert_restore_alternating_game()
{
    assert_equal(_game_hash, _game.game_hash());
}
