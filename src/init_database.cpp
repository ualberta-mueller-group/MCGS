#include "init_database.h"

#include <cctype>
#include <filesystem>
#include <functional>
#include <type_traits>
#include <unordered_set>
#include <vector>
#include <string>
#include <iostream>
#include <cassert>

#include "config_map.h"
#include "database.h"
#include "db_game_generator.h"
#include "global_database.h"
#include "global_options.h"

// Registered games
#include "clobber_1xn.h"
#include "grid_generator.h"
#include "gridlike_db_game_generator.h"
#include "nogo_1xn.h"
#include "elephants.h"

#include "clobber.h"
#include "nogo.h"
#include "domineering.h"
#include "amazons.h"
#include "fission.h"
#include "sheep_grid_generator.h"
#include "throw_assert.h"
#include "toppling_dominoes.h"
#include "sheep.h"

#include "create_game_gen_fn.h"

using namespace std;

namespace {

i_db_game_generator* create_sheep_gen(const config_map& config)
{
    const optional<int_pair> max_dims = config.get_dims("max_dims");
    const optional<vector<int>> max_sheep = config.get_int_vec("max_sheep");

    THROW_ASSERT(max_dims.has_value() &&     //
                     max_dims->first >= 0 && //
                     max_dims->second >= 0,  //
                 "Unspecified/invalid config value for max_dims.");

    THROW_ASSERT(max_sheep.has_value() &&      //
                     max_sheep->size() == 2 && //
                     (*max_sheep)[0] >= 0 &&   //
                     (*max_sheep)[1] >= 0,     //
                 "Unspecified/invalid config value for max_sheep.");

    i_grid_generator* gg = new sheep_grid_generator(
        max_dims.value(), max_sheep.value()[0], max_sheep.value()[1]);

    return new gridlike_db_game_generator<sheep>(gg);
}

} // namespace

namespace {
unordered_map<string, create_game_gen_fn_t> create_gen_funcs;

void register_games(database& db);


void register_create_game_gen_fn(const string& name, create_game_gen_fn_t& fn)
{
    THROW_ASSERT(
        name.size() > 0,
        "Attempted to register create_game_gen_fn_t for game with blank name!");

    auto inserted = create_gen_funcs.emplace(name, fn);

    THROW_ASSERT(inserted.second,
                 "create_game_gen_fn_t registered twice for game \"" + name +
                     "\"!");
}

void register_create_game_gen_fn(const string& name, create_game_gen_fn_t&& fn)
{
    register_create_game_gen_fn(name, fn);
}

void fill_database(database& db, const string& db_config_string, bool dry_run)
{
    vector<pair<string, string>> config_pairs =
        mcgs_init::split_db_config_string_by_game_name(db_config_string);

    // Validate config pairs
    {
        unordered_set<string> game_names;

        for (const pair<string, string>& config_pair : config_pairs)
        {
            const string& game_name = config_pair.first;
            const string& game_config = config_pair.second;

            auto create_fn_it = create_gen_funcs.find(game_name);

            THROW_ASSERT(
                create_fn_it != create_gen_funcs.end(),
                "Error: DB config references game \"" + game_name +
                    "\" which has no registered create_game_gen_fn_t!");

            THROW_ASSERT(game_names.insert(game_name).second,
                         "Error: DB config references game \"" + game_name +
                             "\" twice!");


            i_db_game_generator* gen = nullptr;

            {
                config_map config(game_config);
                create_game_gen_fn_t& fn = create_fn_it->second;
                gen = fn(config);
                config.check_unused_keys();
            }

            THROW_ASSERT(gen != nullptr);

            if (!dry_run)
                db.generate_entries(*gen);

            delete gen;
        }
    }

    if (!dry_run)
        db.update_metadata_string(db_config_string);
}

init_database_enum resolve_auto_init_type(const string& filename)
{
    if (filesystem::exists(filename))
    {
        cout << "Autodetected database file: \"" << filename << "\". Loading..."
                  << endl;

        return INIT_DATABASE_LOAD;
    }

    cout << "Failed to autodetect database file: \"" << filename
              << "\". Disabling database..." << endl;

    global::use_db.set(false);
    return INIT_DATABASE_NONE;
}

} // namespace

namespace mcgs_init {
void init_database(const string& filename, init_database_enum init_type,
                   const string& db_config_string)
{
    init_global_database();
    database& db = get_global_database();

    assert(db.empty());

    if (init_type == INIT_DATABASE_AUTO)
        init_type = resolve_auto_init_type(filename);

    if (init_type == INIT_DATABASE_LOAD)
    {
        db.load(filename);
        cout << "Database file loaded:";
        cout << " \"" << filename << "\"" << endl;
    }

    register_games(db);
    fill_database(db, db_config_string, true);

    if (init_type == INIT_DATABASE_CREATE)
    {
        fill_database(db, db_config_string, false);
        db.save(filename);
        cout << "Database file saved:";
        cout << " \"" << filename << "\"" << endl;
    }

    if (init_type != INIT_DATABASE_NONE && global::print_db_info())
    {
        cout << "Database info: " << endl;
        cout << db << std::flush;
    }
}

vector<pair<string, string>> split_db_config_string_by_game_name(
    const string& db_config_string)
{
    vector<pair<string, string>> config_pairs;

    size_t idx = 0;
    const size_t SIZE = db_config_string.size();

    string current_game_name;
    string current_game_config;

    auto consume_white_space = [&]() -> void
    {
        while (idx < SIZE)
        {
            const char c = db_config_string[idx];

            if (isspace(c))
                idx++;
            else
                break;
        }
    };

    auto get_game_name = [&]() -> bool
    {
        if (!(idx < SIZE))
            return false;

        if (db_config_string[idx] != '[')
            return false;
        
        idx++;

        current_game_name.clear();
        bool found_closing_bracket = false;

        while (idx < SIZE)
        {
            const char c = db_config_string[idx];
            idx++;

            if (c == ']')
            {
                found_closing_bracket = true;
                break;
            }

            current_game_name.push_back(c);
        }

        return found_closing_bracket && !current_game_name.empty();
    };

    auto get_game_config = [&]() -> bool
    {
        current_game_config.clear();

        if (!(idx < SIZE))
            return true;

        while (idx < SIZE)
        {
            const char c = db_config_string[idx];

            if (c == '[')
                break;

            if (c == ']')
                return false;

            idx++;
            current_game_config.push_back(c);
        }

        return true;
    };

    while (idx < SIZE)
    {
        consume_white_space();

        if (!(idx < SIZE))
            break;

        const bool found_name = get_game_name();
        THROW_ASSERT(
            found_name,
            "Error when parsing DB config string: failed to find game name!");

        const bool found_config = get_game_config();
        THROW_ASSERT(
            found_config,
            "Error when parsing DB config string, in config for game \"" +
                current_game_name + "\"!");

        config_pairs.emplace_back(std::move(current_game_name),
                                  std::move(current_game_config));
    }

    return config_pairs;
}


} // namespace mcgs_init




//////////////////////////////////////////////////
namespace {

void register_games(database& db)
{
    /*
        Types used to query the database must be registered, but order doesn't
        matter. The database maps runtime game type IDs to disk game type IDs.
    */
    assert(create_gen_funcs.empty());

    // clobber_1xn
    DATABASE_REGISTER_TYPE(db, clobber_1xn);

    register_create_game_gen_fn("clobber_1xn",
                                 get_gridlike_create_game_gen_fn<clobber_1xn>(
                                     {BLACK, WHITE}, true, EMPTY));

    // nogo_1xn
    DATABASE_REGISTER_TYPE(db, nogo_1xn);

    register_create_game_gen_fn(
        "nogo_1xn", get_gridlike_create_game_gen_fn<nogo_1xn>({BLACK, WHITE},
                                                              false, EMPTY));

    // elephants
    DATABASE_REGISTER_TYPE(db, elephants);

    register_create_game_gen_fn(
        "elephants",
        get_gridlike_create_game_gen_fn<elephants>({BLACK, WHITE, EMPTY}));

    // clobber
    DATABASE_REGISTER_TYPE(db, clobber);

    register_create_game_gen_fn(
        "clobber",
        get_gridlike_create_game_gen_fn<clobber>({BLACK, WHITE}, true, EMPTY));

    // nogo
    DATABASE_REGISTER_TYPE(db, nogo);

    register_create_game_gen_fn("nogo", get_gridlike_create_game_gen_fn<nogo>(
                                             {BLACK, WHITE}, false, EMPTY));

    // domineering
    DATABASE_REGISTER_TYPE(db, domineering);

    register_create_game_gen_fn(
        "domineering",
        get_gridlike_create_game_gen_fn<domineering>({EMPTY}, true, BORDER));

    // amazons
    DATABASE_REGISTER_TYPE(db, amazons);

    register_create_game_gen_fn("amazons",
                                 get_gridlike_create_game_gen_fn<amazons>(
                                     {BORDER, BLACK, WHITE}, false, EMPTY));

    // fission
    DATABASE_REGISTER_TYPE(db, fission);

    register_create_game_gen_fn(
        "fission", get_gridlike_create_game_gen_fn<fission>({BORDER, BLACK},
                                                            false, EMPTY));

    // toppling_dominoes
    DATABASE_REGISTER_TYPE(db, toppling_dominoes);

    register_create_game_gen_fn(
        "toppling_dominoes", get_gridlike_create_game_gen_fn<toppling_dominoes>(
                                 {BLACK, WHITE}, true, BORDER));

    // sheep
    DATABASE_REGISTER_TYPE(db, sheep);

    register_create_game_gen_fn("sheep", create_sheep_gen);
}

} // namespace
