#include "init_hashing.h"
#include "hashing.h"
#include "random.h"

namespace mcgs_init {

void init_hashing()
{
    // Multiply by a prime number to avoid collisions with random generators
    init_global_random_tables(get_random_u64() * 4421771);
}

} // namespace mcgs_init
