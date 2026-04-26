# TODO emscripten flags
# TODO use clang instead of ld for linking (???)

cmake_minimum_required(VERSION 4.2)
project(mcgs-cmake)

# Generate a compile_commands.json in the "build" directory
set(CMAKE_EXPORT_COMPILE_COMMANDS True)

# Put executables in the root of the "build" directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

############################## Configurable options
set(DEBUG "1" CACHE STRING
"Debug level (0/[1]/2). 0 disables asserts and removes debug info from \
executables. 1 includes debug info, and (reasonably fast) asserts. 2 \
implies 1 and also includes more expensive asserts. NOTE: MCGS_test is always \
built as if DEBUG=2 was set."
)

set(DEBUG_MESSAGES "0" CACHE STRING
"Debug messages ([0]/1). If 1, print extra/annoying debugging info."
)

set(ASAN "0" CACHE STRING
"Compile with AddressSanitizer? ([0]/1). \
If 1, compiles and links with AddressSanitizer to find memory errors \
at runtime. See `src/asan_options.cpp` for enabled checks."
)

set(ONLY_EMIT "" CACHE STRING
"Values: ([Undefined],\"MCGS\",\"MCGS_test\"). Only emit the specified \
executable. Used when running `run-clang-tidy`, to prevent \
`compile_commands.json` from storing flags for several executables."
)

set(LINT_FILES "src/*.cpp;src/*.h;test/*.cpp;test/*.h" CACHE STRING
"String specifying a semicolon-separated list of (recursive) CMake file globs. \
Example: \"src/*.cpp;src/*.h\" (this will also include src/main/main.cpp \
because '*' is recursive). If not set, defaults to all source files."
)

set(SCRIPTS_DIR "./utils" CACHE STRING
"Relative path to `utils` directory, from the project root."
)
cmake_path(RELATIVE_PATH SCRIPTS_DIR BASE_DIRECTORY CMAKE_CURRENT_SOURCE_DIR)

############################## Validate options
if (NOT
    (
        (DEBUG GREATER_EQUAL 0) AND
        (DEBUG LESS_EQUAL 2)
    )
)
    message(FATAL_ERROR "Bad value for DEBUG: \"${DEBUG}\"")
endif()

if (NOT
    (
        (DEBUG_MESSAGES GREATER_EQUAL 0) AND
        (DEBUG_MESSAGES LESS_EQUAL 1)
    )
)
    message(FATAL_ERROR "Bad value for DEBUG_MESSAGES: \"${DEBUG_MESSAGES}\"")
endif()

if (NOT 
    (
        (ASAN GREATER_EQUAL 0) AND
        (ASAN LESS_EQUAL 1)
    )
)
    message(FATAL_ERROR "Bad value for ASAN: \"${ASAN}\"")
endif()

if (NOT 
    (
        (ONLY_EMIT STREQUAL "") OR
        (ONLY_EMIT STREQUAL "MCGS") OR
        (ONLY_EMIT STREQUAL "MCGS_test")
    )
)
    message(FATAL_ERROR "Bad value for ONLY_EMIT: \"${ONLY_EMIT}\"")
endif()

# Unset ONLY_EMIT if it's empty
if ("${ONLY_EMIT}" STREQUAL "")
    unset(CACHE{ONLY_EMIT})
endif()

############################## Threading library
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

############################## Python interpreter
find_package(Python3 OPTIONAL COMPONENTS Interpreter)

############################## Common flags
# Flags used by all executables

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(mcgs_flags_common INTERFACE)

if (MSVC)
    target_compile_options(mcgs_flags_common INTERFACE
        /Wall
        /O2
        /Ot
    )

else()
    target_compile_options(mcgs_flags_common INTERFACE
        -Wall
        -O3
        -march=native
        -mtune=native
    )

endif()

############################## Flag groups 
# Chosen by DEBUG/DEBUG_MESSAGES variables, and executable

#################### Flags: none
add_library(mcgs_flags_none INTERFACE)

target_compile_definitions(mcgs_flags_none INTERFACE
    -DNDEBUG
)

#################### Flags: basic
add_library(mcgs_flags_basic INTERFACE)

target_compile_definitions(mcgs_flags_basic INTERFACE 
    -DASSERT_RESTORE_DEBUG
    -DGAME_UNDO_DEBUG
)

if(MSVC)
    target_compile_options(mcgs_flags_basic INTERFACE
        /Z7
    )

else()
    target_compile_options(mcgs_flags_basic INTERFACE
        -g
    )

endif()

#################### Flags: extra
# Implies "basic"

add_library(mcgs_flags_extra INTERFACE)
target_link_libraries(mcgs_flags_extra INTERFACE mcgs_flags_basic)

target_compile_definitions(mcgs_flags_extra INTERFACE 
    -DSUMGAME_DEBUG
    -DTYPE_TABLE_DEBUG
    -DNOGO_DEBUG
    -D_GLIBCXX_DEBUG
)

#################### Flags: debug_messages
add_library(mcgs_flags_debug_messages INTERFACE)

target_compile_definitions(mcgs_flags_debug_messages INTERFACE 
    -DDEFAULT_IMPL_DEBUG
)

#################### ASAN=1 flags
add_library(asan_flags INTERFACE)

if (MSVC)
    target_compile_options(asan_flags INTERFACE
        /Z7
        /fsanitize=address
        /fno-omit-frame-pointer
    )

    target_link_options(asan_flags INTERFACE
        /fsanitize=address
        /fno-omit-frame-pointer
    )

else()
    target_compile_options(asan_flags INTERFACE
        -g
        -fsanitize=address
        -fno-omit-frame-pointer
    )

    target_link_options(asan_flags INTERFACE
        -fsanitize=address
        -fno-omit-frame-pointer
    )

endif()

############################## Compute flags for MCGS and MCGS_test executables
#################### MCGS executable flags
add_library(mcgs_flags INTERFACE)
target_link_libraries(mcgs_flags INTERFACE
    mcgs_flags_common
    Threads::Threads

    $<$<STREQUAL:${ASAN},1>:asan_flags>

    $<$<STREQUAL:${DEBUG_MESSAGES},1>:mcgs_flags_debug_messages>

    $<$<STREQUAL:${DEBUG},0>:mcgs_flags_none>
    $<$<STREQUAL:${DEBUG},1>:mcgs_flags_basic>
    $<$<STREQUAL:${DEBUG},2>:mcgs_flags_extra>
)

target_include_directories(mcgs_flags INTERFACE
    src
)

#################### MCGS_test executable flags
add_library(mcgs_test_flags INTERFACE)
target_link_libraries(mcgs_test_flags INTERFACE
    mcgs_flags_common
    Threads::Threads

    $<$<STREQUAL:${ASAN},1>:asan_flags>

    $<$<STREQUAL:${DEBUG_MESSAGES},1>:mcgs_flags_debug_messages>

    $<$<STREQUAL:${DEBUG},0>:mcgs_flags_extra>
    $<$<STREQUAL:${DEBUG},1>:mcgs_flags_extra>
    $<$<STREQUAL:${DEBUG},2>:mcgs_flags_extra>
)

target_include_directories(mcgs_test_flags INTERFACE
    src
    test
)

############################## Create executables and other targets
#################### MCGS executable
if ((NOT DEFINED ONLY_EMIT) OR ("${ONLY_EMIT}" STREQUAL "MCGS"))
    file(GLOB mcgs_SRC CONFIGURE_DEPENDS src/*.cpp src/main/*.cpp)
    add_executable(MCGS ${mcgs_SRC})
    target_link_libraries(MCGS PRIVATE mcgs_flags)
endif()

#################### MCGS_test executable, and dependent scripts
if ((NOT DEFINED ONLY_EMIT) OR ("${ONLY_EMIT}" STREQUAL "MCGS_test"))
    file(GLOB mcgs_test_SRC CONFIGURE_DEPENDS test/*.cpp src/*.cpp)
    add_executable(MCGS_test EXCLUDE_FROM_ALL ${mcgs_test_SRC})
    target_link_libraries(MCGS_test PRIVATE mcgs_test_flags)

    add_custom_target(test
        COMMAND $<TARGET_FILE:MCGS_test>
        DEPENDS MCGS_test
        VERBATIM
        USES_TERMINAL
    )

    add_custom_target(test_extra
        COMMAND $<TARGET_FILE:MCGS_test> --extra-tests
        DEPENDS MCGS_test
        VERBATIM
        USES_TERMINAL
    )
endif()

#################### Format targets
if (Python3_Interpreter_FOUND)
    set(CLANG_FORMAT_PY ${SCRIPTS_DIR}/format-files.py)
    file(GLOB_RECURSE _LINT_FILES ${LINT_FILES})

    add_custom_target(format
        COMMAND ${Python3_EXECUTABLE} ${CLANG_FORMAT_PY} ${_LINT_FILES}
        USES_TERMINAL
    )

    add_custom_target(format_delete
        COMMAND ${Python3_EXECUTABLE} ${CLANG_FORMAT_PY} --delete ${_LINT_FILES}
        USES_TERMINAL
    )

    add_custom_target(format_replace
        COMMAND ${Python3_EXECUTABLE} ${CLANG_FORMAT_PY} --replace ${_LINT_FILES}
        USES_TERMINAL
    )
else()
    message(WARNING "Couldn't find Python3 interpreter. `format`-like targets \
won't be available...")
endif()
