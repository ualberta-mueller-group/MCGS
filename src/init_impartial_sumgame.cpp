#include "init_impartial_sumgame.h"
#include <cstddef>
#include "throw_assert.h"
#include "impartial_sumgame.h"

namespace mcgs_init {
void init_impartial_sumgame(size_t idx_bits)
{
    THROW_ASSERT(idx_bits > 0);
    init_impartial_sumgame_ttable(idx_bits);
}
} // namespace mcgs_init
