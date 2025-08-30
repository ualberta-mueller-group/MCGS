/*
    Sits on the runtime stack, at the start of sumgame solve functions,
    and upon destruction, undoes actions not undone since the start
    of the function call. See enum
    sumgame_undo_code definition in sumgame.h

    See development-notes.md
*/
#pragma once
#include "sumgame.h"

class sumgame::undo_stack_unwinder
{
public:
    undo_stack_unwinder(sumgame& sum);
    ~undo_stack_unwinder();

private:
    sumgame& _sum;
};
