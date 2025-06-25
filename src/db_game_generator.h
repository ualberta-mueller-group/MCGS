#pragma once
#include "game.h"
#include <type_traits>

////////////////////////////////////////////////// class db_game_generator
class db_game_generator
{
public:
    virtual ~db_game_generator() {}

    virtual operator bool() const = 0;
    virtual void operator++() = 0;
    virtual game* gen_game() const = 0;
};

////////////////////////////////////////////////// type traits
// NOLINTBEGIN(readability-identifier-naming)

//////////////////////////////////////// has_create_db_game_generator
template <class T, class Enable = void>
struct has_create_db_game_generator
{
    static constexpr bool value = false;
};

template <class T>
struct has_create_db_game_generator<T,
    std::enable_if_t<
        std::is_same_v<
            decltype(&T::create_db_game_generator),
            db_game_generator* (*)()
        >,
        void
    >
>
{
    static constexpr bool value = true;
};

template <class T>
static constexpr bool has_create_db_game_generator_v = has_create_db_game_generator<T>::value;
// NOLINTEND(readability-identifier-naming)
