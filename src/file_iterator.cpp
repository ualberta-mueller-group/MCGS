#include "file_iterator.h"

#include <filesystem>
#include <vector>
#include <cassert>
#include <algorithm>

using namespace std;

//////////////////////////////////////////////////
// file_iterator_alphabetical methods
void file_iterator_alphabetical::_expand(const filesystem::directory_entry& dir)
{
    assert(dir.is_directory());
    assert(_file_entries.empty());

    vector<filesystem::directory_entry> new_dirs;

    filesystem::directory_iterator it(dir);

    for (const filesystem::directory_entry& entry : it)
    {
        if (entry.is_directory())
            new_dirs.push_back(entry);
        else
            _file_entries.push_back(entry);
    }

    sort(_file_entries.begin(), _file_entries.end(), _compare);
    sort(new_dirs.begin(), new_dirs.end(), _compare);

    for (const filesystem::directory_entry& entry : new_dirs)
        _dir_entries.push_back(entry);
}

void file_iterator_alphabetical::_increment(bool init)
{
    assert(init || !_file_entries.empty());

    if (!init)
        _file_entries.pop_back();

    if (!_file_entries.empty())
        return;

    // Keep expanding directories
    while (!_dir_entries.empty() && _file_entries.empty())
    {
        filesystem::directory_entry dir = _dir_entries.back();
        _dir_entries.pop_back();

        _expand(dir);
    }
}

