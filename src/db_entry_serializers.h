#pragma once

#include "serializer.h"
#include "iobuffer.h"
#include "database.h"
#include "serializer_lib_therm.h" // IWYU pragma: keep
#include "game_bounds_serializer.h" // IWYU pragma: keep

////////////////////////////////////////////////// db_link_t serializer
template <>
struct serializer<db_link_t>
{
    static void save(i_obuffer& os, const db_link_t& link, serializer_ctx* ctx)
    {
        hash_t hash = 0;

        const std::pair<const hash_t, db_entry_partisan>* ptr = link.get_as_pointer();

        if (ptr != nullptr)
            hash = ptr->first;

        serializer<hash_t>::save(os, hash, ctx);
    }

    static db_link_t load(i_ibuffer& is, serializer_ctx* ctx)
    {
        db_link_t link;

        const hash_t hash = serializer<hash_t>::load(is, ctx);
        link.set_as_hash(hash);

        return link;
    }
};

////////////////////////////////////////////////// serializer<db_entry_partisan>
template <>
struct serializer<db_entry_partisan>
{
    inline static void save(i_obuffer& os, const db_entry_partisan& entry,
                            serializer_ctx* ctx)
    {

#ifdef DB_INCLUDE_STRINGS
        serializer_save(os, entry.sum_string, ctx);
#endif
        serializer_save(os, entry.disk_game_type, ctx);
        serializer_save(os, entry.outcome, ctx);
        serializer_save(os, entry.thermograph, ctx);
        serializer_save(os, entry.bounds_data, ctx);
        serializer_save(os, entry.complexity, ctx);
        serializer_save(os, entry.size_score, ctx);
        serializer_save(os, entry.dominated_moves, ctx);
        serializer_save(os, entry.serialized_sum, ctx);
        serializer_save(os, entry.simplest_equal_entry, ctx);
    }

    inline static db_entry_partisan load(i_ibuffer& is, serializer_ctx* ctx)
    {
        db_entry_partisan entry;

#ifdef DB_INCLUDE_STRINGS
        serializer_load(is, entry.sum_string, ctx);
#endif
        serializer_load(is, entry.disk_game_type, ctx);
        serializer_load(is, entry.outcome, ctx);
        serializer_load(is, entry.thermograph, ctx);
        serializer_load(is, entry.bounds_data, ctx);
        serializer_load(is, entry.complexity, ctx);
        serializer_load(is, entry.size_score, ctx);
        serializer_load(is, entry.dominated_moves, ctx);
        serializer_load(is, entry.serialized_sum, ctx);
        serializer_load(is, entry.simplest_equal_entry, ctx);

        return entry;
    }
};

//////////////////////////////////////////////////
// serializer<db_entry_impartial>

template <>
struct serializer<db_entry_impartial>
{
    inline static void save(i_obuffer& os, const db_entry_impartial& entry,
                            serializer_ctx* ctx)
    {
#ifdef DB_INCLUDE_STRINGS
        serializer_save(os, entry.sum_string, ctx);
#endif
        os.write_i32(entry.nim_value);
    }

    inline static db_entry_impartial load(i_ibuffer& is, serializer_ctx* ctx)
    {
        db_entry_impartial entry;
#ifdef DB_INCLUDE_STRINGS
        serializer_load(is, entry.sum_string, ctx);
#endif
        entry.nim_value = is.read_i32();
        return entry;
    }
};

