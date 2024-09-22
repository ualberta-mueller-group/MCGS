#include "game.h"

// solve by negamax boolean search
bool solve(game& g)
{
    // todo use std::unique_ptr, std::make_unique
    move_generator* mgp = g.create_mg();
    move_generator& mg = *mgp;
    
    // to_play
    for (; mg; ++mg)
    {
        g.play(mg.gen_move());
        bool success = false;
        bool found = g.find_static_winner(success);
        if (! found)
            success = not solve(g);
        g.undo_move();
        if (success)
            {delete mgp; return true;}
    }
    delete mgp; return false;
}
