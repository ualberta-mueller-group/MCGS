#pragma once

#include "serializer.h"
#include "database.h"
#include "serializer_lib_therm.h" // IWYU pragma: keep
#include "game_bounds_serializer.h" // IWYU pragma: keep

////////////////////////////////////////////////// serializer<db_entry_partisan>
template <>
struct serializer<db_entry_partisan>
{
    inline static void save(obuffer& os, const db_entry_partisan& entry,
                            serializer_ctx* ctx)
    {
        serializer_save(os, entry.outcome, ctx);
        serializer_save(os, entry.thermograph, ctx);
        serializer_save(os, entry.bounds_data, ctx);
        serializer_save(os, entry.complexity, ctx);
        serializer_save(os, entry.dominated_moves, ctx);
    }

    inline static db_entry_partisan load(ibuffer& is, serializer_ctx* ctx)
    {
        db_entry_partisan entry;

        serializer_load(is, entry.outcome, ctx);
        serializer_load(is, entry.thermograph, ctx);
        serializer_load(is, entry.bounds_data, ctx);
        serializer_load(is, entry.complexity, ctx);
        serializer_load(is, entry.dominated_moves, ctx);

        return entry;
    }
};

//////////////////////////////////////////////////
// serializer<db_entry_impartial>

template <>
struct serializer<db_entry_impartial>
{
    inline static void save(obuffer& os, const db_entry_impartial& entry,
                            serializer_ctx* ctx)
    {
        os.write_i32(entry.nim_value);
    }

    inline static db_entry_impartial load(ibuffer& is, serializer_ctx* ctx)
    {
        db_entry_impartial entry;
        entry.nim_value = is.read_i32();
        return entry;
    }
};

