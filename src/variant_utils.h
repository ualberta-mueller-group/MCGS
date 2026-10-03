#pragma once

#include <variant>
#include "custom_traits.h"

/*
    Check whether a pointer variant is nullptr. Example argument type:
        `std::variant<std::unique_ptr<T1>, std::shared_ptr<T2>, T3*>`
*/
template <class... Ts>
inline bool ptr_variant_non_null(const std::variant<Ts...>& ptr_variant)
{
    // Fold expression checks that each variant alternative is some sort of
    // pointer
    static_assert((is_smart_or_raw_pointer_v<Ts> && ...));

    return std::visit([](const auto& ptr) -> bool
    {
        return static_cast<bool>(ptr);
    }, ptr_variant);
}

template <class... Ts>
inline bool ptr_variant_null(const std::variant<Ts...>& ptr_variant)
{
    // Fold expression checks that each variant alternative is some sort of
    // pointer
    static_assert((is_smart_or_raw_pointer_v<Ts> && ...));

    return std::visit([](const auto& ptr) -> bool
    {
        return !static_cast<bool>(ptr);
    }, ptr_variant);
}

