/*
    Template db_game_generator implementation for strips and grids, using
    grid_generator.

    Generates all legal games using the given game type and grid_generator type.
    Legal games are those successfully constructed without an exception being
    thrown, and, additionally, if the game type has an is_legal() method
    (according to has_is_legal_v<T> in custom_traits.h), when is_legal() returns
    true.

    The constructors accepting both row and column dimensions are only for
    grid games, and this is enforced at compile time
*/
#pragma once
#include <exception>
#include <type_traits>
#include <cassert>
#include "db_game_generator.h"
#include "grid_generator.h"
#include "nogo_1xn.h"
#include "custom_traits.h"
#include "nogo.h"
#include "strip.h"

////////////////////////////////////////////////// gridlike_game_generator
// i.e. gridlike_db_game_generator<clobber, grid_generator_clobber>

template <class Game_T, class Generator_T>
class gridlike_db_game_generator: public db_game_generator
{
    static_assert(                                 //
        std::is_base_of_v<grid, Game_T> ||         //
        std::is_base_of_v<strip, Game_T>,          //
        "Game must be derived from grid or strip." //
    );                                             //

    static_assert(!std::is_abstract_v<Game_T>, "Game must not be abstract.");

    static_assert(                              //
        std::is_base_of_v<grid_generator, Generator_T> && //
        !std::is_abstract_v<Generator_T>        //
    );                                          //

public:
    virtual ~gridlike_db_game_generator() {}

    gridlike_db_game_generator(int max_cols);

    // Usable for grids, but not strips
    gridlike_db_game_generator(int max_rows, int max_cols);
    gridlike_db_game_generator(const int_pair& max_shape);

    operator bool() const override;
    void operator++() override;
    game* gen_game() const override;

protected:
    Generator_T _gen;

    void _init();
    bool _game_legal() const;

};


//////////////////////////////////////////////////
// gridlike_db_game_generator methods

template <class Game_T, class Generator_T>
gridlike_db_game_generator<Game_T, Generator_T>::gridlike_db_game_generator(
    int max_cols)
    : _gen(int_pair(1, max_cols))
{
    assert(max_cols >= 0);
    _init();
}

template <class Game_T, class Generator_T>
gridlike_db_game_generator<Game_T, Generator_T>::gridlike_db_game_generator(
    int max_rows, int max_cols)
    : _gen(int_pair(max_rows, max_cols))
{
    static_assert(std::is_base_of_v<grid, Game_T>,
                  "This constructor is for grids");

    assert(max_rows >= 0 && max_cols >= 0);
    _init();
}

template <class Game_T, class Generator_T>
gridlike_db_game_generator<Game_T, Generator_T>::gridlike_db_game_generator(
    const int_pair& max_shape)
    : _gen(max_shape)
{
    static_assert(std::is_base_of_v<grid, Game_T>,
                  "This constructor is for grids");

    assert(max_shape.first >= 0 && max_shape.second >= 0);
    _init();
}

template <class Game_T, class Generator_T>
inline gridlike_db_game_generator<Game_T, Generator_T>::operator bool() const
{
    return _gen;
}

template <class Game_T, class Generator_T>
void gridlike_db_game_generator<Game_T, Generator_T>::operator++()
{
    assert(*this);

    do
    {
        assert(_gen);
        ++_gen;
    }
    while (_gen && !_game_legal());
}


template <class Game_T, class Generator_T>
game* gridlike_db_game_generator<Game_T, Generator_T>::gen_game() const
{
    assert(*this);
    return new Game_T(_gen.gen_board());
}

template <class Game_T, class Generator_T>
void gridlike_db_game_generator<Game_T, Generator_T>::_init()
{
    if (!_gen)
    {
        assert(!*this);
        return;
    }

    if (_game_legal())
    {
        assert(*this);
        return;
    }

    ++(*this);
}

template <class Game_T, class Generator_T>
bool gridlike_db_game_generator<Game_T, Generator_T>::_game_legal() const
{
    assert(_gen);

    try
    {
        Game_T g(_gen.gen_board());

        if constexpr (has_is_legal_v<Game_T>)
            return g.is_legal();

        return true;
    }
    catch (std::exception& e)
    {
        return false;
    }

    assert(false);
}

