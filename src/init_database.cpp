#include "init_database.h"

#include <filesystem>
#include <vector>
#include <string>
#include <iostream>
#include <cassert>

#include "clobber_1xn.h"
#include "clobber.h"
#include "database.h"
#include "db_game_generator.h"
#include "elephants.h"
#include "global_database.h"
#include "global_options.h"
#include "nogo_1xn.h"
#include "nogo.h"
#include "domineering.h"
#include "amazons.h"
#include "gridlike_db_game_generator.h"
#include "grid_generator.h"

namespace {

void register_types(database& db)
{
    /*
        Types used to query the database must be registered, but order doesn't
        matter. The database maps runtime game type IDs to disk game type IDs.
    */

    DATABASE_REGISTER_TYPE(db, clobber_1xn);
    DATABASE_REGISTER_TYPE(db, nogo_1xn);
    DATABASE_REGISTER_TYPE(db, elephants);
    DATABASE_REGISTER_TYPE(db, clobber);
    DATABASE_REGISTER_TYPE(db, nogo);
    DATABASE_REGISTER_TYPE(db, domineering);
    DATABASE_REGISTER_TYPE(db, amazons);
}

void fill_database(database& db)
{
    std::vector<db_game_generator*> generators = {
        //new gridlike_db_game_generator<elephants, grid_generator_default>(15),
        //new gridlike_db_game_generator<clobber_1xn, grid_generator_clobber>(15),
        //new gridlike_db_game_generator<nogo_1xn, grid_generator_nogo>(15),
        //new gridlike_db_game_generator<clobber, grid_generator_clobber>(3, 3),
        //new gridlike_db_game_generator<nogo, grid_generator_nogo>(3, 3),
        //new gridlike_db_game_generator<domineering, grid_generator_domineering>(4, 4),
        new gridlike_db_game_generator<amazons, grid_generator_amazons>(3, 3),
    };

    for (db_game_generator* gen : generators)
    {
        db.generate_entries(*gen);
        delete gen;
    }
}

init_database_enum resolve_auto_init_type(const std::string& filename)
{
    if (std::filesystem::exists(filename))
    {
        std::cout << "Found database file: \"" << filename << "\". Loading..."
                  << std::endl;
        return INIT_DATABASE_LOAD;
    }

    std::cout << "Failed to find database file: \"" << filename
              << "\". Creating..." << std::endl;
    return INIT_DATABASE_CREATE;
}

} // namespace

namespace mcgs_init {
void init_database(const std::string& filename, init_database_enum init_type)
{
    init_global_database();
    database& db = get_global_database();

    assert(db.empty());

    if (init_type == INIT_DATABASE_AUTO)
        init_type = resolve_auto_init_type(filename);

    if (init_type == INIT_DATABASE_LOAD)
    {
        db.load(filename);
        std::cout << "Database file loaded:";
        std::cout << " \"" << filename << "\"" << std::endl;
    }

    register_types(db);

    if (init_type == INIT_DATABASE_CREATE)
    {
        fill_database(db);
        db.save(filename);
        std::cout << "Database file saved:";
        std::cout << " \"" << filename << "\"" << std::endl;
    }

    if (init_type != INIT_DATABASE_NONE && global::print_db_info())
    {
        std::cout << "Database: " << std::endl;
        std::cout << db << std::endl;
    }
}

} // namespace mcgs_init
