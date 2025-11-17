/*
    Defines type create_game_gen_fn_t

    A function type for creating db_game_generators for a game. These are
    called during database generation.
*/
#pragma once

#include <functional>
#include <type_traits>

#include "config_map.h"
#include "db_game_generator.h"
#include "grid_generator.h"
#include "gridlike_db_game_generator.h"
#include "grid.h"
#include "strip.h"

typedef std::function<i_db_game_generator*(const config_map&)>
    create_game_gen_fn_t;

//////////////////////////////////////////////////
/*
    These functions return a function when called, and can be used for
    most grid/strip games.

    Note the '=' in the lambda capture clause to capture by value instead of
    by reference.
*/

template <class Gridlike_Game_T, gridlike_type_enum gridlike_type>
create_game_gen_fn_t get_gridlike_create_game_gen_fn(
    const std::vector<int>& tile_sequence)
{
    static_assert(!std::is_abstract_v<Gridlike_Game_T>);

    create_game_gen_fn_t fn =
        [=](const config_map& config) -> i_db_game_generator*
    {
        constexpr bool STRIPS_ONLY = gridlike_type == GRIDLIKE_TYPE_STRIP;
        const std::optional<int_pair> max_dims = config.get_dims("max_dims");

        THROW_ASSERT(max_dims.has_value() && //
                     max_dims->first >= 0 && //
                     max_dims->second >= 0   //
        );

        grid_generator* gg =
            new grid_generator(max_dims.value(), tile_sequence, STRIPS_ONLY);

        return new gridlike_db_game_generator<Gridlike_Game_T, gridlike_type>(
            gg);
    };

    return fn;
}

template <class Gridlike_Game_T, gridlike_type_enum gridlike_type>
create_game_gen_fn_t get_gridlike_create_game_gen_fn(
    const std::vector<int>& tile_sequence, bool mask_active_bit,
    int mask_inactive_tile)
{
    static_assert(!std::is_abstract_v<Gridlike_Game_T>);

    create_game_gen_fn_t fn =
        [=](const config_map& config) -> i_db_game_generator*
    {
        constexpr bool STRIPS_ONLY = gridlike_type == GRIDLIKE_TYPE_STRIP;
        const std::optional<int_pair> max_dims = config.get_dims("max_dims");

        THROW_ASSERT(max_dims.has_value() && //
                     max_dims->first >= 0 && //
                     max_dims->second >= 0   //
        );

        grid_generator* gg =
            new grid_generator(max_dims.value(), tile_sequence, mask_active_bit,
                               mask_inactive_tile, STRIPS_ONLY);

        return new gridlike_db_game_generator<Gridlike_Game_T, gridlike_type>(
            gg);
    };

    return fn;
}
