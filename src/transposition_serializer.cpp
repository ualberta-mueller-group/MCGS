
#include "transposition_serializer.h"

#include <iostream>
#include <type_traits>

#include "integral_conversion.h"
#include "iobuffer.h"
#include "serializer.h"
#include "transposition.h"

using namespace std;

//////////////////////////////////////////////////
struct entry_struct
{
    entry_struct() : x(0), y(0) {}

    bool operator==(const entry_struct& rhs) const
    {
        return x == rhs.x && y == rhs.y;
    }

    bool operator!=(const entry_struct& rhs) const
    {
        return !(*this == rhs);
    }

    int x;
    int y;
};

template<>
struct serializer<entry_struct>
{
    static void save(i_obuffer& os, const entry_struct& val, serializer_ctx* ctx)
    {
        os.write_i32(val.x);
        os.write_i32(val.y);
    }

    static entry_struct load(i_ibuffer& is, serializer_ctx* ctx)
    {
        entry_struct val;

        val.x = is.read_i32();
        val.y = is.read_i32();

        return val;
    }
};


/*
    TODO ttable should really use vectors instead of raw pointers...
    This will require a private ttable constructor that doesn't initialize
    the vectors.
*/
template <class Entry>
struct serializer<ttable<Entry>>
{
    using ttable_t = ttable<Entry>;

    static void save(i_obuffer& os, const ttable_t& tt, serializer_ctx* ctx)
    {
        os.write_u64(tt._n_index_bits);
        os.write_u64(tt._n_tag_bits);

        os.write_u64(tt._n_entries);

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

        serializer_load(is, tt._entries_vec, ctx);

        tt._bytes_per_tag = is.read_u64();
        serializer_load(is, tt._tags_vec, ctx);

        tt._bools_per_entry = is.read_u64();
        serializer_load(is, tt._bools_vec, ctx);
    }

};




//////////////////////////////////////////////////
void test_tt_serializer_stuff()
{
    cout << __FILE__ << ":" << __LINE__ << endl;

    typedef ttable<entry_struct> ttable_test_t;
    ttable_test_t tt1(4, 1);

    memory_obuffer os;
    serializer<ttable_test_t>::save(os, tt1, nullptr);

    memory_ibuffer is(std::move(os.get_data()));
    ttable_test_t tt2 = serializer<ttable_test_t>::load(is, nullptr);

    assert(tt1 == tt2);

    cout << __FILE__ << ":" << __LINE__ << endl;
}

