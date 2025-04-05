#pragma once

/*
    This file defines an alternate implementation of game type info, requiring
    each game's constructor to explicitly initialize its type info. This is
    a bit faster, but adds an extra step for users...
*/

static_assert(false, "This file probably shouldn't be used");

#include <stdexcept>
#include <type_traits>
#include <typeinfo>
#include <cassert>
#include <string>
#include <typeindex>

// NOLINTBEGIN(readability-identifier-naming)
class game;

typedef unsigned int game_type_t;

////////////////////////////////////////////////// implementation details
namespace __game_type_impl {

// One instance per game type. Shared by all games of that type.
struct game_type_info_t
{
    game_type_info_t() = delete;
    game_type_info_t(game_type_t type_number); 

    const game_type_t type_number;
};

// create another game_type_info_t
game_type_info_t new_game_type_info();

} // namespace __game_type_impl

////////////////////////////////////////////////// interface i_game_type
class i_game_type
{
public:
    i_game_type(): _game_type_info(nullptr)
    {
    }

    game_type_t game_type() const
    {
        _validate_game_type_info();
        return _game_type_info->type_number;
    }

    // This has to be public even though it should be private...
    __game_type_impl::game_type_info_t* _game_type_info;

private:
    // ensure the type is polymorphic
    virtual void __make_poly() const final
    {
        assert(false); // method shouldn't be called
    }

    // Ensure game has initialized its type info
    void _validate_game_type_info() const
    {
        if (_game_type_info == nullptr)
        {
            std::type_index index(typeid(*this));

            std::string why = "Game type info not initialized for class \"";
            why += index.name();
            why += "\". In the game's constructor, call \"init_game_type_info<T>(*this)\"";

            throw std::logic_error(why);
        }
    }
};

////////////////////////////////////////////////// template functions

/*
    Implementation detail. This function is where all `game_type_info_t`s
    reside.

    TODO I don't think this is undefined behaviour but it should be checked.
    This template will be instantiated in multiple translation units, and
    relies on all instantiations of the same types owning the same 
    static variable.
*/
template <class T>
__game_type_impl::game_type_info_t& __get_game_type_info()
{
    static_assert(!std::is_abstract_v<T>);
    static_assert(std::is_base_of_v<game, T>);

    static __game_type_impl::game_type_info_t info = __game_type_impl::new_game_type_info();

    return info;
}

// Get game_type_t of a game without an instance of the game
template <class T>
game_type_t game_type()
{
    static const game_type_t MY_TYPE = __get_game_type_info<T>().type_number;
    return MY_TYPE;
}

// Call this in each game's constructor
template <class T>
void init_game_type_info(i_game_type& g)
{
    assert(typeid(g) == typeid(T));
    g._game_type_info = &__get_game_type_info<T>();
}

// NOLINTEND(readability-identifier-naming)
