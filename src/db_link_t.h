/*
    Represents either `hash_t` or `std::pair<hash_t, db_entry_partisan>*`

    Save to disk as `hash_t`. Load from disk as `hash_t`, and then convert to
    pointer once all entries are loaded.
*/
#pragma once

#include <utility>

#include "hashing.h"
#include "serializer.h"
//#include "database.h"

struct db_entry_partisan;

////////////////////////////////////////////////// class db_link_t
class db_link_t
{
public:
    db_link_t();

    void set_as_hash(hash_t hash);
    void set_as_pointer(std::pair<const hash_t, db_entry_partisan>* pointer);

    hash_t get_as_hash() const;
    std::pair<const hash_t, db_entry_partisan>* get_as_pointer() const;

    bool equal_as_pointers(const db_link_t& other) const;

private:
    friend serializer<db_link_t>;

    union db_link_union_t
    {
        hash_t as_hash;
        std::pair<const hash_t, db_entry_partisan>* as_pointer;
    };

    db_link_union_t _hash_or_pointer;
};


////////////////////////////////////////////////// db_link_t methods
inline db_link_t::db_link_t()
{
    _hash_or_pointer.as_hash = 0;
}

inline void db_link_t::set_as_hash(hash_t hash)
{
    _hash_or_pointer.as_hash = hash;
}

inline void db_link_t::set_as_pointer(
    std::pair<const hash_t, db_entry_partisan>* pointer)
{
    _hash_or_pointer.as_pointer = pointer;
}

inline hash_t db_link_t::get_as_hash() const
{
    return _hash_or_pointer.as_hash;
}

inline std::pair<const hash_t, db_entry_partisan>* db_link_t::get_as_pointer() const
{
    return _hash_or_pointer.as_pointer;
}

