#pragma once

#include "serializer.h"
#include "transposition.h"

template <class Entry>
struct serializer<ttable<Entry>>
{
    using ttable_t = ttable<Entry>;

    static void save(i_obuffer& os, const ttable_t& tt, serializer_ctx* ctx)
    {
        os.write_u64(tt._n_index_bits);
        os.write_u64(tt._n_tag_bits);

        os.write_u64(tt._n_entries);

        if constexpr (!ttable_t::_ENTRY_EMPTY)
            serializer_save(os, tt._entries_vec, ctx);

        os.write_u64(tt._bytes_per_tag);
        serializer_save(os, tt._tags_vec, ctx);

        os.write_u64(tt._bools_per_entry);
        serializer_save(os, tt._bools_vec, ctx);
    }

    static ttable_t load(i_ibuffer& is, serializer_ctx* ctx)
    {
        ttable_t tt;
        _load_impl(is, tt, ctx);
        return tt;
    }

    static ttable_t* load_ptr(i_ibuffer& is, serializer_ctx* ctx)
    {
        ttable_t* tt = new ttable_t();
        _load_impl(is, *tt, ctx);
        return tt;
    }

private:
    static void _load_impl(i_ibuffer& is, ttable_t& tt, serializer_ctx* ctx)
    {
        tt._n_index_bits = is.read_u64();
        tt._n_tag_bits = is.read_u64();

        tt._n_entries = is.read_u64();

        if constexpr (!ttable_t::_ENTRY_EMPTY)
            serializer_load(is, tt._entries_vec, ctx);

        tt._bytes_per_tag = is.read_u64();
        serializer_load(is, tt._tags_vec, ctx);

        tt._bools_per_entry = is.read_u64();
        serializer_load(is, tt._bools_vec, ctx);
    }
};

