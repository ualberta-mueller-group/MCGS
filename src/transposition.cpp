#include "transposition.h"
#include <memory>
#include <sstream>

#include "clobber_1xn.h"
#include "sumgame.h"

////////////////////////////////////////////////////////////////////// design
/*

    ttable<T>
    ttable<T>::iterator
    ttable_entry_sumgame
*/

////////////////////////////////////////////////////////////////////// .h file

#include <cstdint>
#include <sstream>
#include <type_traits>
#include <vector>
#include <iostream>
#include "hashing.h"
#include "utilities.h"

class ttable_entry_non_empty
{
public:
    uint8_t win;
};

class ttable_entry_empty
{
public:

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
            _table._reset(_table_idx, _tag, new_valid);
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
        : _n_entries(1 << index_width),
        _index_width(index_width),
        _tag_width(size_in_bits<hash_t>() - index_width),
        _n_bits(extra_bits + 1)

    {
        assert(index_width > 0);
        assert(index_width <= size_in_bits<hash_t>());

        // _entries
        if constexpr (!_ENTRY_EMPTY)
            _entries.resize(_n_entries);
        else
            _entries.resize(1);

        // _bits
        const size_t bits_total = (_n_bits * _n_entries);
        const size_t bits_vec_size = (bits_total / size_in_bits<unsigned int>()) + 1;
        _bits.resize(bits_vec_size);

        // _tags
        const size_t full_bytes_per_tag = _tag_width / size_in_bits<uint8_t>();
        const size_t partial_bytes_per_tag = (full_bytes_per_tag * size_in_bits<uint8_t>()) < _tag_width;
        _n_bytes_per_tag = full_bytes_per_tag + partial_bytes_per_tag;
        const size_t tags_vec_size = _n_bytes_per_tag * _n_entries;
        _tags.resize(tags_vec_size);

        // TODO: approximate total memory usage
        uint64_t total_bytes = 0;
        total_bytes += _entries.size() * sizeof(Entry);
        total_bytes += _bits.size() * sizeof(unsigned int);
        total_bytes += _tags.size() * sizeof(uint8_t);

        std::cout << "Approximate table size: " << (total_bytes / (1024 * 1024)) << " MiB" << std::endl;
        std::cout << _entries.size() << std::endl;
        std::cout << _bits.size() << std::endl;
        std::cout << _tags.size() << std::endl;


        std::cout << "IS EMPTY: " << _ENTRY_EMPTY << std::endl;
    }

    iterator get(const hash_t& hash)
    {
        const hash_t index = _extract_index(hash);
        const hash_t tag = _extract_tag(hash);

        assert((index | (tag << _index_width)) == hash);

        Entry* entry_ptr = _get_entry_ptr(index);

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
        assert(entry_idx < _n_entries);
        assert(bit_idx < _n_bits);

        const size_t global_bit_idx = _n_bits * entry_idx + bit_idx;
        bits_vec_idx = global_bit_idx / sizeof(unsigned int);
        element_bit_idx = global_bit_idx % sizeof(unsigned int);
    }

    bool _get_bit(size_t entry_idx, size_t bit_idx) const
    {
        assert(entry_idx < _n_entries);
        assert(bit_idx < _n_bits);

        size_t bits_vec_idx;
        size_t element_bit_idx;
        _get_bit_location(entry_idx, bit_idx, bits_vec_idx, element_bit_idx);

        const unsigned int& element = _bits[bits_vec_idx];
        return (element >> element_bit_idx) & 0x1;
    }

    void _set_bit(size_t entry_idx, size_t bit_idx, bool new_val)
    {
        assert(entry_idx < _n_entries);
        assert(bit_idx < _n_bits);

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
        assert(entry_idx < _n_entries);

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
        assert(entry_idx < _n_entries);

        const size_t tag_start = _n_bytes_per_tag * entry_idx;

        for (size_t i = 0; i < _n_bytes_per_tag; i++)
        {
            const size_t idx = tag_start + i;
            const uint8_t tag_byte = (tag >> (i * size_in_bits<uint8_t>())) & ((uint8_t) -1);
            _tags[idx] = tag_byte;
        }
    }

    void _reset(size_t entry_idx, hash_t tag, bool new_valid)
    {
        assert(entry_idx < _n_entries);

        if (!new_valid)
        {
            _set_bit(entry_idx, 0, false);
            return;
        }

        Entry* entry_ptr = _get_entry_ptr(entry_idx);
        *entry_ptr = Entry();

        _set_tag(entry_idx, tag);

        for (size_t i = 1; i < _n_bits; i++)
        {
            _set_bit(entry_idx, i, false);
        }
        _set_bit(entry_idx, 0, true);
    }

    Entry* _get_entry_ptr(size_t entry_idx)
    {
        assert(entry_idx < _n_entries);

        if constexpr (_ENTRY_EMPTY)
            return &_entries[0];

        return &_entries[entry_idx];
    }

    std::vector<Entry> _entries;
    std::vector<unsigned int> _bits;
    std::vector<uint8_t> _tags;

    static constexpr bool _ENTRY_EMPTY = std::is_empty_v<Entry>;

    const size_t _n_entries;

    const size_t _index_width;
    const size_t _tag_width;
    size_t _n_bytes_per_tag; // TODO const
    const size_t _n_bits;
};


////////////////////////////////////////////////////////////////////// .cpp file

typedef ttable<ttable_entry_non_empty> ttable_non_empty;
typedef ttable<ttable_entry_empty> ttable_empty;

using namespace std;

void test_transposition()
{
    {
        ttable_non_empty t1(28, 0);
    }

    {
        ttable_empty t2(28, 1);
    }

}
