#include "init_random.h"
#include "random.h"

namespace mcgs_init {
void init_random()
{
    mcgs_init::init_random_impl();
}
} // namespace mcgs_init
