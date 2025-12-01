#pragma once
#include <string>
#include <vector>
#include <utility>

enum init_database_enum
{
    INIT_DATABASE_NONE = 0,
    INIT_DATABASE_AUTO,
    INIT_DATABASE_CREATE,
    INIT_DATABASE_LOAD,
};

namespace mcgs_init {
void init_database(const std::string& filename, init_database_enum init_type,
                   const std::string& db_config_string);

/*
    Given the DB config string, return a vector of pairs where each first
    element is the game name, and each second element is that game's config
    string.


    i.e. the input:

    "[clobber] max_dims = 3,3; some_other_property = something; [nogo]
    max_dims = 6;"

    produces the output:

    "clobber", " max_dims = 3,3; some_other_property = something; "
    "nogo", "\nmax_dims = 6;"
*/
std::vector<std::pair<std::string, std::string>>
split_db_config_string_by_game_name(const std::string& db_config_string);

} // namespace mcgs_init
