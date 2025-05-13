#pragma once

#include <type_traits>
#include <typeinfo>
#include <cassert>

class game;

//////////////////////////////////////////////////

typedef unsigned int game_type_t;


// NOLINTBEGIN(readability-identifier-naming)
namespace __game_type_impl {

game_type_t __get_game_type(const std::type_info& info);

} // namespace __game_type_impl
// NOLINTEND(readability-identifier-naming)

class i_game_type
{
public:
    i_game_type()
    {
        _type = 0; // no game has type of 0
    }

    game_type_t game_type() const
    {
        if (_type == 0)
            _type = __game_type_impl::__get_game_type(typeid(*this));

        assert(_type != 0);

#ifdef GAME_TYPE_DEBUG
        // Type is cached after first call to game_type(), but if this happens
        // during construction, the type may be incorrect
        assert(_type == __game_type_impl::__get_game_type(typeid(*this)));
#endif
        return _type;
    }

    virtual ~i_game_type()
    {
    }

private:
    // ensure the type is polymorphic
    // NOLINTNEXTLINE(readability-identifier-naming)
    virtual void __make_poly() const final
    {
        assert(false); // method shouldn't be called
    }

    mutable game_type_t _type;
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
