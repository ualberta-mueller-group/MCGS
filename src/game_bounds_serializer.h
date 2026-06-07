#pragma once

#include "serializer.h"
#include "bounds.h"

template <>
struct serializer<game_bounds>
{
    static_assert(std::is_same_v<bound_t, int32_t>);

    inline static void save(obuffer& os, const game_bounds& bounds, serializer_ctx* ctx)
    {
        os.write_enum<bound_scale>(bounds._scale);

        os.write_i32(bounds._lower);
        os.write_bool(bounds._lower_valid);
        os.write_enum<relation>(bounds._lower_relation);

        os.write_i32(bounds._upper);
        os.write_bool(bounds._upper_valid);
        os.write_enum<relation>(bounds._upper_relation);
    }

    inline static game_bounds load(ibuffer& is, serializer_ctx* ctx)
    {
        const bound_scale scale = is.read_enum<bound_scale>();

        game_bounds bounds(scale);

        bounds._lower = is.read_i32();
        bounds._lower_valid = is.read_bool();
        bounds._lower_relation = is.read_enum<relation>();

        bounds._upper = is.read_i32();
        bounds._upper_valid = is.read_bool();
        bounds._upper_relation = is.read_enum<relation>();

        return bounds;
    }

    inline static game_bounds* load_ptr(ibuffer& is, serializer_ctx* ctx)
    {
        const bound_scale scale = is.read_enum<bound_scale>();

        game_bounds* bounds = new game_bounds(scale);

        bounds->_lower = is.read_i32();
        bounds->_lower_valid = is.read_bool();
        bounds->_lower_relation = is.read_enum<relation>();

        bounds->_upper = is.read_i32();
        bounds->_upper_valid = is.read_bool();
        bounds->_upper_relation = is.read_enum<relation>();

        return bounds;
    }
};
