#pragma once
#include <memory>
#include <type_traits>
#include "game.h"

namespace custom_traits {
// NOLINTBEGIN

//////////////////////////////////////// is_smart_ptr<T>
template <class T>
struct is_smart_ptr
{
    constexpr static bool value = false;
};

template <class T>
struct is_smart_ptr<std::shared_ptr<T>>
{
    constexpr static bool value = true;
};

template <class T>
struct is_smart_ptr<std::unique_ptr<T>>
{
    constexpr static bool value = true;
};

template <class T>
constexpr bool is_smart_ptr_v = is_smart_ptr<T>::value;

//////////////////////////////////////// is_some_ptr<T>
template <class T>
struct is_some_ptr
{
    constexpr static bool value = std::is_pointer_v<T> || is_smart_ptr_v<T>;
};

template <class T>
constexpr bool is_some_ptr_v = is_some_ptr<T>::value;

//////////////////////////////////////// is_some_game_ptr<T>
template <class T, typename Enable = void>
struct is_some_game_ptr
{
    constexpr static bool value = false;
};

// Specialization for pointer types
template <class T>
struct is_some_game_ptr<T, typename std::enable_if<is_some_ptr_v<T>>::type>
{
    using Element = typename std::pointer_traits<T>::element_type;

    constexpr static bool value = std::is_base_of_v<game, Element>;
};

template <class T>
constexpr bool is_some_game_ptr_v = is_some_game_ptr<T>::value;


// NOLINTEND
} // namespace custom_type_traits 
