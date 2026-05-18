#include "test_file_iterator.h"
#include "file_iterator.h"

#include <cassert>
#include <filesystem>

////////////////////////////////////////////////// test_file_iterator methods
void test_file_iterator::_increment(bool init)
{
    assert(init || *this);

    if (!init)
    {
        assert(_file_iter);
        ++_file_iter;
    }

    while (_file_iter && !_is_test_file(_file_iter.gen_entry()))
        ++_file_iter;
}

bool test_file_iterator::_is_test_file(
    const std::filesystem::directory_entry& entry)
{
    if (!entry.is_regular_file())
        return false;

    const std::filesystem::path& path = entry.path();
    return path.extension() == ".test";
}
