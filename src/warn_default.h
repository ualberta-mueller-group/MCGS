#pragma once

// IWYU pragma: begin_exports
#include <unordered_map>
#include <unordered_set>
#include <typeinfo>
#include <typeindex>
#include <iostream>
// IWYU pragma: end_exports

#ifndef NO_WARN_DEFAULT_IMPL

#define WARN_DEFAULT_IMPL()                                                   \
{                                                                             \
    static std::unordered_set<std::type_index> unimplemented_set;             \
                                                                              \
    std::type_index tidx(typeid(*this));                                      \
                                                                              \
    if (unimplemented_set.find(tidx) == unimplemented_set.end())              \
    {                                                                         \
        unimplemented_set.insert(tidx);                                       \
                                                                              \
        std::cerr << "WARNING: Game type \"" << tidx.name() << "\" uses "     \
            "default " << __func__ << "!" << std::endl;                       \
    }                                                                         \
}

#else
#define WARN_DEFAULT_IMPL()
#endif


