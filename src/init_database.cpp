#include "init_database.h"

#include <cctype>
#include <filesystem>
#include <functional>
#include <unordered_set>
#include <optional>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>
#include <cstddef>
#include <string>
#include <iostream>
#include <cassert>

#include "config_map.h"
#include "database.h"
#include "db_game_generator.h"
#include "db_make_simplest_equal_game.h"
#include "global_database.h"
#include "global_options.h"

// Registered games
#include "clobber_1xn.h"
#include "grid_generator.h"
#include "gridlike_db_game_generator.h"
#include "impartial_game_wrapper.h"
#include "nogo_1xn.h"
#include "elephants.h"

#include "clobber.h"
#include "cannibal_clobber.h"
#include "nogo.h"
#include "domineering.h"
#include "amazons.h"
#include "fission.h"
#include "cgt_integer_game.h"
#include "paths.h"
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

    return new gridlike_db_game_generator<sheep, GRIDLIKE_TYPE_GRID>(gg);
}

struct db_game_gen_registration_t
{
    db_game_gen_registration_t(create_game_gen_fn_t func, bool is_impartial,
                           db_gen_size_score_type size_score_type)
        : func(func), is_impartial(is_impartial), size_score_type(size_score_type)
    {
    }

    create_game_gen_fn_t func;
    bool is_impartial;
    // Game-specific default size_score. Can override in DB config string
    db_gen_size_score_type size_score_type;
};

unordered_map<string, db_game_gen_registration_t> db_game_gen_registrations;

void register_games(database& db);

void register_db_game_gen(const string& name, bool is_impartial,
                          db_gen_size_score_type default_size_score_type,
                          create_game_gen_fn_t& fn)
{
    THROW_ASSERT(
        name.size() > 0,
        "Attempted to register DB game generator for game with blank name!");

    const auto inserted = db_game_gen_registrations.emplace(
        name, db_game_gen_registration_t(fn, is_impartial, default_size_score_type));

    THROW_ASSERT(inserted.second,
                 "DB game generator registered twice for game \"" + name +
                     "\"!");

    if (!is_impartial)
        register_db_game_gen("impartial " + name, true, default_size_score_type, fn);
}

void register_db_game_gen(const string& name, bool is_impartial,
                          db_gen_size_score_type default_size_score_type,
                          create_game_gen_fn_t&& fn)
{
    register_db_game_gen(name, is_impartial, default_size_score_type, fn);
}

pair<unique_ptr<i_db_game_generator>, db_gen_options_t>
create_generator_and_options(const db_game_gen_registration_t& reg,
                             const string& game_config_string)
{
    pair<unique_ptr<i_db_game_generator>, db_gen_options_t> p;
    unique_ptr<i_db_game_generator>& gen = p.first;
    db_gen_options_t& opts = p.second;

    config_map config(game_config_string);

    // Make generator
    gen.reset(reg.func(config));

    // Apply game-specific default DB gen options
    opts.size_score_type = reg.size_score_type;

    // Apply override DB gen options
    const string* size_score_type_str = config.get_string_nullable("size_score");
    if (size_score_type_str != nullptr)
    {
        optional<db_gen_size_score_type> size_score_enum =
            string_to_db_gen_size_score_type(*size_score_type_str);

        THROW_ASSERT(size_score_enum.has_value(),
                     "Invalid value for size_score in DB config string: \"" +
                         *size_score_type_str + "\"");

        opts.size_score_type = *size_score_enum;
    }

    const string* stop_after_str = config.get_string_nullable("stop_after");
    if (stop_after_str != nullptr)
    {
        optional<db_gen_stop_after_enum> stop_after_enum =
            string_to_db_gen_stop_after_enum(*stop_after_str);

        THROW_ASSERT(stop_after_enum.has_value(),
                     "Invalid value for stop_after in DB config string: \"" +
                         *stop_after_str + "\"");

        opts.stop_after = *stop_after_enum;
    }

    config.check_unused_keys();

    return p;
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

            const auto reg_iter = db_game_gen_registrations.find(game_name);

            THROW_ASSERT(
                reg_iter != db_game_gen_registrations.end(),
                "Error: DB config references game \"" + game_name +
                    "\" which has no registered DB game generator!");

            THROW_ASSERT(game_names.insert(game_name).second,
                         "Error: DB config references game \"" + game_name +
                             "\" twice!");

            const db_game_gen_registration_t& reg = reg_iter->second;

            pair<unique_ptr<i_db_game_generator>, db_gen_options_t>
                gen_and_opts = create_generator_and_options(reg, game_config);

            unique_ptr<i_db_game_generator>& gen = gen_and_opts.first;
            const db_gen_options_t& opts = gen_and_opts.second;

            THROW_ASSERT(gen.get() != nullptr);

            if (!dry_run)
            {
                if (reg.is_impartial)
                    db.generate_entries_impartial(*gen);
                else
                    db.generate_entries_partisan(*gen, opts);

                db.refine_partisan_links();
                delete_equivalence_classes();
            }

        }
    }

    if (!dry_run)
    {
        db.update_metadata_string(db_config_string);

        db.assert_links_equal();
    }
}

init_database_enum resolve_auto_init_type(optional<string>& filename)
{
    THROW_ASSERT(!filename.has_value());

    const filesystem::path p = get_default_db_path();

    if (filesystem::exists(p))
        filename = p.string();

    if (filename.has_value())
    {
        cout << "Autodetected database file: \"" << *filename << "\". Loading..."
                  << endl;

        return INIT_DATABASE_LOAD;
    }

    cout << "Failed to autodetect database.bin. Disabling database. See "
            "--db-file-load in `MCGS -h` output."
         << endl;

    global::use_db.set(false);
    return INIT_DATABASE_NONE;
}

} // namespace

namespace mcgs_init {
void init_database(optional<string> filename, init_database_enum init_type,
                   const string& db_config_string)
{
    init_global_database();
    database& db = get_global_database();

    assert(db.empty());

    if (init_type == INIT_DATABASE_AUTO)
        init_type = resolve_auto_init_type(filename);

    if (init_type == INIT_DATABASE_LOAD)
    {
        THROW_ASSERT(filename.has_value());

        db.load(*filename);
        cout << "Database file loaded:";
        cout << " \"" << *filename << "\"" << endl;
    }

    register_games(db);
    fill_database(db, db_config_string, true);

    if (init_type == INIT_DATABASE_CREATE)
    {
        THROW_ASSERT(filename.has_value());

        fill_database(db, db_config_string, false);
        db.save(*filename);
        cout << "Database file saved:";
        cout << " \"" << *filename << "\"" << endl;
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
    assert(db_game_gen_registrations.empty());

    /*
        Types used to query the database must be registered. The order matters:
        see the "Database File Portability" section in development-notes.md
    */

    // Registers `integer_game`
    db.__register_built_in_types();

    /*
        TODO impartial wrapper games having the same game_type_t is problematic
        because their DB entries will all be stored in the same terminal
        layer...
    */
    DATABASE_REGISTER_TYPE(db, impartial_game_wrapper);

    DATABASE_REGISTER_TYPE(db, clobber_1xn);
    DATABASE_REGISTER_TYPE(db, nogo_1xn);
    DATABASE_REGISTER_TYPE(db, elephants);
    DATABASE_REGISTER_TYPE(db, clobber);
    DATABASE_REGISTER_TYPE(db, nogo);
    DATABASE_REGISTER_TYPE(db, domineering);
    DATABASE_REGISTER_TYPE(db, amazons);
    DATABASE_REGISTER_TYPE(db, fission);
    DATABASE_REGISTER_TYPE(db, toppling_dominoes);
    DATABASE_REGISTER_TYPE(db, sheep);

    DATABASE_REGISTER_TYPE(db, cannibal_clobber);

    /*
        NOTE: 2nd argument to register_create_game_gen_fn(...) indicates
        whether or not the game is impartial.

        If your game is partisan: this argument should be false. The impartial
        wrapper variant will automatically be registered i.e. both
        "your_game_name" and "impartial your_game_name"

        If your game is impartial: this argument should be true
    */

    // clobber_1xn
    register_db_game_gen(
        "clobber_1xn", false, DEFAULT_DB_GEN_SIZE_SCORE_TYPE,
        get_gridlike_create_game_gen_fn<clobber_1xn, GRIDLIKE_TYPE_STRIP>(
            {BLACK, WHITE}, true, EMPTY));

    // nogo_1xn
    register_db_game_gen(
        "nogo_1xn", false, DEFAULT_DB_GEN_SIZE_SCORE_TYPE,
        get_gridlike_create_game_gen_fn<nogo_1xn, GRIDLIKE_TYPE_STRIP>(
            {BLACK, WHITE}, false, EMPTY));

    // elephants
    register_db_game_gen(
        "elephants", false, DEFAULT_DB_GEN_SIZE_SCORE_TYPE,
        get_gridlike_create_game_gen_fn<elephants, GRIDLIKE_TYPE_STRIP>(
            {BLACK, WHITE, EMPTY}));

    // clobber
    register_db_game_gen(
        "clobber", false, DEFAULT_DB_GEN_SIZE_SCORE_TYPE,
        get_gridlike_create_game_gen_fn<clobber, GRIDLIKE_TYPE_GRID>(
            {BLACK, WHITE}, true, EMPTY));

    // nogo
    register_db_game_gen(
        "nogo", false, DEFAULT_DB_GEN_SIZE_SCORE_TYPE,
        get_gridlike_create_game_gen_fn<nogo, GRIDLIKE_TYPE_GRID>(
            {BLACK, WHITE}, false, EMPTY));

    // domineering
    register_db_game_gen(
        "domineering", false, DEFAULT_DB_GEN_SIZE_SCORE_TYPE,
        get_gridlike_create_game_gen_fn<domineering, GRIDLIKE_TYPE_GRID>(
            {EMPTY}, true, BORDER));

    // amazons
    register_db_game_gen(
        "amazons", false, DEFAULT_DB_GEN_SIZE_SCORE_TYPE,
        get_gridlike_create_game_gen_fn<amazons, GRIDLIKE_TYPE_GRID>(
            {BORDER, BLACK, WHITE}, false, EMPTY));

    // fission
    register_db_game_gen(
        "fission", false, DEFAULT_DB_GEN_SIZE_SCORE_TYPE,
        get_gridlike_create_game_gen_fn<fission, GRIDLIKE_TYPE_GRID>(
            {BORDER, BLACK}, false, EMPTY));

    // toppling_dominoes
    register_db_game_gen(
        "toppling_dominoes", false, DEFAULT_DB_GEN_SIZE_SCORE_TYPE,
        get_gridlike_create_game_gen_fn<toppling_dominoes, GRIDLIKE_TYPE_STRIP>(
            {BLACK, WHITE}, true, BORDER));

    // sheep
    register_db_game_gen("sheep", false, DEFAULT_DB_GEN_SIZE_SCORE_TYPE,
                         create_sheep_gen);

    // cannibal_clobber
    register_db_game_gen(
        "cannibal_clobber", false, DEFAULT_DB_GEN_SIZE_SCORE_TYPE,
        get_gridlike_create_game_gen_fn<cannibal_clobber, GRIDLIKE_TYPE_GRID>(
            {BLACK, WHITE}, true, EMPTY));
}

} // namespace
