#pragma once

#include <exception>
#include <type_traits>
#include <memory>

#include "grid.h"
#include "strip.h"
#include "db_game_generator.h"
#include "grid_generator.h"
#include "custom_traits.h"
#include "throw_assert.h"
#include "utilities.h"


//////////////////////////////////////////////////
// class gridlike_db_game_generator<Game_T>

enum gridlike_type_enum
{
    GRIDLIKE_TYPE_GRID = 0,
    GRIDLIKE_TYPE_STRIP,
};

template <class Game_T, gridlike_type_enum gridlike_type>
class gridlike_db_game_generator : public i_db_game_generator
{
public:
    static_assert(std::is_base_of_v<game, Game_T> &&
                  !std::is_abstract_v<Game_T>);

    // If grid, the game must be constructible from a board and its dimensions
    static_assert(
        LOGICAL_IMPLIES(
            gridlike_type == GRIDLIKE_TYPE_GRID, //
            (std::is_constructible_v<Game_T, const std::vector<int>&,
                                     int_pair>) ), //
        "Game type isn't constructible like a grid game (from a board and "
        "dimensions)");

    // If strip, the game must be constructible from a board
    static_assert(
        LOGICAL_IMPLIES(
            gridlike_type == GRIDLIKE_TYPE_STRIP,                         //
            (std::is_constructible_v<Game_T, const std::vector<int>&>) ), //
        "Game type isn't constructible like a strip game (from a board) ");

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

template <class Game_T, gridlike_type_enum gridlike_type>
gridlike_db_game_generator<Game_T, gridlike_type>::gridlike_db_game_generator(
    i_grid_generator* grid_gen)
    : _grid_gen(grid_gen)
{
    assert(_grid_gen.get() != nullptr);

    // TODO proper error message
    if constexpr (gridlike_type == GRIDLIKE_TYPE_STRIP)
        THROW_ASSERT(_grid_gen->only_strips());

    _increment(true);
}

template <class Game_T, gridlike_type_enum gridlike_type>
inline gridlike_db_game_generator<Game_T, gridlike_type>::operator bool() const
{
    return (_grid_gen.get() != nullptr) && (*_grid_gen);
}

template <class Game_T, gridlike_type_enum gridlike_type>
inline void gridlike_db_game_generator<Game_T, gridlike_type>::operator++()
{
    assert(*this);
    _increment(false);
}

template <class Game_T, gridlike_type_enum gridlike_type>
inline game* gridlike_db_game_generator<Game_T, gridlike_type>::gen_game() const
{
    assert(*this);

    if constexpr (gridlike_type == GRIDLIKE_TYPE_GRID)
        // grids
        return new Game_T(_grid_gen->gen_board(), _grid_gen->get_shape());
    else
        // strips
        return new Game_T(_grid_gen->gen_board());
}

template <class Game_T, gridlike_type_enum gridlike_type>
void gridlike_db_game_generator<Game_T, gridlike_type>::_increment(bool init)
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
        if constexpr (gridlike_type == GRIDLIKE_TYPE_STRIP)
            found = _strip_game_is_legal(); // strips
        else
            found = _grid_game_is_legal(); // grids

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

template <class Game_T, gridlike_type_enum gridlike_type>
bool gridlike_db_game_generator<Game_T, gridlike_type>::_strip_game_is_legal() const
{
    static_assert(gridlike_type == GRIDLIKE_TYPE_STRIP);

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

template <class Game_T, gridlike_type_enum gridlike_type>
bool gridlike_db_game_generator<Game_T, gridlike_type>::_grid_game_is_legal() const
{
    static_assert(gridlike_type == GRIDLIKE_TYPE_GRID);
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

