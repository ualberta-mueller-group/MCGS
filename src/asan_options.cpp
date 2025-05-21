/*
    Settings for AddressSanitizer; some of these may not be enabled
    by default, especially on Mac OS

    The function defined by this file is used by external tooling, when
    compiling with the -fsanitize flag.

    See: https://github.com/google/sanitizers/wiki/AddressSanitizerFlags

    extern "C" needed because of C++ name mangling?
*/

// NOLINTNEXTLINE(readability-identifier-naming,misc-use-internal-linkage)
extern "C" const char* __asan_default_options()
{
    // string literals automatically concatenated

    return "detect_stack_use_after_return=1"

           ":detect_leaks=1"

           // Catch global variable initialization order problems
           ":check_initialization_order=1"
           ":strict_init_order=1";
}
