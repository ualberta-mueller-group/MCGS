/*
    Utilities to find paths of project files, i.e. default `autotests`,
    `database.bin`, `out.csv`, .test files for unit tests, etc.

    Returned paths are all absolute, except for the value of
    `path_relative_to_cwd(...)`, which returns a relative path.
*/
#pragma once
#include <filesystem>

// Current executable
std::filesystem::path get_exec_path();

// Repository root
std::filesystem::path get_project_root_path();

// Default database.bin
std::filesystem::path get_default_db_path();

// Default `input/autotests` directory
std::filesystem::path get_default_input_path();

// Default out.csv path
std::filesystem::path get_default_csv_path();

// Main input directory for MCGS_test
std::filesystem::path get_test_exec_input_path();

// Game-specific input directory for MCGS_test
std::filesystem::path get_game_unit_test_input_path();

// Make p absolute, then get its path relative to the current working directory
std::filesystem::path path_relative_to_cwd(const std::filesystem::path& p);

namespace mcgs_init {
void init_paths(const char* exec_path); // Call with argv[0]
} // namespace mcgs_init

