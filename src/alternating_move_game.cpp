//---------------------------------------------------------------------------
// alternating_move_game - a game where both players take turns to play
//---------------------------------------------------------------------------
#include "alternating_move_game.h"
#include "game.h"

#include <memory>
#include <cassert>

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

#ifdef ASSERT_RESTORE_DEBUG
assert_restore_alternating_game::assert_restore_alternating_game(
    const alternating_move_game& game)
    : _game(game),
    _arg(!game.has_game_pos()
        ? nullptr : new assert_restore_game(game.game_pos())),
    _to_play(game.to_play()),
    _game_hash(game.game_hash())
{
}

assert_restore_alternating_game::~assert_restore_alternating_game()
{
    if (_arg != nullptr)
        delete _arg;

    assert(_to_play == _game.to_play());
    assert(_game_hash == _game.game_hash());
}
#endif
