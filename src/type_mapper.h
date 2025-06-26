#pragma once
#include <string>
#include "serializer.h"
#include "throw_assert.h"
#include "type_table.h"

////////////////////////////////////////////////// class type_mapper
class type_mapper
{
public:
    type_mapper();

    void clear();

    void register_type(const std::string& type_name, game_type_t runtime_type);
    game_type_t translate_type(game_type_t runtime_type) const;

private:
    game_type_t& _type_remapping_ref(game_type_t runtime_type);

    game_type_t _next_disk_type;

    std::unordered_map<std::string, game_type_t> _disk_types;
    std::vector<game_type_t> _type_remappings;

    friend serializer<type_mapper>;
    friend std::ostream& operator<<(std::ostream& os, const type_mapper& mapper);
};

//////////////////////////////////////// serializer<type_mapper>
template <>
struct serializer<type_mapper>
{
    inline static void save(obuffer& os, const type_mapper& mapper)
    {
        serializer<decltype(mapper._next_disk_type)>::save(os, mapper._next_disk_type);
        serializer<decltype(mapper._disk_types)>::save(os, mapper._disk_types);
    }

    inline static type_mapper load(ibuffer& is)
    {
        type_mapper mapper;

        mapper._next_disk_type = serializer<decltype(mapper._next_disk_type)>::load(is);
        mapper._disk_types = serializer<decltype(mapper._disk_types)>::load(is);

        return mapper;
    }
};

////////////////////////////////////////////////// type_mapper methods
inline type_mapper::type_mapper(): _next_disk_type(1)
{
}

inline void type_mapper::clear()
{
    _next_disk_type = 1;
    _disk_types.clear();
    _type_remappings.clear();
}

inline game_type_t type_mapper::translate_type(game_type_t runtime_type) const
{
    // If fails, the game's type wasn't registered
    //THROW_ASSERT(runtime_type < _type_remappings.size() && _type_remappings[runtime_type] > 0);

    // TODO should print warnings...

    if (!(runtime_type < _type_remappings.size()))
        return 0;

    return _type_remappings[runtime_type];
}
