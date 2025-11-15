/*
   str_to_XYZ() and str_to_XYZ_opt()

   The non-optional version will throw if the string argument is not integral,
   or when it's out of range.

   The optional version will return an empty optional if the argument is not
   integral, but will still throw if it is an out of range integral value.
*/
#pragma once


#include <string>
#include <optional>
#include "utilities.h"

// TODO test all of these
// TODO not sure which of these should be inlined, if any

////////////////////////////////////////////////// short
short str_to_sh(const std::string& str);
std::optional<short> str_to_sh_opt(const std::string& str);

////////////////////////////////////////////////// unsigned short
unsigned short str_to_ush(const std::string& str);
std::optional<unsigned short> str_to_ush_opt(const std::string& str);

////////////////////////////////////////////////// int
inline int str_to_i(const std::string& str)
{
    THROW_ASSERT(is_int(str));
    return std::stoi(str);
}

inline std::optional<int> str_to_i_opt(const std::string& str)
{
    if (!is_int(str))
        return {};

    return std::stoi(str);
}

////////////////////////////////////////////////// long
inline long str_to_l(const std::string& str)
{
    THROW_ASSERT(is_int(str));
    return std::stol(str);
}

inline std::optional<long> str_to_l_opt(const std::string& str)
{
    if (!is_int(str))
        return {};

    return std::stol(str);
}

////////////////////////////////////////////////// unsigned long
inline unsigned long str_to_ul(const std::string& str)
{
    THROW_ASSERT(is_unsigned_int(str));
    return std::stoul(str);
}

inline std::optional<unsigned long> str_to_ul_opt(
    const std::string& str)
{
    if (!is_unsigned_int(str))
        return {};

    return std::stoul(str);
}

////////////////////////////////////////////////// long long
inline long long str_to_ll(const std::string& str)
{
    THROW_ASSERT(is_int(str));
    return std::stoll(str);
}

inline std::optional<long long> str_to_ll_opt(
    const std::string& str)
{
    if (!is_int(str))
        return {};

    return std::stoll(str);
}

////////////////////////////////////////////////// unsigned long long
inline unsigned long long str_to_ull(const std::string& str)
{
    THROW_ASSERT(is_unsigned_int(str));
    return std::stoull(str);
}

inline std::optional<unsigned long long> str_to_ull_opt(
    const std::string& str)
{
    if (!is_unsigned_int(str))
        return {};

    return std::stoull(str);
}

