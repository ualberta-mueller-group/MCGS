#include "init_random.h"
#include "global_options.h"
#include "random.h"
#include "utilities.h"

namespace mcgs_init {
void init_random()
{
    while (global::random_seed() == 0)
        global::random_seed.set(ms_since_epoch());

    set_random_seed(global::random_seed());
}
} // namespace mcgs_init
