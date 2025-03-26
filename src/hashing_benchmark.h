#pragma once

#include "strip.h"

#include <cstdint>
#include <functional>

typedef std::function<uint64_t(const strip&)> hash_func_t;

void __benchmark_hash_function(hash_func_t& fn, const std::string& label); // NOLINT

#define benchmark_hash_function(fn) __benchmark_hash_function(fn, std::string(__FILE__)) // NOLINT
