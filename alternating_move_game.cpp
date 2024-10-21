//---------------------------------------------------------------------------
// alternating_move_game - a game where both players take turns to play
//---------------------------------------------------------------------------
#include "alternating_move_game.h"

#include <memory>

// Solve combinatorial game - find winner
// Game-independent implementation of basic boolean minimax
bool alternating_move_game::solve()
{
    game& pos = game_pos();
    std::unique_ptr<move_generator>mgp(pos.create_move_generator(to_play()));
    move_generator& mg = *mgp;
    
    for (; mg; ++mg)
    {
        play(mg.gen_move());
        bool success = false;
        bool found = find_static_winner(success);
        if (! found)
            success = not solve();
        undo_move();
        if (success)
            return true;
    }
    return false;
}
