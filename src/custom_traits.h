#include <type_traits>

// NOLINTBEGIN(readability-identifier-naming)

////////////////////////////////////////////////// has_is_legal_v<T>
// True IFF T has method: bool is_legal() const

template <class T, class Enable = void>
struct has_is_legal
{
    static constexpr bool value = false;
};

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
// NOLINTEND(readability-identifier-naming)
