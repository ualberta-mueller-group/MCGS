/*
    Default settings for AddressSanitizer; some of these may not be enabled
    by default on Mac OS

    extern "C" needed because of C++ name mangling?

    See: https://github.com/google/sanitizers/wiki/AddressSanitizerFlags
*/

// NOLINTNEXTLINE(readability-identifier-naming,misc-use-internal-linkage)
extern "C" const char* __asan_default_options() {
    return "detect_stack_use_after_return=1:detect_leaks=1";
}

