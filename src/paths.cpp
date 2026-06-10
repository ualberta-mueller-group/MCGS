#include "paths.h"

#include <filesystem>
#include <cassert>

#include "throw_assert.h"

// All relative to project root (except DEFAULT_DB_PATH)
#define DEFAULT_DB_PATH "database.bin"
#define DEFAULT_INPUT_PATH "input/autotests"
#define DEFAULT_CSV_PATH "out.csv"

#define TEST_EXEC_INPUT_PATH "test/input"
#define GAME_UNIT_TEST_INPUT_PATH "test/input/unit_tests"

using namespace std;
using std::filesystem::path;

////////////////////////////////////////////////// Helpers
namespace {
bool is_initialized = false;
path exec_abs_path;
} // namespace

////////////////////////////////////////////////// Exported functions
path get_exec_path()
{
    assert(is_initialized);
    return exec_abs_path;
}

path get_project_root_path()
{
    assert(is_initialized);

    const path root_path = exec_abs_path.parent_path();
    return root_path;
}

path get_default_db_path()
{
    assert(is_initialized);

    const path root_path = get_project_root_path();
    const path db_path = root_path / DEFAULT_DB_PATH;

    return db_path;
}

path get_default_input_path()
{
    assert(is_initialized);

    const path root_path = get_project_root_path();
    const path input_path = root_path / DEFAULT_INPUT_PATH;

    return input_path;
}

path get_default_csv_path()
{
    assert(is_initialized);

    const path root_path = get_project_root_path();
    const path csv_path = root_path / DEFAULT_CSV_PATH;

    return csv_path;
}

path get_test_exec_input_path()
{
    assert(is_initialized);

    const path root_path = get_project_root_path();
    const path test_exec_input_path = root_path / TEST_EXEC_INPUT_PATH;

    return test_exec_input_path;
}

path get_game_unit_test_input_path()
{
    assert(is_initialized);

    const path root_path = get_project_root_path();
    const path game_unit_test_input_path = root_path / GAME_UNIT_TEST_INPUT_PATH;

    return game_unit_test_input_path;
}

path path_relative_to_cwd(const path& p)
{
    const path p_abs = filesystem::weakly_canonical(p);
    const path cwd_abs =
        filesystem::weakly_canonical(filesystem::current_path());

    return filesystem::proximate(p_abs, cwd_abs);
}

namespace mcgs_init {
void init_paths(const char* exec_path)
{
    assert(!is_initialized);

#ifndef __EMSCRIPTEN__
    exec_abs_path = filesystem::canonical(exec_path);
    THROW_ASSERT(filesystem::exists(exec_abs_path));
    THROW_ASSERT(filesystem::is_regular_file(exec_abs_path));
#else
    // Relax constraint that the file must exist
    exec_abs_path = filesystem::weakly_canonical(exec_path);
#endif

    is_initialized = true;
}
} // namespace mcgs_init
