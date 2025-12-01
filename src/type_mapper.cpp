#include "type_mapper.h"

#include <string>
#include <cassert>
#include <unordered_map>
#include <utility>
#include <cstddef>
#include <iostream>

#include "utilities.h"
#include "type_table.h"
#include "throw_assert.h"

using namespace std;

void type_mapper::register_type(const string& type_name,
                                game_type_t runtime_type)
{
    // Search for existing disk type
    auto it = _disk_types.find(type_name);

    game_type_t disk_type = 0; // 0 is invalid type

    if (it == _disk_types.end())
    {
        // Disk type doesn't exist; create it
        disk_type = _next_disk_type;
        _next_disk_type++;
        _disk_types[type_name] = disk_type;
    }
    else
    {
        // Use existing disk type
        disk_type = it->second;
    }

    // runtime_type's mapped disk type (should be 0)
    game_type_t& mapped_type = _type_remapping_ref(runtime_type);

    // runtime_type not already registered, AND disk_type is valid
    THROW_ASSERT(mapped_type == 0 && disk_type > 0);

    mapped_type = disk_type;

    assert(translate_type(runtime_type) == disk_type);
}

unordered_map<game_type_t, string> type_mapper::get_disk_type_to_name_map()
    const
{
    unordered_map<game_type_t, string> disk_type_to_name_map;

    for (const pair<const string, game_type_t>& p : _disk_types)
    {
        auto inserted = disk_type_to_name_map.emplace(p.second, p.first);
        assert(inserted.second);
    }

    return disk_type_to_name_map;
}

game_type_t& type_mapper::_type_remapping_ref(game_type_t runtime_type)
{
    if (!(runtime_type < _type_remappings.size()))
    {
        const size_t new_size =
            new_vector_capacity(runtime_type, _type_remappings.size());
        _type_remappings.resize(new_size, 0);
    }

    assert(runtime_type < _type_remappings.size());
    return _type_remappings[runtime_type];
}

//////////////////////////////////////////////////
ostream& operator<<(ostream& os, const type_mapper& mapper)
{

    os << "Types in mapper: \n";
    for (const auto& it : mapper._disk_types)
    {
        os << it.first << " -> " << it.second << '\n';
    }

    return os;
}
