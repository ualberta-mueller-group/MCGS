#pragma once
#include "strip.h"
#include <cstdint>
#include <functional>


#define __CHECK_COLLISIONS 1
typedef uint64_t hash_t;


typedef std::function<hash_t(const strip&)> hash_func_t;

void __benchmark_hash_function(hash_func_t& fn, const std::string& label); // NOLINT

#define benchmark_hash_function(fn) __benchmark_hash_function(fn, std::string(__FILE__)) // NOLINT
