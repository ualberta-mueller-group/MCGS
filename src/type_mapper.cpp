#include "type_mapper.h"

#include <string>
#include <cassert>
#include <unordered_map>
#include <utility>
#include <iostream>

#include "type_table.h"
#include "throw_assert.h"

using namespace std;

void type_mapper::register_type(const string& type_name,
                                game_type_t runtime_type)
{
    assert(runtime_type > 0);

    auto name_search = _name_to_id_map.find(type_name);

    // type name has a disk type
    if (name_search != _name_to_id_map.end())
    {
        const game_type_t disk_type = name_search->second;

        if (runtime_type == disk_type)
        {
            THROW_ASSERT(!_type_is_valid(runtime_type)); // no double register
            _mark_type_valid(runtime_type);
            THROW_ASSERT(_type_is_valid(runtime_type));
        }
        else
        {
            cout << "WARNING: Database game_type_t mismatch for game \""
                 << type_name
                 << "\" (the loaded file has this game name, but assigns it a "
                    "different game_type_t). DB lookups will not work for this "
                    "game. To fix this, create a new DB file."
                 << endl;

            THROW_ASSERT(!_type_is_valid(runtime_type));
        }
    }
    else // type name has no disk type
    {
        auto type_search = _id_to_name_map.find(runtime_type);

        if (type_search == _id_to_name_map.end())
        {
            // we can safely allow this translation
            _name_to_id_map[type_name] = runtime_type;
            _id_to_name_map[runtime_type] = type_name;

            THROW_ASSERT(!_type_is_valid(runtime_type));
            _mark_type_valid(runtime_type);
            THROW_ASSERT(_type_is_valid(runtime_type));
        }
        else
        {
            cout
                << "WARNING: Database game_type_t mismatch for game \""
                << type_name
                << "\" (the loaded file does not have this game name, but uses "
                   "the game_type_t for it). DB lookups will not work for this "
                   "game. To fix this, create a new DB file."
                << endl;

            THROW_ASSERT(!_type_is_valid(runtime_type));
        }
    }
}

unordered_map<game_type_t, string> type_mapper::get_disk_type_to_name_map()
    const
{
    return _id_to_name_map;
}


//////////////////////////////////////////////////
ostream& operator<<(ostream& os, const type_mapper& mapper)
{

    os << "Types in mapper: \n";
    for (const auto& it : mapper._id_to_name_map)
    {
        os << it.first << " -> " << it.second << '\n';
    }

    return os;
}
