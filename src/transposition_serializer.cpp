#include "transposition_serializer.h"

#include <iostream>
#include <type_traits>

#include "integral_conversion.h"
#include "iobuffer.h"
#include "serializer.h"
#include "transposition.h"

using namespace std;

//////////////////////////////////////////////////

/*
    TODO ttable should really use vectors instead of raw pointers...
    This will require a private ttable constructor that doesn't initialize
    the vectors.
*/
template <class Entry>
struct serializer<ttable<Entry>>
{
    using ttable_t = ttable<Entry>;

    static_assert(                                                     //
        std::is_same_v<                                                //
            uint8_t,                                                   //
            std::remove_reference_t<decltype(ttable_t::_tags_arr[0])>> //
    );                                                                 //

    static_assert(                                                      //
        std::is_same_v<                                                 //
            uint8_t,                                                    //
            std::remove_reference_t<decltype(ttable_t::_bools_arr[0])>> //
    );                                                                  //

    static void save(i_obuffer& os, const ttable_t& tt, serializer_ctx* ctx)
    {
        const size_t n_index_bits = tt.n_index_bits();
        const size_t n_entry_bools = tt.n_entry_bools();

        // Save constructor parameters
        os.write_u64(n_index_bits);
        os.write_u64(n_entry_bools);

        // Save entries
        os.write_u64(tt._entries_arr_size);

        for (size_t i = 0; i < tt._entries_arr_size; i++)
            serializer<Entry>::save(os, tt._entries_arr[i], ctx);

        // Save tags
        os.write_u64(tt._tags_arr_size);
        for (size_t i = 0; i < tt._tags_arr_size; i++)
            os.write_u8(tt._tags_arr[i]);

        // Save bools
        os.write_u64(tt._bools_arr_size);
        for (size_t i = 0; i < tt._bools_arr_size; i++)
            os.write_u8(tt._bools_arr[i]);
    }

    static ttable_t load(i_ibuffer& is, serializer_ctx* ctx)
    {
        const size_t n_index_bits = integral_cast_checked<size_t>(is.read_u64());
        const size_t n_entry_bools = integral_cast_checked<size_t>(is.read_u64());
        ttable_t tt(n_index_bits, n_entry_bools);

        _load_impl(is, tt, ctx);

        return tt;
    }

    static ttable_t* load_ptr(i_ibuffer& is, serializer_ctx* ctx)
    {
        const size_t n_index_bits = integral_cast_checked<size_t>(is.read_u64());
        const size_t n_entry_bools = integral_cast_checked<size_t>(is.read_u64());
        ttable_t* tt = new ttable_t(n_index_bits, n_entry_bools);

        _load_impl(is, *tt, ctx);

        return tt;
    }

private:
    static void _load_impl(i_ibuffer& is, ttable_t& tt, serializer_ctx* ctx)
    {
        // Load entries
        const size_t entries_arr_size =
            integral_cast_checked<size_t>(is.read_u64());

        if (entries_arr_size != tt._entries_arr_size)
            std::abort();

        for (size_t i = 0; i < tt._entries_arr_size; i++)
            tt._entries_arr[i] = serializer<Entry>::load(is, ctx);

        // Load tags
        const size_t tags_arr_size = integral_cast_checked<size_t>(is.read_u64());

        if (tags_arr_size != tt._tags_arr_size)
            std::abort();

        for (size_t i = 0; i < tt._tags_arr_size; i++)
            tt._tags_arr[i] = is.read_u8();

        // Save bools
        const size_t bools_arr_size = integral_cast_checked<size_t>(is.read_u64());

        if (bools_arr_size != tt._bools_arr_size)
            std::abort();

        for (size_t i = 0; i < tt._bools_arr_size; i++)
            tt._bools_arr[i] = is.read_u8();
    }

};

struct entry_struct
{
    entry_struct() : x(0), y(0) {}

    bool operator==(const entry_struct& rhs)
    {
        return x == rhs.x && y == rhs.y;
    }

    bool operator!=(const entry_struct& rhs)
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

