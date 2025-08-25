#include "game_compare_utils.h"

#include "game.h"
#include "type_table.h"
#include <sstream>
#include <cstdint>
#include <string>
#include <functional>

namespace compare_games_by_print {

__game_info::__game_info(const game& g)
{
    type = g.game_type();

    std::stringstream stream;
    g.print(stream);
    printed = stream.str();
}

bool __game_info::operator==(const __game_info& rhs) const
{
    return (type == rhs.type) && (printed == rhs.printed);
}

uint64_t __game_info_hash::operator()(const __game_info& info) const
{
    static std::hash<game_type_t> hasher1;
    static std::hash<std::string> hasher2;

    return hasher1(info.type) ^ hasher2(info.printed);
}
} // namespace compare_games_by_print
