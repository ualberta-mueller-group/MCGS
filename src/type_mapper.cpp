#include "type_mapper.h"
#include "utilities.h"

void type_mapper::register_type(const std::string& type_name, game_type_t runtime_type)
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

game_type_t& type_mapper::_type_remapping_ref(game_type_t runtime_type)
{
    if (!(runtime_type < _type_remappings.size()))
    {
        const size_t new_size = new_vector_capacity(runtime_type, _type_remappings.size());
        _type_remappings.resize(new_size, 0);
    }

    assert(runtime_type < _type_remappings.size());
    return _type_remappings[runtime_type];
}
