/*
    Defines type_mapper class, used by database implementation.

    Acts as a translation layer from runtime-allocated game_type_t values, to
    those from the disk.

    NOTE: Translation is trivial right now, and only acts as a check to ensure
    that types match between the file and MCGS executable
*/
#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include "serializer.h"
#include "type_table.h"
#include "iobuffer.h"

////////////////////////////////////////////////// class type_mapper
class type_mapper
{
public:
    type_mapper();

    void clear();

    void register_type(const std::string& type_name, game_type_t runtime_type);
    game_type_t translate_type(game_type_t runtime_type) const;

    std::unordered_map<game_type_t, std::string> get_disk_type_to_name_map()
        const;

private:
    std::unordered_map<std::string, game_type_t> _name_to_id_map;
    std::unordered_map<game_type_t, std::string> _id_to_name_map;
    std::vector<uint8_t> _valid_ids; // bools as u8

    bool _type_is_valid(game_type_t runtime_type) const;
    void _mark_type_valid(game_type_t runtime_type);

    friend serializer<type_mapper>;
    friend std::ostream& operator<<(std::ostream& os,
                                    const type_mapper& mapper);
};

//////////////////////////////////////// serializer<type_mapper>
template <>
struct serializer<type_mapper>
{
    inline static void save(obuffer& os, const type_mapper& mapper)
    {
        serializer<decltype(mapper._name_to_id_map)>::save(
            os, mapper._name_to_id_map);

        serializer<decltype(mapper._id_to_name_map)>::save(
            os, mapper._id_to_name_map);
    }

    inline static type_mapper load(ibuffer& is)
    {
        type_mapper mapper;

        mapper._name_to_id_map =
            serializer<decltype(mapper._name_to_id_map)>::load(is);

        mapper._id_to_name_map =
            serializer<decltype(mapper._id_to_name_map)>::load(is);

        return mapper;
    }
};

////////////////////////////////////////////////// type_mapper methods
inline type_mapper::type_mapper()
{
}

inline void type_mapper::clear()
{
    _id_to_name_map.clear();
    _name_to_id_map.clear();
    _valid_ids.clear();
}

inline game_type_t type_mapper::translate_type(game_type_t runtime_type) const
{
    if (!_type_is_valid(runtime_type))
        return 0;
    return runtime_type;
}

inline bool type_mapper::_type_is_valid(game_type_t runtime_type) const
{
    return (runtime_type < _valid_ids.size() && _valid_ids[runtime_type] == 1);
}

inline void type_mapper::_mark_type_valid(game_type_t runtime_type)
{
    if (!(runtime_type < _valid_ids.size()))
        _valid_ids.resize(runtime_type + 1, 0);

    assert(runtime_type < _valid_ids.size() && _valid_ids[runtime_type] == 0);
    _valid_ids[runtime_type] = 1;
}
