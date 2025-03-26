#pragma once

#include "strip.h"

#include <cstdint>
#include <functional>

typedef std::function<uint64_t(const strip&)> hash_func_t;

void benchmark_hash_function(hash_func_t& fn, const std::string& label);
