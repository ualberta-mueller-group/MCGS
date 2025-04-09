#include "transposition.h"

////////////////////////////////////////////////////////////////////// design
/*

    ttable<T>
    ttable<T>::iterator
    ttable_entry_sumgame
*/

////////////////////////////////////////////////////////////////////// .h file

#include <cstdint>
#include <type_traits>
#include <vector>
#include <iostream>
#include "hashing.h"
#include "utilities.h"

class ttable_entry_sumgame
{
public:
    std::string sum_string; // for testing purposes
};

template <class Entry>
class ttable
{
public:
    class iterator
    {
    public:
        bool is_valid() const
        {
            return _table._get_bit(_table_idx, 0) && (_table._get_tag(_table_idx) == _tag);
        }

        void set_valid(bool new_valid)
        {
            _table[_table_idx] = Entry();
            _table._set_bit(_table_idx, 0, new_valid);
            _table._set_tag(_table_idx, _tag * new_valid);
        }

        bool get_bit(size_t bit_idx) const
        {
            assert(is_valid());
            return _table._get_bit(_table_idx, bit_idx + 1);
        }

        void set_bit(size_t bit_idx, bool new_val)
        {
            assert(is_valid());
            _table._set_bit(_table_idx, bit_idx + 1, new_val);
        }

        Entry& entry()
        {
            assert(is_valid());
            assert(_entry_ptr != nullptr);

            return *_entry_ptr;
        }

    private:
        iterator(ttable<Entry>& table, size_t table_idx, hash_t tag, Entry* entry_ptr)
            : _table(table),
            _table_idx(table_idx),
            _tag(tag),
            _entry_ptr(entry_ptr)
        {
        }

        ttable<Entry>& _table;
        const size_t _table_idx;
        const hash_t _tag;
        Entry* _entry_ptr;

        friend ttable<Entry>;
    };

    ttable(size_t index_width, size_t extra_bits)
        : _index_width(index_width),
        _tag_width(size_in_bits<hash_t>() - index_width),
        _n_bits(extra_bits + 1)

    {
        assert(index_width > 0);
        assert(index_width <= size_in_bits<hash_t>());

        // _entries
        const size_t entry_count = (1 << index_width);
        _entries.resize(entry_count);

        // _bits
        const size_t bits_per_entry = 1 + extra_bits;
        const size_t bits_total = (bits_per_entry * entry_count);
        const size_t bits_vec_size = (bits_total / sizeof(unsigned int)) + 1;
        _bits.resize(bits_vec_size);

        // _tags
        const size_t full_bytes_per_tag = _tag_width / size_in_bits<uint8_t>();
        const size_t partial_bytes_per_tag = (full_bytes_per_tag * size_in_bits<uint8_t>()) > 0;
        _n_bytes_per_tag = full_bytes_per_tag + partial_bytes_per_tag;
        const size_t tags_vec_size = _n_bytes_per_tag * entry_count;
        _tags.resize(tags_vec_size);

        // TODO: total memory usage
    }

    iterator get(const hash_t& hash)
    {
        const hash_t index = _extract_index(hash);
        const hash_t tag = _extract_tag(hash);

        assert(index < _entries.size());
        Entry* entry_ptr = &(_entries[index]);

        return iterator(*this, index, tag, entry_ptr);
    }

private:
    inline hash_t _extract_index(const hash_t& hash) const
    {
        assert(_index_width > 0);

        using Hash_Unsigned = std::make_unsigned<hash_t>::type; // NOLINT
        const Hash_Unsigned& hash_u = reinterpret_cast<const Hash_Unsigned&>(hash);

        const size_t hash_width = size_in_bits<hash_t>();
        const size_t shift_distance = hash_width - _index_width;

        return (hash_u << shift_distance) >> shift_distance;
    }

    inline hash_t _extract_tag(const hash_t& hash) const
    {
        return (hash >> _index_width);
    }

    void _get_bit_location(size_t entry_idx, size_t bit_idx, size_t& bits_vec_idx, size_t& element_bit_idx) const
    {
        const size_t global_bit_idx = _n_bits * entry_idx + bit_idx;
        bits_vec_idx = global_bit_idx / sizeof(unsigned int);
        element_bit_idx = global_bit_idx % sizeof(unsigned int);
    }

    bool _get_bit(size_t entry_idx, size_t bit_idx) const
    {
        size_t bits_vec_idx;
        size_t element_bit_idx;
        _get_bit_location(entry_idx, bit_idx, bits_vec_idx, element_bit_idx);

        const unsigned int& element = _bits[bits_vec_idx];
        return (element >> element_bit_idx) & 0x1;
    }

    void _set_bit(size_t entry_idx, size_t bit_idx, bool new_val)
    {
        size_t bits_vec_idx;
        size_t element_bit_idx;
        _get_bit_location(entry_idx, bit_idx, bits_vec_idx, element_bit_idx);

        unsigned int& element = _bits[bits_vec_idx];
        const unsigned int bit_mask = ((unsigned int) 1) << element_bit_idx;
        const unsigned int inverse_bit_mask = ~bit_mask;

        element = (element & inverse_bit_mask) | (bit_mask * new_val);
    }

    hash_t _get_tag(size_t entry_idx) const
    {
        hash_t tag = 0;

        const size_t tag_start = _n_bytes_per_tag * entry_idx;

        for (size_t i = 0; i < _n_bytes_per_tag; i++)
        {
            const size_t idx = tag_start + i;
            const uint8_t tag_byte = _tags[idx];
            tag |= ((hash_t) (tag_byte)) << (i * size_in_bits<uint8_t>());
        }

        return tag;
    }

    void _set_tag(size_t entry_idx, const hash_t& tag)
    {
        const size_t tag_start = _n_bytes_per_tag * entry_idx;

        for (size_t i = 0; i < _n_bytes_per_tag; i++)
        {
            const size_t idx = tag_start + i;
            const uint8_t tag_byte = (tag >> (i * size_in_bits<uint8_t>())) & ((uint8_t) -1);
            _tags[idx] = tag_byte;
        }
    }

    std::vector<Entry> _entries;
    std::vector<unsigned int> _bits;
    std::vector<uint8_t> _tags;

    const size_t _index_width;
    const size_t _tag_width;
    size_t _n_bytes_per_tag; // TODO const
    const size_t _n_bits;
};



typedef ttable<ttable_entry_sumgame> ttable_sumgame_t;


////////////////////////////////////////////////////////////////////// .cpp file

using namespace std;


void test_transposition()
{
    ttable_sumgame_t tt(16, 0);

}
