//---------------------------------------------------------------------------
// Move utilities for impartial game wrapper
// This (mis-)uses the SIGN_BIT in cgt_move
// To encode the color of the move in the "wrapped" game.
// This color is different from the "regular" color
// used in minimax play.
//---------------------------------------------------------------------------

#pragma once

#include "cgt_move.h"
#include "cgt_basics.h"
#include <cassert>

namespace impartial_wrapper_move {

inline bw get_color(move m)
{
    return cgt_move::get_bit(m, cgt_move::SIGN_BIT);
}

inline move encode_wrapped_move(move m, bw color)
{
    assert_black_white(color);
    assert(get_color(m) == 0);
    if (color == 1)
    {
        m = cgt_move::set_bit(m, cgt_move::SIGN_BIT);
    }
    return m;
}

inline move decode_wrapped(move m) // strip color bit
{
    return m & (~cgt_move::SIGN_MASK);
}

} // namespace impartial_wrapper_move
