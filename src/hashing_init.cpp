#include "hashing_init.h"
#include "hashing.h"

namespace mcgs_init {

void init_hashing(uint64_t seed)
{
    init_global_random_tables(seed);
}

} // namespace mcgs_init
