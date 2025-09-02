/*
    Custom type traits to enforce conditions at compile-time. Also defines
    other compile-time hacks.

    For details on the template specialization technique used, read about
    "SFINAE" (substitution failure is not an error).
*/
#pragma once
#include <type_traits>

// NOLINTBEGIN(readability-identifier-naming)
// clang-format off

////////////////////////////////////////////////// has_is_legal_v<T>
// True IFF T has method: bool is_legal() const
// i.e. nogo, nogo_1xn classes

// Generic template to match all types
template <class T, class Enable = void>
struct has_is_legal
{
    static constexpr bool value = false;
};

// Template specialization matching types implementing the method
template <class T>
struct has_is_legal<
    T,
    std::enable_if_t<
        std::is_same_v<
            bool (T::*)() const,
            decltype(&T::is_legal)
        >,
        void
    >
>
{
    static constexpr bool value = true;
};

template <class T>
static constexpr bool has_is_legal_v = has_is_legal<T>::value;

////////////////////////////////////////////////// deferred_false_v<T>
/*
    With older compilers, putting static_assert(false) in a template struct
    causes it to fire even when the template is not instantiated.

    Instead use static_assert(deferred_false_v<T>) so that the static_assert
    can't be evaluated without the template being instantiated. This works
    because the compiler must account for the fact that a specialization of
    deferred_false could exist for T.

    NOTE: Such static_asserts are still not SFINAE-friendly
*/
template <class>
struct deferred_false: public std::false_type
{
};

template <class T>
static constexpr bool deferred_false_v = deferred_false<T>::value;

// clang-format on
// NOLINTEND(readability-identifier-naming)
