#include "sumgame_undo_stack_unwinder.h"
#include "sumgame.h"
#include <cassert>

using namespace std;

sumgame::undo_stack_unwinder::undo_stack_unwinder(sumgame& sum) : _sum(sum)
{
    _sum._push_undo_code(SUMGAME_UNDO_STACK_FRAME);
}

sumgame::undo_stack_unwinder::~undo_stack_unwinder()
{
    while (!_sum._undo_code_stack.empty())
    {
        sumgame_undo_code value = _sum._undo_code_stack.back();
        // don't pop -- sumgame's methods will do this

        switch (value)
        {
            case SUMGAME_UNDO_STACK_FRAME:
            {
                // but pop this one
                _sum._pop_undo_code(SUMGAME_UNDO_STACK_FRAME);
                return;
            }

            case SUMGAME_UNDO_PLAY:
            {
                _sum.undo_move();
                continue;
            }

            case SUMGAME_UNDO_SIMPLIFY_BASIC:
            {
                _sum.undo_simplify_basic();
                continue;
            }
        }
    }

    assert(false);
}
