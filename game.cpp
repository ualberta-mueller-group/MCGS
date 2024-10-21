//---------------------------------------------------------------------------
// Combinatorial game and move generator
//---------------------------------------------------------------------------
#include "game.h"

#include "solve.h"

bool alternating_move_game::solve()
{
    return ::solve(*this);
}