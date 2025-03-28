#pragma once
#include <string>

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
   THROW_ASSERT(condition);
   THROW_ASSERT(condition, exception);
*/
#define THROW_ASSERT(...)                                                      \
    _THROW_ASSERT_SELECTOR(__VA_ARGS__, _THROW_ASSERT_2,                       \
                           _THROW_ASSERT_1)(__VA_ARGS__)
