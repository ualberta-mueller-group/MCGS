#include "type_table.h"

#include <typeinfo>
#include <typeindex>
#include <cassert>
#include <unordered_map>
#include <memory>

bool type_table_t::_initialized = false;

void type_table_t::set_initialized()
{
    assert(!_initialized);
    _initialized = true;
}

namespace {
std::unordered_map<std::type_index, std::shared_ptr<type_table_t>>
    type_table_map;

} // namespace

namespace __type_table_impl {

type_table_t* get_type_table(const std::type_info& tinfo)
{
    const std::type_index tidx(tinfo);

    auto it = type_table_map.find(tidx);

    if (it == type_table_map.end())
    {
        type_table_t* table = new type_table_t();
        type_table_map.emplace(tidx, table);
        return table;
    }

    return it->second.get();
}

} // namespace __type_table_impl
