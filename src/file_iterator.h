/*
    Recursive file iterator. Returns file paths which are not directories
    (but which may belong to subdirectories).

    Used by autotests, to ensure .test file order is well-defined.
    Interface is similar to move_generator interface.

    Defines:
        interface i_file_iterator
        class file_iterator_alphabetical
*/
#pragma once

#include <filesystem>
#include <vector>
#include <cassert>
#include <string>

#include "utilities.h"

////////////////////////////////////////////////// interface i_file_iterator
class i_file_iterator
{
public:
    virtual ~i_file_iterator() {}

    virtual operator bool() const = 0;
    virtual void operator++() = 0;
    virtual const std::filesystem::directory_entry& gen_entry() const = 0;
};

//////////////////////////////////////////////////
// class file_iterator_alphabetical

class file_iterator_alphabetical: public i_file_iterator
{
public:
    file_iterator_alphabetical(const std::string& start_dir);
    file_iterator_alphabetical(const std::filesystem::directory_entry& start_dir);

    operator bool() const override;
    void operator++() override;
    const std::filesystem::directory_entry& gen_entry() const override;

private:
    void _expand(const std::filesystem::directory_entry& dir);
    void _increment(bool init);

    static bool _compare(const std::filesystem::directory_entry& entry1,
                         const std::filesystem::directory_entry& entry2);

    std::vector<std::filesystem::directory_entry> _file_entries;
    std::vector<std::filesystem::directory_entry> _dir_entries;
};

//////////////////////////////////////////////////
// file_iterator_alphabetical methods

inline file_iterator_alphabetical::file_iterator_alphabetical(const std::string& start_dir)
{
    _expand(std::filesystem::directory_entry(start_dir));
    _increment(true);
}

inline file_iterator_alphabetical::file_iterator_alphabetical(const std::filesystem::directory_entry& start_dir)
{
    _expand(start_dir);
    _increment(true);
}

inline file_iterator_alphabetical::operator bool() const
{
    // No files --implies--> No directories
    assert(LOGICAL_IMPLIES(_file_entries.empty(), _dir_entries.empty()));
    return !_file_entries.empty();
}

inline void file_iterator_alphabetical::operator++()
{
    assert(*this);
    _increment(false);
}

inline const std::filesystem::directory_entry& file_iterator_alphabetical::gen_entry() const
{
    assert(*this);
    assert(!_file_entries.empty());

    const std::filesystem::directory_entry& entry = _file_entries.back();
    assert(!entry.is_directory());

    return entry;
}

inline bool file_iterator_alphabetical::_compare(
    const std::filesystem::directory_entry& entry1,
    const std::filesystem::directory_entry& entry2)
{
    return entry1.path().string() > entry2.path().string();
}

