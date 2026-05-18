#pragma once

#include <cassert>
#include <filesystem>
#include <string>

#include "file_iterator.h"
#include "throw_assert.h"

////////////////////////////////////////////////// class test_file_iterator
class test_file_iterator : public i_file_iterator
{
public:
    test_file_iterator(const std::string& start_dir);
    test_file_iterator(const std::filesystem::directory_entry& start_dir);

    operator bool() const override;
    void operator++() override;
    const std::filesystem::directory_entry& gen_entry() const override;

private:
    void _increment(bool init);

    static bool _is_test_file(const std::filesystem::directory_entry& entry);

    file_iterator_alphabetical _file_iter;
};

////////////////////////////////////////////////// test_file_iterator methods
inline test_file_iterator::test_file_iterator(const std::string& start_dir)
    : _file_iter(start_dir)
{
    THROW_ASSERT(std::filesystem::is_directory(start_dir));
    _increment(true);
}

inline test_file_iterator::test_file_iterator(
    const std::filesystem::directory_entry& start_dir)
    : _file_iter(start_dir)
{
    THROW_ASSERT(std::filesystem::is_directory(start_dir));
    _increment(true);
}

inline test_file_iterator::operator bool() const
{
    return _file_iter;
}

inline void test_file_iterator::operator++()
{
    assert(*this);
    _increment(false);
}

inline const std::filesystem::directory_entry& test_file_iterator::gen_entry()
    const
{
    assert(*this);
    const std::filesystem::directory_entry& entry = _file_iter.gen_entry();

    assert(_is_test_file(entry));
    return entry;
}


