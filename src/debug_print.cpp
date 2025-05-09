#include "debug_print.h"
#include <cassert>
#include <cstdlib>
#include "throw_assert.h"
#include <string>

namespace {
// NOLINTNEXTLINE(readability-identifier-naming)
std::ofstream _debug_stream;

void exit_handler()
{
    if (!_debug_stream.is_open())
        return;

    _debug_stream.flush();
    _debug_stream.close();
}

} // namespace

std::ofstream& debug_stream()
{
    assert(_debug_stream.is_open());
    return _debug_stream;
}

namespace mcgs_init {
void init_debug_print(const std::string& filename)
{
    THROW_ASSERT(!_debug_stream.is_open());

    if (filename.empty())
        return;

    _debug_stream.open(filename);
    THROW_ASSERT(std::atexit(exit_handler) == 0);

    THROW_ASSERT(_debug_stream.is_open());
}
} // namespace mcgs_init 


