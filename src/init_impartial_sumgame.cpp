#include "init_impartial_sumgame.h"

#include <cstddef>
#include <string>

#include "throw_assert.h"
#include "impartial_sumgame.h"

namespace mcgs_init {
void init_impartial_sumgame(size_t idx_bits,
                            const std::string& ttable_load_file_name)
{
    THROW_ASSERT(idx_bits > 0);
    init_impartial_sumgame_ttable(idx_bits, ttable_load_file_name);
}
} // namespace mcgs_init
