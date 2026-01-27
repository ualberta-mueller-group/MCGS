/*
    Transposition table template ttable<T>

    TODO: Currently each entry's valid bit is inside of the packed bools
          array. This means ttable::clear() will take longer if the table is
          constructed with packed bools. These two types of bools should
          be separated in the future
*/
#pragma once
#include "utilities.h"

#include "hashing.h"
#include <algorithm>
#include <cstddef>
#include <cassert>
#include <cstring>
#include <type_traits>
#include <optional>
#include <iostream>
#include "throw_assert.h"
#include "global_options.h"


////////////////////////////////////////////////// class ttable
template <class Entry>
class ttable
{
public:
    class search_result
    {
    public:
        bool entry_valid() const;

        Entry& get_entry();
        const Entry& get_entry() const;

        void set_entry(const Entry& entry);

        void init_entry();
        void init_entry(const Entry& entry);

        bool get_bool(size_t bool_idx) const;
        void set_bool(size_t bool_idx, bool new_val);

    private:
        search_result() = delete;
        search_result(ttable<Entry>& table, hash_t idx, hash_t tag,
                      Entry* entry_ptr);

        ttable<Entry>& _table;
        const hash_t _entry_idx;
        const hash_t _entry_tag;
        Entry* _entry_ptr;

        friend ttable<Entry>;
    };

    ttable(size_t index_bits, size_t entry_bools);
    ~ttable();

    // no copy/move
    ttable(const ttable& rhs) = delete;
    ttable& operator=(const ttable& rhs) = delete;
    ttable(ttable&& rhs) = delete;
    ttable& operator=(ttable&& rhs) = delete;

    search_result search(hash_t hash);

    void store(hash_t hash, const Entry& entry);
    std::optional<Entry> get(hash_t hash) const;

    void clear();

    size_t n_index_bits() const;
    size_t n_entry_bools() const;

private:
    inline hash_t _extract_index(hash_t hash) const;
    inline hash_t _extract_tag(hash_t hash) const;

    inline void _get_bool_indices(hash_t entry_idx, size_t bool_idx,
                                  size_t& bools_arr_idx,
                                  size_t& element_bit_no) const;

    bool _get_bool(hash_t entry_idx, size_t bool_idx) const;
    void _set_bool(hash_t entry_idx, size_t bool_idx, bool new_val);

    hash_t _get_tag(hash_t entry_idx) const;
    void _set_tag(hash_t entry_idx, hash_t tag);

    void _clear_tags_and_bools();
    void _clear_bools();

    inline Entry* _get_entry_ptr(hash_t entry_idx);

    void _init_entry(hash_t index, hash_t tag, const Entry& entry);

    static constexpr bool _ENTRY_EMPTY = std::is_empty_v<Entry>;

    const size_t _n_index_bits;
    const size_t _n_tag_bits;

    const size_t _n_entries;

    Entry* _entries_arr;
    size_t _entries_arr_size;

    size_t _bytes_per_tag;
    uint8_t* _tags_arr;
    size_t _tags_arr_size;

    const size_t _bools_per_entry;
    unsigned int* _bools_arr;
    size_t _bools_arr_size;
};

////////////////////////////////////////////////// ttable<Entry> implementation
template <class Entry>
ttable<Entry>::ttable(size_t index_bits, size_t n_packed_bools)
    : _n_index_bits(index_bits),
      _n_tag_bits(size_in_bits<hash_t>() - index_bits),
      _n_entries(size_t(1) << index_bits),
      _bools_per_entry(1 + n_packed_bools) // +1 for valid bit
{
    assert(index_bits > 0);
    // avoid shifting entire width of hash_t or size_t
    assert(index_bits < size_in_bits<hash_t>() &&
           index_bits < size_in_bits<size_t>());

    //// Initialize numeric variables
    // entries
    if constexpr (_ENTRY_EMPTY)
        _entries_arr_size = 1;
    else
        _entries_arr_size = _n_entries;

    // tags
    const size_t byte_bits = size_in_bits<uint8_t>();
    const size_t full_bytes_per_tag = _n_tag_bits / byte_bits;
    const size_t partial_bytes_per_tag = (_n_tag_bits % byte_bits) != 0;
    _bytes_per_tag = full_bytes_per_tag + partial_bytes_per_tag;

    _tags_arr_size = _n_entries * _bytes_per_tag;

    THROW_ASSERT((_n_entries == (_tags_arr_size / _bytes_per_tag)) &&   //
                     (_bytes_per_tag == (_tags_arr_size / _n_entries)), //
                 "ttable too large!");

    // bools
    const size_t total_bools = _n_entries * _bools_per_entry;

    THROW_ASSERT(LOGICAL_IMPLIES(
                     (_bools_per_entry != 0),                               //
                     (_n_entries == (total_bools / _bools_per_entry)) &&    //
                         (_bools_per_entry == (total_bools / _n_entries))), //
                 "ttable has too many packed bools!");

    _bools_arr_size = 1 + (total_bools / size_in_bits<unsigned int>());


    //// Estimate memory cost
    // TODO: DEBUG PRINTING
    if (global::print_ttable_size())
    {
        uint64_t byte_count = 0;
        byte_count += _entries_arr_size * sizeof(Entry);
        byte_count += _tags_arr_size * sizeof(uint8_t);
        byte_count += _bools_arr_size * sizeof(unsigned int);
        double byte_count_formatted = ((double) byte_count) / (1024.0 * 1024.0);
        std::cout << "Estimated table size: " << byte_count_formatted;
        std::cout << " MiB" << std::endl;

        // std::cout << "Estimated table size: " << byte_count;
        // std::cout << " B" << std::endl;
    }

    //// Initialize arrays
    _entries_arr = new Entry[_entries_arr_size];
    for (size_t i = 0; i < _entries_arr_size; i++)
        _entries_arr[i] = Entry();
    /* TODO: above loop could maybe be optimized using some type traits to
       optionally remove it. It's there to reduce page faults during
       search (by ensuring all pages of the array have been written to).

       std::is_trivially_default_constructible_v<T> doesn't seem quite right
       for this...

       This is probably unimportant
    */

    _tags_arr = new uint8_t[_tags_arr_size];
    _bools_arr = new unsigned int[_bools_arr_size];
    _clear_tags_and_bools();
}

template <class Entry>
ttable<Entry>::~ttable()
{
    delete[] _entries_arr;
    delete[] _tags_arr;
    delete[] _bools_arr;
}

template <class Entry>
typename ttable<Entry>::search_result ttable<Entry>::search(hash_t hash)
{
    const hash_t index = _extract_index(hash);
    const hash_t tag = _extract_tag(hash);
    assert(hash == (index | (tag << _n_index_bits)));

    Entry* entry_ptr = _get_entry_ptr(index);

    return search_result(*this, index, tag, entry_ptr);
}

template <class Entry>
void ttable<Entry>::store(hash_t hash, const Entry& entry)
{
    /*
        When using this function, the ttable must be constructed with
        n_packed_bools = 0, to avoid accidentally resetting bools

        NOTE: _bools_per_entry is always 1 more than the n_packed_bools passed
        to the ttable constructor, as it's used internally
    */
    THROW_ASSERT_DEBUG(_bools_per_entry == 1);

    ttable<Entry>::search_result tt_result = search(hash);
    tt_result.set_entry(entry);
}

template <class Entry>
std::optional<Entry> ttable<Entry>::get(hash_t hash) const
{
    ttable<Entry>& tt_mutable = const_cast<ttable<Entry>&>(*this);
    ttable<Entry>::search_result tt_result = tt_mutable.search(hash);

    if (tt_result.entry_valid())
        return std::optional<Entry>(tt_result.get_entry());

    return std::optional<Entry>();
}

template <class Entry>
void ttable<Entry>::clear()
{
    //_clear_tags_and_bools();
    _clear_bools();
}

template <class Entry>
inline size_t ttable<Entry>::n_index_bits() const
{
    return _n_index_bits;
}

template <class Entry>
inline size_t ttable<Entry>::n_entry_bools() const
{
    assert(_bools_per_entry > 0);
    return _bools_per_entry - 1; // -1 due to valid bit
}

template <class Entry>
inline hash_t ttable<Entry>::_extract_index(hash_t hash) const
{
    static_assert(!std::is_signed_v<hash_t>);
    const size_t shift_width = size_in_bits<hash_t>() - _n_index_bits;
    return (hash << shift_width) >> shift_width;
}

template <class Entry>
inline hash_t ttable<Entry>::_extract_tag(hash_t hash) const
{
    static_assert(!std::is_signed_v<hash_t>);
    return hash >> _n_index_bits;
}

template <class Entry>
inline void ttable<Entry>::_get_bool_indices(hash_t entry_idx, size_t bool_idx,
                                             size_t& bools_arr_idx,
                                             size_t& element_bit_no) const
{
    assert(entry_idx < _n_entries);
    assert(bool_idx < _bools_per_entry);

    size_t global_bit_idx = entry_idx * _bools_per_entry + bool_idx;
    bools_arr_idx = global_bit_idx / size_in_bits<unsigned int>();
    element_bit_no = global_bit_idx % size_in_bits<unsigned int>();

    assert(bools_arr_idx < _bools_arr_size);
    assert(element_bit_no < size_in_bits<unsigned int>());
}

template <class Entry>
bool ttable<Entry>::_get_bool(hash_t entry_idx, size_t bool_idx) const
{
    assert(entry_idx < _n_entries);
    THROW_ASSERT_DEBUG(bool_idx < _bools_per_entry);

    size_t bools_arr_idx;
    size_t element_bit_no;
    _get_bool_indices(entry_idx, bool_idx, bools_arr_idx, element_bit_no);

    assert(bools_arr_idx < _bools_arr_size);
    const unsigned int& element = _bools_arr[bools_arr_idx];
    return (element >> element_bit_no) & 0x1;
}

template <class Entry>
void ttable<Entry>::_set_bool(hash_t entry_idx, size_t bool_idx, bool new_val)
{
    assert(entry_idx < _n_entries);
    THROW_ASSERT_DEBUG(bool_idx < _bools_per_entry);

    size_t bools_arr_idx;
    size_t element_bit_no;
    _get_bool_indices(entry_idx, bool_idx, bools_arr_idx, element_bit_no);

    const unsigned int bit_mask = ((unsigned int) 1) << element_bit_no;
    const unsigned int inv_bit_mask = ~bit_mask;

    assert(bools_arr_idx < _bools_arr_size);
    unsigned int& element = _bools_arr[bools_arr_idx];

    if (new_val)
        element |= bit_mask; // set bit
    else
        element &= inv_bit_mask; // unset bit
}

template <class Entry>
hash_t ttable<Entry>::_get_tag(hash_t entry_idx) const
{
    assert(entry_idx < _n_entries);

    hash_t tag = 0;

    const size_t tag_start = entry_idx * _bytes_per_tag;

    for (size_t i = 0; i < _bytes_per_tag; i++)
    {
        const size_t idx = tag_start + i;
        const uint8_t& tag_byte = _tags_arr[idx];

        tag |= ((hash_t) tag_byte) << (i * size_in_bits<uint8_t>());
    }

    return tag;
}

template <class Entry>
void ttable<Entry>::_set_tag(hash_t entry_idx, hash_t tag)
{
    assert(entry_idx < _n_entries);

    const size_t tag_start = entry_idx * _bytes_per_tag;

    for (size_t i = 0; i < _bytes_per_tag; i++)
    {
        const uint8_t byte =
            (tag >> (i * size_in_bits<uint8_t>())) & ((uint8_t) -1);

        const size_t idx = tag_start + i;
        _tags_arr[idx] = byte;
    }
}

template <class Entry>
inline void ttable<Entry>::_clear_tags_and_bools()
{
    assert(_tags_arr != nullptr);
    std::fill_n(_tags_arr, _tags_arr_size, 0);

    _clear_bools();
}

template <class Entry>
inline void ttable<Entry>::_clear_bools()
{
    assert(_bools_arr != nullptr);
    std::fill_n(_bools_arr, _bools_arr_size, 0);
}

template <class Entry>
inline Entry* ttable<Entry>::_get_entry_ptr(hash_t entry_idx)
{
    assert(entry_idx < _n_entries);

    if constexpr (_ENTRY_EMPTY)
        return &_entries_arr[0];

    return &_entries_arr[entry_idx];
}

template <class Entry>
void ttable<Entry>::_init_entry(hash_t index, hash_t tag, const Entry& entry)
{
    assert(index < _n_entries);

    _set_tag(index, tag);

    _set_bool(index, 0, true); // valid bit
    for (size_t i = 1; i < _bools_per_entry; i++)
        _set_bool(index, i, false);

    Entry* entry_ptr = _get_entry_ptr(index);
    *entry_ptr = entry;
}

////////////////////////////////////////////////// ttable<Entry>::search_result
/// implementation
template <class Entry>
bool ttable<Entry>::search_result::entry_valid() const
{
    return _table._get_bool(_entry_idx, 0) &&
           (_table._get_tag(_entry_idx) == _entry_tag);
}

template <class Entry>
Entry& ttable<Entry>::search_result::get_entry()
{
    THROW_ASSERT_DEBUG(entry_valid());
    assert(_entry_ptr != nullptr);
    return *_entry_ptr;
}

template <class Entry>
const Entry& ttable<Entry>::search_result::get_entry() const
{
    THROW_ASSERT_DEBUG(entry_valid());
    assert(_entry_ptr != nullptr);
    return *_entry_ptr;
}

template <class Entry>
void ttable<Entry>::search_result::set_entry(const Entry& entry)
{
    if (!entry_valid())
    {
        init_entry(entry);
        return;
    }

    assert(_entry_ptr != nullptr);
    *_entry_ptr = entry;
}

template <class Entry>
void ttable<Entry>::search_result::init_entry()
{
    _table._init_entry(_entry_idx, _entry_tag, Entry());
}

template <class Entry>
void ttable<Entry>::search_result::init_entry(const Entry& entry)
{
    _table._init_entry(_entry_idx, _entry_tag, entry);
}

template <class Entry>
bool ttable<Entry>::search_result::get_bool(size_t bool_idx) const
{
    THROW_ASSERT_DEBUG(entry_valid());

    // +1 because index 0 is valid bit
    return _table._get_bool(_entry_idx, bool_idx + 1);
}

template <class Entry>
void ttable<Entry>::search_result::set_bool(size_t bool_idx, bool new_val)
{
    THROW_ASSERT_DEBUG(entry_valid());

    // +1 because index 0 is valid bit
    _table._set_bool(_entry_idx, bool_idx + 1, new_val);
}

template <class Entry>
ttable<Entry>::search_result::search_result(ttable<Entry>& table, hash_t idx,
                                            hash_t tag, Entry* entry_ptr)
    : _table(table), _entry_idx(idx), _entry_tag(tag), _entry_ptr(entry_ptr)
{
}
