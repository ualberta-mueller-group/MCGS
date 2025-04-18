#include "game_type.h"

static_assert(false, "This file probably shouldn't be used");

namespace {
game_type_t next_id = 1; // At least 1 (in case it's used for multiplication)
} // namespace

namespace __game_type_impl {

game_type_info_t::game_type_info_t(game_type_t type_number): type_number(type_number)
{
}

game_type_info_t new_game_type_info()
{
    game_type_info_t info(next_id);
    next_id++;

    return info;
}

} // namespace __game_type_impl
