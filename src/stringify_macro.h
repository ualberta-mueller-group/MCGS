/*
    i.e. STRINGIFY(__LINE__)
*/
#pragma once

// NOLINTNEXTLINE(readability-identifier-naming)
#define _STRINGIFY_IMPL(arg) #arg
#define STRINGIFY(arg) _STRINGIFY_IMPL(arg)
