#pragma once

#include <exception>
#include <type_traits>
#include <unordered_map>
#include <memory>

#include "grid.h"
#include "strip.h"
#include "db_game_generator.h"
#include "grid_generator.h"
#include "custom_traits.h"
#include "throw_assert.h"


//////////////////////////////////////////////////
// class gridlike_db_game_generator<Game_T>

template <class Game_T>
class gridlike_db_game_generator : public i_db_game_generator
{
public:
    static_assert(std::is_base_of_v<grid, Game_T> ||
                  std::is_base_of_v<strip, Game_T>);

    virtual ~gridlike_db_game_generator() {}

    gridlike_db_game_generator(i_grid_generator* grid_gen);

    operator bool() const override;
    void operator++() override;
    game* gen_game() const override;

protected:
    void _increment(bool init);

    bool _strip_game_is_legal() const;
    bool _grid_game_is_legal() const;

    std::unique_ptr<i_grid_generator> _grid_gen;
};

//////////////////////////////////////////////////
// gridlike_db_game_generator<Game_T> methods

template <class Game_T>
gridlike_db_game_generator<Game_T>::gridlike_db_game_generator(
    i_grid_generator* grid_gen)
    : _grid_gen(grid_gen)
{
    assert(_grid_gen.get() != nullptr);

    // TODO proper error message
    if constexpr (std::is_base_of_v<strip, Game_T>)
        THROW_ASSERT(_grid_gen->only_strips());

    _increment(true);
}

template <class Game_T>
inline gridlike_db_game_generator<Game_T>::operator bool() const
{
    return (_grid_gen.get() != nullptr) && (*_grid_gen);
}

template <class Game_T>
inline void gridlike_db_game_generator<Game_T>::operator++()
{
    assert(*this);
    _increment(false);
}

template <class Game_T>
inline game* gridlike_db_game_generator<Game_T>::gen_game() const
{
    assert(*this);

    if constexpr (std::is_base_of_v<grid, Game_T>)
        return new Game_T(_grid_gen->gen_board(), _grid_gen->get_shape());
    else
        return new Game_T(_grid_gen->gen_board());
}

template <class Game_T>
void gridlike_db_game_generator<Game_T>::_increment(bool init)
{
    assert(init || *this);

    if (!init)
    {
        assert(*_grid_gen);
        ++(*_grid_gen);
    }

    bool found = false;
    while (*_grid_gen && !found)
    {
        if constexpr (std::is_base_of_v<strip, Game_T>)
            found = _strip_game_is_legal();
        else
            found = _grid_game_is_legal();

        if (found)
            break;

        ++(*_grid_gen);
    }

    if (!found)
    {
        assert(!*_grid_gen);
        _grid_gen.reset();
        assert(!*this);
    }
}

template <class Game_T>
bool gridlike_db_game_generator<Game_T>::_strip_game_is_legal() const
{
    static_assert(std::is_base_of_v<strip, Game_T>);

    assert(*_grid_gen &&                     //
           _grid_gen->get_shape().first <= 1 //
    );

    bool found = false;

    try
    {
        Game_T g(_grid_gen->gen_board());

        if constexpr (has_is_legal_v<Game_T>)
            found = g.is_legal();
        else
            found = true;
    }
    catch (const std::exception& exc)
    {
        found = false;
    }

    return found;
}

template <class Game_T>
bool gridlike_db_game_generator<Game_T>::_grid_game_is_legal() const
{
    static_assert(std::is_base_of_v<grid, Game_T>);
    assert(*_grid_gen);

    bool found = false;

    try
    {
        Game_T g(_grid_gen->gen_board(), _grid_gen->get_shape());

        if constexpr (has_is_legal_v<Game_T>)
            found = g.is_legal();
        else
            found = true;
    }
    catch (const std::exception& exc)
    {
        found = false;
    }

    return found;
}

