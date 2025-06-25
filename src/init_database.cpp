#include "init_database.h"
#include <iostream>

#include "clobber_1xn.h"
#include "db_game_generator.h"
#include "elephants.h"
#include "global_database.h"
#include "nogo_1xn.h"
#include "strip_db_game_generator.h"

using namespace std;

namespace mcgs_init {

/*
    TODO: saving/loading is useless if game_type_ts change...
*/
void init_database()
{
    std::cout << "Initializing DB" << std::endl;

    init_global_database();

    database& db = get_global_database();
    assert(db.empty());

    std::vector<db_game_generator*> generators =
    {
        new strip_db_game_generator<clobber_1xn>(8),
        new strip_db_game_generator<nogo_1xn>(8),
        new strip_db_game_generator<elephants>(8),
    };

    for (db_game_generator* gen : generators)
    {
        db.generate_entries(*gen);
        delete gen;
    }

    cout << db << endl;

    assert(!db.empty());

    db.save("db.bin");
}

} // namespace mcgs_init
