#include "game.h"

// solve by negamax boolean search
bool solve(game& g)
{
    std::unique_ptr<move_generator>mgp(g.create_move_generator());
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
            {return true;}
    }
    return false;
}
