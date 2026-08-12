#include "init_sumgame.h"

#include <cstddef>
#include <string>

#include "sumgame.h"

namespace mcgs_init {
void init_sumgame(size_t ttable_index_bits,
                  const std::string& ttable_load_file_name)
{
    sumgame::init_sumgame(ttable_index_bits, ttable_load_file_name);
}
} // namespace mcgs_init
