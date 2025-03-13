#pragma once

// NOLINTBEGIN
#define _GET_THROW_MESSAGE(cond) (std::string(__FILE__) + ":" + std::to_string(__LINE__) + ": Condition `" + std::string(#cond) + "' failed.")

#define _THROW_IF_NOT_1(expr) if (!(expr)) throw std::logic_error(_GET_THROW_MESSAGE(expr))
#define _THROW_IF_NOT_2(expr, exception) if (!(expr)) throw exception

#define _THROW_IF_NOT_SELECTOR(arg1, arg2, arg3, ...) arg3
// NOLINTEND

/*
   THROW_IF_NOT(condition);
   THROW_IF_NOT(condition, exception);
*/
#define THROW_IF_NOT(...) _THROW_IF_NOT_SELECTOR(__VA_ARGS__, _THROW_IF_NOT_2, _THROW_IF_NOT_1) (__VA_ARGS__)

