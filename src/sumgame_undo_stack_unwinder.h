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
