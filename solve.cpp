#include "solve.h"

#include "game.h"

// solve by negamax boolean search
bool solve(alternating_move_game& g)
{
    game& pos = g.game_pos();
    std::unique_ptr<move_generator>mgp(pos.create_move_generator(g.to_play()));
    move_generator& mg = *mgp;
    
    for (; mg; ++mg)
    {
        g.play(mg.gen_move());
        bool success = false;
        bool found = g.find_static_winner(success);
        if (! found)
            success = not solve(g);
        g.undo_move();
        if (success)
            return true;
    }
    return false;
}
