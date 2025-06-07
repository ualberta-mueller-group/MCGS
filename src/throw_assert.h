#pragma once
#include <string>    // IWYU pragma: keep
#include <stdexcept> // IWYU pragma: keep

// NOLINTBEGIN(readability-identifier-naming)
#define _GET_THROW_MESSAGE(cond)                                               \
    (std::string(__FILE__) + ":" + std::to_string(__LINE__) +                  \
     ": Condition `" + std::string(#cond) + "' failed.")

#define _THROW_ASSERT_1(expr)                                                  \
    if (!(expr))                                                               \
    throw std::logic_error(_GET_THROW_MESSAGE(expr))

#define _THROW_ASSERT_2(expr, exception)                                       \
    if (!(expr))                                                               \
    throw exception

#define _THROW_ASSERT_SELECTOR(arg1, arg2, arg3, ...) arg3
// NOLINTEND(readability-identifier-naming)

/*
    Throw in both release and debug builds:
       THROW_ASSERT(condition);
       THROW_ASSERT(condition, exception);

    Throw only in debug builds:
        THROW_ASSERT_DEBUG(condition);
        THROW_ASSERT_DEBUG(condition, exception);

*/
#define THROW_ASSERT(...)                                                      \
    _THROW_ASSERT_SELECTOR(__VA_ARGS__, _THROW_ASSERT_2,                       \
                           _THROW_ASSERT_1)(__VA_ARGS__)

#ifndef NDEBUG
#define THROW_ASSERT_DEBUG(...) THROW_ASSERT(__VA_ARGS__)
#else
#define THROW_ASSERT_DEBUG(...)
#endif
