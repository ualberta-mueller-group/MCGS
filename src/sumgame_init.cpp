#include "sumgame_init.h"
#include "sumgame.h"
#include <cstddef>

namespace mcgs_init {
void init_ttable_sumgame(size_t ttable_index_bits)
{
    sumgame::init_ttable(ttable_index_bits);
}
} // namespace mcgs_init

