/*
   Defines assert-like macro which throws (by default a std::logic_error)

    Throw in both release and debug builds:
       THROW_ASSERT(condition);
       THROW_ASSERT(condition, exception);

    Throw only in debug builds (useful for unit tests as they can catch):
        THROW_ASSERT_DEBUG(condition);
        THROW_ASSERT_DEBUG(condition, exception);


    If no exception is passed to the macro, a std::logic_error is thrown, with
    assert-like information. Example:

    terminate called after throwing an instance of 'std::logic_error'
    what():  src/main/main.cpp:24: Condition `1 + 1 == 3' failed
*/
#pragma once
#include <string>    // IWYU pragma: keep
#include <stdexcept> // IWYU pragma: keep

// NOLINTBEGIN(readability-identifier-naming)

// Generate assert-like exception message
#define _GET_THROW_MESSAGE(cond)                                               \
    (std::string(__FILE__) + ":" + std::to_string(__LINE__) +                  \
     ": Condition `" + std::string(#cond) + "' failed.")

// 1 arg implementation
#define _THROW_ASSERT_1(expr)                                                  \
    if (!(expr))                                                               \
    throw std::logic_error(_GET_THROW_MESSAGE(expr))

// 2 arg implementation
#define _THROW_ASSERT_2(expr, _exception)                                      \
    if (!(expr))                                                               \
    {                                                                          \
        const auto& _exc = _exception;                                         \
        if constexpr (std::is_base_of_v<std::exception, decltype(_exc)>)       \
            throw _exc;                                                        \
        else                                                                   \
            throw std::logic_error(_exc);                                      \
    }                                                                          \
    static_assert(true)

// Resolves to correct implementation macro based on number of args
#define _THROW_ASSERT_SELECTOR(arg1, arg2, arg3, ...) arg3

// NOLINTEND(readability-identifier-naming)

// Top level macro called by consumer of this file.
#define THROW_ASSERT(...)                                                      \
    _THROW_ASSERT_SELECTOR(__VA_ARGS__, _THROW_ASSERT_2,                       \
                           _THROW_ASSERT_1)(__VA_ARGS__)

#ifndef NDEBUG
#define THROW_ASSERT_DEBUG(...) THROW_ASSERT(__VA_ARGS__)
#else
#define THROW_ASSERT_DEBUG(...)
#endif
