//---------------------------------------------------------------------------
// alternating_move_game - a game where both players take turns to play
//---------------------------------------------------------------------------
#include "alternating_move_game.h"

#include "solve.h"

bool alternating_move_game::solve()
{
    return ::solve(*this);
}