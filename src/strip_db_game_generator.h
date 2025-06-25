#pragma once
#include <exception>
#include <type_traits>
#include "db_game_generator.h"
#include "grid_utils.h"
#include "nogo_1xn.h"
#include "strip.h"

////////////////////////////////////////////////// class strip_db_game_generator<T>
template <class T>
class strip_db_game_generator: public db_game_generator
{
    static_assert(std::is_base_of_v<strip, T>);
    static_assert(!std::is_abstract_v<T>);

public:
    strip_db_game_generator(int board_size);

    operator bool() const override;
    void operator++() override;
    game* gen_game() const override;

private:
    void _next(bool init);
    bool _current_board_valid() const;

    grid_generator _grid_gen;
};

////////////////////////////////////////////////// strip_db_game_generator methods
/*
   TODO this is inefficient -- shouldn't have to create game objects,
   especially not with operator new...
*/

template <class T>
inline strip_db_game_generator<T>::strip_db_game_generator(int board_size)
    : _grid_gen(board_size)
{
    assert(board_size >= 0);
    _next(true);
}

template <class T>
inline strip_db_game_generator<T>::operator bool() const
{
    return _grid_gen;
}

template <class T>
inline void strip_db_game_generator<T>::operator++()
{
    assert(*this);
    _next(false);
}

template <class T>
inline game* strip_db_game_generator<T>::gen_game() const
{
    assert(*this);
    return new T(_grid_gen.gen_board());
}

template <class T>
void strip_db_game_generator<T>::_next(bool init)
{
    assert(init || *this);
    assert(_grid_gen);

    if (!init)
        ++_grid_gen;

    while (_grid_gen && !_current_board_valid())
        ++_grid_gen;
}

template <class T>
bool strip_db_game_generator<T>::_current_board_valid() const
{
    assert(_grid_gen);

    split_result sr;
    bool valid = false;

    /*
        nogo_1xn may or may not throw during construction when the board is
        illegal (depending on whether NOGO_DEBUG is defined).

        If it doesn't throw, is_legal() will still filter the illegal boards

        TODO: Fix this...
    */
    try
    {
        T g(_grid_gen.gen_board());

        if constexpr (std::is_same_v<T, nogo_1xn>)
            if (!g.is_legal())
                return false;

        sr = g.split();

        if (!sr.has_value())
            valid = true;
    }
    catch (std::exception& exc)
    {
        valid = false;
    }

    if (sr.has_value())
        for (game* g : *sr)
            delete g;

    return valid;
}
