#pragma once

// TODO IS_EMSCRIPTEN macro that will complain if mis-typed

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/bind.h>
#endif
