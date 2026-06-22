#include "db_link_t.h"
#include "database.h" // IWYU pragma: keep

using namespace std;

bool db_link_t::equal_as_pointers(const db_link_t& other) const
{
    const pair<const hash_t, db_entry_partisan>* ptr1 = get_as_pointer();
    const pair<const hash_t, db_entry_partisan>* ptr2 = other.get_as_pointer();

    if ((ptr1 == nullptr) != (ptr2 == nullptr))
        return false;

    if (ptr1 == nullptr)
        return true;

    const hash_t hash1 = ptr1->first;
    const hash_t hash2 = ptr2->first;

    return hash1 == hash2;
}


