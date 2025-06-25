#include "init_database.h"
#include "clobber_1xn.h"
#include "database.h"
#include "db_game_generator.h"
#include "elephants.h"
#include "global_database.h"
#include "strip_db_game_generator.h"

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
}

void fill_database(database& db)
{
    std::vector<db_game_generator*> generators =
    {
        new strip_db_game_generator<clobber_1xn>(10),
        new strip_db_game_generator<nogo_1xn>(10),
        new strip_db_game_generator<elephants>(10),
    };

    for (db_game_generator* gen : generators)
    {
        db.generate_entries(*gen);
        delete gen;
    }
}

} // namespace

namespace mcgs_init {
void init_database(const std::string& filename, init_database_enum init_type)
{
    init_global_database();
    database& db = get_global_database();

    assert(db.empty());

    if (init_type == INIT_DATABASE_LOAD)
        db.load(filename);

    register_types(db);

    if (init_type == INIT_DATABASE_CREATE)
    {
        fill_database(db);
        db.save(filename);
    }
}

} // namespace mcgs_init
