#pragma once
#include <exception>
#include <type_traits>
#include "db_game_generator.h"
#include "grid_utils.h"
#include "nogo_1xn.h"
#include "nogo.h"
#include "strip.h"

////////////////////////////////////////////////// class gridlike_db_game_generator<T>
template <class T>
class gridlike_db_game_generator: public db_game_generator
{
    static_assert(std::is_base_of_v<strip, T> || std::is_base_of_v<grid, T>);
    static_assert(!std::is_abstract_v<T>);

public:
    gridlike_db_game_generator(int strip_size);
    gridlike_db_game_generator(int n_rows, int n_cols);

    operator bool() const override;
    void operator++() override;
    game* gen_game() const override;
    int_pair get_shape() const;

private:
    void _next(bool init);
    bool _current_board_valid() const;
    bool _game_is_legal(const T* g) const;

    grid_generator _grid_gen;
};

////////////////////////////////////////////////// gridlike_db_game_generator methods
/*
   TODO this is inefficient -- shouldn't have to create game objects,
   especially not with operator new...

   Maybe it's OK?
*/

template <class T>
inline gridlike_db_game_generator<T>::gridlike_db_game_generator(int strip_size)
    : _grid_gen(strip_size)
{
    assert(strip_size >= 0);
    _next(true);
}

template <class T>
inline gridlike_db_game_generator<T>::gridlike_db_game_generator(int n_rows, int n_cols)
    : _grid_gen(n_rows, n_cols)
{
    static_assert(std::is_base_of_v<grid, T>, "This function is only for grids");
    assert(n_rows >= 0 && n_cols >= 0);
    _next(true);
}


template <class T>
inline gridlike_db_game_generator<T>::operator bool() const
{
    return _grid_gen;
}

template <class T>
inline void gridlike_db_game_generator<T>::operator++()
{
    assert(*this);
    _next(false);
}

template <class T>
inline game* gridlike_db_game_generator<T>::gen_game() const
{
    assert(*this);
    return new T(_grid_gen.gen_board());
}

template <class T>
inline int_pair gridlike_db_game_generator<T>::get_shape() const
{
    assert(*this);
    return _grid_gen.get_shape();
}

template <class T>
void gridlike_db_game_generator<T>::_next(bool init)
{
    assert(init || *this);
    assert(_grid_gen);

    if (!init)
        ++_grid_gen;

    while (_grid_gen && !_current_board_valid())
        ++_grid_gen;
}

template <class T>
bool gridlike_db_game_generator<T>::_current_board_valid() const
{
    assert(_grid_gen);

    /*
        nogo_1xn may or may not throw during construction when the board is
        illegal (depending on whether NOGO_DEBUG is defined).

        If it doesn't throw, is_legal() will still filter the illegal boards

        TODO: Fix this...
    */
    try
    {
        // Reject illegal boards
        T g(_grid_gen.gen_board());

        if (!_game_is_legal(&g))
            return false;
    }
    catch (std::exception& exc)
    {
        return false;
    }

    return true;
}


template <class T>
inline bool gridlike_db_game_generator<T>::_game_is_legal(const T* g) const
{
    if constexpr (std::is_same_v<T, nogo_1xn> || std::is_same_v<T, nogo>)
        return g->is_legal();

    return true;
}
