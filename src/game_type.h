#pragma once

#include <type_traits>
#include <typeinfo>
#include <cassert>

class game;

//////////////////////////////////////////////////

typedef unsigned int game_type_t;

namespace __game_type_impl {                             // NOLINT
game_type_t __get_game_type(const std::type_info& info); // NOLINT
} // namespace __game_type_impl

class i_game_type
{
public:
    game_type_t game_type() const
    {
        // can't save this in a static variable because this method is used by
        // all game classes
        return __game_type_impl::__get_game_type(typeid(*this));
    }

private:
    // ensure the type is polymorphic
    virtual void __make_poly() const final // NOLINT
    {
        assert(false); // method shouldn't be called
    }
};

template <class T>
game_type_t game_type()
{
    static_assert(!std::is_abstract_v<T>);
    static_assert(std::is_base_of_v<game, T>);

    static const game_type_t MY_TYPE =
        __game_type_impl::__get_game_type(typeid(T));
    return MY_TYPE;
}
