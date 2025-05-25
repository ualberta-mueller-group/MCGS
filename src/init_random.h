#pragma once
/*
    Initialize global random generator based on random_seed variable in
    globals.h. If seed is 0, seeds with time since epoch, and overwrites the
    seed in globals.h.

    IMPORTANT: No other random generators should use the current time
    as their seed.
*/

namespace mcgs_init {
void init_random();
} // namespace mcgs_init
