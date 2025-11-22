/*
    Run-time type information. See development-notes.md
*/
#pragma once
#include <cassert>
#include <type_traits>
#include <cstdint>
#include <typeinfo>

////////////////////////////////////////////////// typedefs
typedef uint32_t dyn_serializable_id_t;
typedef uint32_t game_type_t;

////////////////////////////////////////////////// struct type_table_t

/*
   TODO:
   - Make some of these const?
   - Move implementations for fields back into some type_table related file,
     rather than consumer files i.e. game.h? Currently implementation is
     fragmented across several files to avoid problems with including many
     headers across the project, but this is probably not necessary?

*/
struct type_table_t
{
public:
    type_table_t();

    // dyn_serializable_id methods
    dyn_serializable_id_t dyn_serializable_id();
    dyn_serializable_id_t& dyn_serializable_id_ref();

    // game_type methods
    game_type_t game_type();
    game_type_t& game_type_ref();

    // grid_hash symmetry mask
    unsigned int grid_hash_mask() const;
    void set_grid_hash_mask(unsigned int mask);

    /*
       Some fields must be explicitly initialized during mcgs_init by the
       programmer. Modification of these will be locked afterward, once this
       set_initialized() function is called.

       Fields which are not explicitly initialized (i.e. game type and
       serialization IDs) will still be automatically initialized later
       as they're used.
    */
    static void set_initialized();

private:
    dyn_serializable_id_t _sid;
    game_type_t _game_type;
    unsigned int _grid_hash_mask;

    static bool _initialized;
};

////////////////////////////////////////////////// implementation details

// NOLINTNEXTLINE(readability-identifier-naming)
namespace __type_table_impl {
type_table_t* get_type_table(const std::type_info& tinfo);
} // namespace __type_table_impl

////////////////////////////////////////////////// class i_type_table
class i_type_table
{
public:
    i_type_table();

    virtual ~i_type_table() {}

    type_table_t* type_table() const;

private:
    // Ensure polymorphic type
    // NOLINTNEXTLINE(readability-identifier-naming)
    virtual void __make_poly() const final { assert(false); }

    mutable type_table_t* _type_table;
};

////////////////////////////////////////////////// type_table_t methods
inline type_table_t::type_table_t() : _sid(0), _game_type(0), _grid_hash_mask(0)
{
}

inline dyn_serializable_id_t type_table_t::dyn_serializable_id()
{
    return _sid;
}

inline dyn_serializable_id_t& type_table_t::dyn_serializable_id_ref()
{
    return _sid;
}

inline game_type_t type_table_t::game_type()
{
    return _game_type;
}

inline game_type_t& type_table_t::game_type_ref()
{
    return _game_type;
}

inline unsigned int type_table_t::grid_hash_mask() const
{
    // Must initialize in some mcgs_init function before using
    assert(_initialized && _grid_hash_mask != 0);
    return _grid_hash_mask;
}

inline void type_table_t::set_grid_hash_mask(unsigned int mask)
{
    // Field is only modifiable during mcgs_init, and not after.
    assert(!_initialized && _grid_hash_mask == 0);
    _grid_hash_mask = mask;
}

////////////////////////////////////////////////// i_type_table methods
inline i_type_table::i_type_table() : _type_table(nullptr)
{
}

inline type_table_t* i_type_table::type_table() const
{
    if (_type_table == nullptr)
        _type_table = __type_table_impl::get_type_table(typeid(*this));

#ifdef TYPE_TABLE_DEBUG
    assert(_type_table == __type_table_impl::get_type_table(typeid(*this)));
#endif

    return _type_table;
}

////////////////////////////////////////////////// type_table<T>() function
template <class T>
inline type_table_t* type_table()
{
    static_assert(std::is_base_of_v<i_type_table, T>);
    static_assert(!std::is_abstract_v<T>);

    static type_table_t* table = __type_table_impl::get_type_table(typeid(T));
    return table;
}
