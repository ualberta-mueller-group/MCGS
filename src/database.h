/*
    Database class, and database entry structs.

    Must register game types during MCGS initialization (i.e. in
    `mcgs_init_all()`) by calling DATABASE_REGISTER_TYPE(...) to map game_type_t
    values to game name strings.
*/
#pragma once
#include <cassert>
#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

#include "bounds.h"
#include "cgt_basics.h"
#include "db_game_generator.h"
#include "dominated_moves.h"
#include "game.h"
#include "hashing.h"
#include "impartial_game.h"
#include "sumgame.h"
#include "thermograph_cache.h"
#include "type_mapper.h"
#include "type_table.h"

#include "ThGraph.h"

#define DATABASE_REGISTER_TYPE(db, game_class_name)                            \
    db.register_type(#game_class_name, game_type<game_class_name>())

class database;

////////////////////////////////////////////////// struct db_entry_partisan
struct db_entry_partisan
{
    inline db_entry_partisan() : outcome(outcome_class::U), complexity(0) {}

    bool operator==(const db_entry_partisan& other) const;
    bool operator!=(const db_entry_partisan& other) const;

    void print(std::ostream& os, const database& db,
               bool print_endl = false) const;

    outcome_class outcome;
    std::shared_ptr<ThGraph> thermograph;
    std::shared_ptr<game_bounds> bounds_data;
    uint64_t complexity;
    std::shared_ptr<db_dom_moves_t> dominated_moves;
};

//////////////////////////////////////// db_entry_partisan methods
inline bool db_entry_partisan::operator!=(const db_entry_partisan& other) const
{
    return !(*this == other);
}

////////////////////////////////////////////////// struct db_entry_impartial
struct db_entry_impartial
{
    inline db_entry_impartial() : nim_value(-1) {}

    bool operator==(const db_entry_impartial& other) const;
    bool operator!=(const db_entry_impartial& other) const;

    int nim_value;
};

std::ostream& operator<<(std::ostream& os, const db_entry_impartial& entry);

//////////////////////////////////////// db_entry_impartial methods
inline bool db_entry_impartial::operator==(
    const db_entry_impartial& other) const
{
    return nim_value == other.nim_value;
}

inline bool db_entry_impartial::operator!=(
    const db_entry_impartial& other) const
{
    return !(*this == other);
}

inline std::ostream& operator<<(std::ostream& os,
                                const db_entry_impartial& entry)
{
    os << entry.nim_value;
    return os;
}


////////////////////////////////////////////////// class database
#define DB_MAP_T std::unordered_map

class database
{
public:
    database();

    /*
        I/O functions
    */
    void save(const std::string& filename) const;
    void load(const std::string& filename);

    // Human readable, one entry per line
    void dump_to_stream(std::ostream& os) const;
    void dump_to_file(const std::string& out_filename) const;

    /*
        Partisan entry lookup. These return nullptr when the entry doesn't
        already exist, except for the "or_allocate" versions (which will
        create the entry).

        - Fields may not be computed, even if the entry exists.
        - The entry is owned by database (the caller should not attempt to
            delete/free it).
        - Pointers to entries are not invalidated by allocating new entries.
    */
    const db_entry_partisan* get_partisan_ptr(const game& g) const;
    db_entry_partisan* get_partisan_ptr(const game& g);
    db_entry_partisan* get_or_allocate_partisan_ptr(const game& g);

    const db_entry_partisan* get_partisan_ptr(const sumgame& sum) const;
    db_entry_partisan* get_partisan_ptr(const sumgame& sum);
    db_entry_partisan* get_or_allocate_partisan_ptr(const sumgame& sum);

    /*
        Impartial lookup functions.
    */
    std::optional<db_entry_impartial> get_impartial(const game& g) const;
    void set_impartial(const game& g, const db_entry_impartial& entry);

    /*
        Entry generation functions. When `silent` is true, info is not
        printed to stdout.
    */
    void generate_entries_partisan(i_db_game_generator& gen,
                                   bool silent = false);
    void generate_single_partisan_entry(sumgame& sum, bool silent);

    void generate_entries_impartial(i_db_game_generator& gen,
                                    bool silent = false);

    /*
        Misc utility functions.
    */
    void clear();
    bool empty() const;

    bool is_equal(const database& other) const;

    void update_metadata_string(const std::string& config_string);

    static hash_t get_db_hash(const game& g, global_hash& gh);
    static hash_t get_db_hash(const sumgame& sum);
    hash_t get_db_hash(const game& g) const;

    /*
        Game-type-related functions.

        TODO: eventually remove `__register_built_in_types()` hack! This
        function registers `integer_game` so that empty sums can be represented
        by using this game type.
    */

    // NOTE: Use macro DATABASE_REGISTER_TYPE instead!
    void register_type(const std::string& type_name, game_type_t runtime_type);

    void __register_built_in_types(); // NOLINT(readability-identifier-naming)
  
private:
    /*
        Private typedefs and friend declarations.
    */

    // Terminal layers
    typedef DB_MAP_T<hash_t, db_entry_partisan> terminal_layer_partisan_t;
    typedef DB_MAP_T<hash_t, db_entry_impartial> terminal_layer_impartial_t;

    // Trees
    typedef DB_MAP_T<game_type_t, terminal_layer_partisan_t> tree_partisan_t;
    typedef DB_MAP_T<game_type_t, terminal_layer_impartial_t> tree_impartial_t;

    friend std::ostream& operator<<(std::ostream& os, const database& db);

    /*
        Misc private functions
    */
    sumgame& _get_sumgame();
    global_hash& _get_global_hash() const;
    thermograph_cache& _get_graph_cache() const;

    void _generate_single_impartial_entry(impartial_game* ig, bool silent);

    static game_type_t _get_sum_db_type(const sumgame& sum);
    static game_type_t _get_game_db_type(const game& g);

    static void _db_print_sum(std::ostream& os, const sumgame& sum);

    /*
        Lookup implementations.
    */
    template <class Game_Or_Sum_T>
    db_entry_partisan* _get_partisan_impl(const Game_Or_Sum_T& g) const;

    template <class Game_Or_Sum_T>
    db_entry_partisan* _get_or_allocate_partisan_impl(const Game_Or_Sum_T& g);

    /*
        "Runtime-only" data that isn't stored on disk.
    */
    std::unique_ptr<sumgame> _sum;
    mutable std::unique_ptr<global_hash> _global_hash;
    uint64_t _n_entries_generated;

    /*
        Data stored on disk.
    */
    std::string _metadata_string;
    type_mapper _mapper;
    std::unique_ptr<thermograph_cache> _graph_cache;
    tree_partisan_t _tree_partisan;
    tree_impartial_t _tree_impartial;
};

std::ostream& operator<<(std::ostream& os, const database& db);

////////////////////////////////////////////////// database methods


