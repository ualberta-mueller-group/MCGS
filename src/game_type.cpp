#include "game_type.h"

#include <typeinfo>
#include <unordered_map>
#include <typeindex>

std::unordered_map<std::type_index, game_type_t> game_type_map;
game_type_t next_id = 0;

namespace __game_type_impl { // NOLINT

game_type_t __get_game_type(const std::type_info& info) // NOLINT
{
    const std::type_index& idx = std::type_index(info);

    auto it = game_type_map.find(idx);
    game_type_t gt = 0;

    if (it == game_type_map.end())
    {
        gt = next_id++;
        game_type_map.insert({idx, gt});
    }
    else
    {
        gt = it->second;
    }

    return gt;
}

} // namespace __game_type_impl
