#include "init_database.h"
#include "clobber_1xn.h"
#include "clobber.h"
#include "database.h"
#include "db_game_generator.h"
#include "elephants.h"
#include "global_database.h"
#include "nogo_1xn.h"
#include "nogo.h"
#include "gridlike_db_game_generator.h"
#include <filesystem>


#include "grid_generator_new.h"
#define DATABASE_REGISTER_TYPE(db, game_class_name) \
db.register_type(#game_class_name, game_type<game_class_name>())

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
}

/*
    elephants

    clobber_1xn
    clobber

    nogo_1xn
    nogo


*/

void fill_database(database& db)
{
    // Times are for DEBUG=0 build
    std::vector<db_game_generator*> generators =
    {
        // (handle subgames, skip sums)
        //new gridlike_db_game_generator<elephants>(16), // (0:55, 0:46)
        //new gridlike_game_generator<elephants, ggen_default>(16), // (0:56, 0:44)
        //new gridlike_game_generator<elephants, ggen_clobber>(16), // (1:11, 0:56)
        //new gridlike_game_generator<elephants, ggen_nogo>(16), // (1:02, 0:50)

        // Can't test 2nd case because nogo always splits...
        //new gridlike_db_game_generator<nogo>(4, 3), // (0:35)
        //new gridlike_game_generator<nogo, ggen_default>(4, 3), // (0:32)
        //new gridlike_game_generator<nogo, ggen_clobber>(4, 3), // (0:32)
        //new gridlike_game_generator<nogo, ggen_nogo>(4, 3), // (0:35)


        //new gridlike_db_game_generator<clobber>(4, 3), // (0:19, 0:23)
        //new gridlike_game_generator<clobber, ggen_default>(4, 3), // (0:21, 0:20)
        //new gridlike_game_generator<clobber, ggen_clobber>(4, 3), // (0:22, 0:21)
        //new gridlike_game_generator<clobber, ggen_nogo>(4, 3), // (0:20, 0:21)


        // skip sum case
        //new gridlike_game_generator<clobber, ggen_clobber>(2, 7), // 1:36
        //new gridlike_game_generator<clobber, ggen_nogo>(2, 7), // 1:40

        // Old total (handle subgames): 0:45.0, 0:43.1
        //new gridlike_db_game_generator<elephants>(15),
        //new gridlike_db_game_generator<clobber_1xn>(15),
        //new gridlike_db_game_generator<nogo_1xn>(15),
        //new gridlike_db_game_generator<clobber>(3, 3),
        //new gridlike_db_game_generator<nogo>(3, 3),

        // New total (handle subgames): 0:46.5, 0:46.5
        new gridlike_game_generator<elephants, ggen_default>(15),
        new gridlike_game_generator<clobber_1xn, ggen_clobber>(15),
        new gridlike_game_generator<nogo_1xn, ggen_nogo>(15),
        new gridlike_game_generator<clobber, ggen_clobber>(3, 3),
        new gridlike_game_generator<nogo, ggen_nogo>(3, 3),

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
        std::cout << "Found database file: \"" << filename << "\". Loading..." << std::endl;
        return INIT_DATABASE_LOAD;
    }

    std::cout << "Failed to find database file: \"" << filename << "\". Creating..." << std::endl;
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
        std::cout << "Database file loaded" << std::endl;
    }

    register_types(db);

    if (init_type == INIT_DATABASE_CREATE)
    {
        fill_database(db);
        db.save(filename);
        std::cout << "Database file saved" << std::endl;
    }

    std::cout << "Database: " << std::endl;
    std::cout << db << std::endl;
}

} // namespace mcgs_init
