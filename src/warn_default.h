/*
    Defines WARN_DEFAULT_IMPL() macro

    Used to print warnings to stderr whenever the DEFAULT_IMPL_DEBUG compilation
    option is defined, and a default implementation is
    called, i.e. in order to find unimplemented non-mandatory functions like
    split(), normalize(), etc
*/
#pragma once

#ifdef DEFAULT_IMPL_DEBUG

// IWYU pragma: begin_exports
#include <unordered_set>
#include <typeinfo>
#include <typeindex>
#include <iostream>
// IWYU pragma: end_exports

#define WARN_DEFAULT_IMPL()                                                    \
    {                                                                          \
        static std::unordered_set<std::type_index> __unimplemented_set;        \
                                                                               \
        std::type_index tidx(typeid(*this));                                   \
                                                                               \
        if (__unimplemented_set.find(tidx) == __unimplemented_set.end())       \
        {                                                                      \
            __unimplemented_set.insert(tidx);                                  \
                                                                               \
            std::cerr << "WARNING: Game type \"" << tidx.name()                \
                      << "\" uses default " << __func__ << "!" << " From "     \
                      << __FILE__ << std::endl;                                \
        }                                                                      \
    }

#else
#define WARN_DEFAULT_IMPL()
#endif
