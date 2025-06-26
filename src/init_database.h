#pragma once
#include <string>

enum init_database_enum
{
    INIT_DATABASE_NONE = 0,
    INIT_DATABASE_AUTO,
    INIT_DATABASE_CREATE,
    INIT_DATABASE_LOAD
};

namespace mcgs_init {
void init_database(const std::string& filename, init_database_enum init_type);
} // namespace mcgs_init
