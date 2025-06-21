#pragma once
#include <cassert>
#include <type_traits>
#include <typeinfo>

// TODO remove this
// NOLINTBEGIN(readability-identifier-naming)

typedef unsigned int serializer_id_t;
typedef unsigned int game_type_t;

struct type_table_t
{
public:
    type_table_t(): _sid(0), _game_type(0)
    {
    }

    inline serializer_id_t& serializer_id()
    {
        return _sid;
    }

    inline game_type_t& game_type()
    {
        return _game_type;
    }

private:
    serializer_id_t _sid;
    game_type_t _game_type;
};

namespace __type_table_impl {
type_table_t* get_type_table(const std::type_info& tinfo);
} // namespace __type_table_impl

class i_type_table
{
public:
    i_type_table(): _type_table(nullptr)
    {
    }

    virtual ~i_type_table()
    {
    }

    type_table_t* type_table() const
    {
        if (_type_table == nullptr)
            _type_table = __type_table_impl::get_type_table(typeid(*this));

        // TODO rename me
#ifdef GAME_TYPE_DEBUG
        assert(_type_table == __type_table_impl::get_type_table(typeid(*this)));
#endif

        return _type_table;
    }

private:
    virtual void __make_poly() const final
    {
        assert(false);
    }

    mutable type_table_t* _type_table;
};

template <class T>
type_table_t* type_table()
{
    static_assert(!std::is_abstract_v<T>);
    static_assert(std::is_base_of_v<i_type_table, T>);

    static type_table_t* table = __type_table_impl::get_type_table(typeid(T));
    return table;
}

// NOLINTEND(readability-identifier-naming)
