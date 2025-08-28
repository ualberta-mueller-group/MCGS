/*
    Custom type traits to enforce conditions at compile-time.

    For details on the template specialization technique used, read about
    "SFINAE" (substitution failure is not an error).
*/
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

//////////////////////////////////////////////////
// clang-format on
// NOLINTEND(readability-identifier-naming)
