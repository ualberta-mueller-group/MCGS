/*
   Database class, and database entry structs.

   Must call register_type() to map game_type_t values to game name strings,
   i.e. in mcgs_init_all()
*/
#pragma once
#include <string>
#include <iostream>
#include <unordered_map>
#include <memory>
#include <cstdint>
#include <optional>

#include "bounds.h"
#include "cgt_basics.h"
#include "hashing.h"
#include "type_table.h"
#include "game.h"
#include "serializer.h"
#include "iobuffer.h"
#include "sumgame.h"
#include "db_game_generator.h"
#include "type_mapper.h"
#include "impartial_game.h"
#include "db_make_dominated_moves.h"

#include "ThGraph.h"
#include "serializer_lib_therm.h"


extern uint64_t n_db_games;
extern uint64_t n_db_games_with_bounds;
extern uint64_t n_db_bounds_infinitesimal;
extern uint64_t n_db_bounds_rational;
extern uint64_t n_db_bounds_equal;


#define DATABASE_REGISTER_TYPE(db, game_class_name)                            \
    db.register_type(#game_class_name, game_type<game_class_name>())

////////////////////////////////////////////////// struct db_entry_partisan
struct db_entry_partisan
{
    db_entry_partisan() : outcome(outcome_class::U) {}
    bool operator==(const db_entry_partisan& other) const;
    bool operator!=(const db_entry_partisan& other) const;

    outcome_class outcome;
#ifdef MCGS_USE_THERM
    std::shared_ptr<ThGraph> thermograph;
#endif

#ifdef MCGS_USE_BOUNDS
    std::optional<std::tuple<bound_scale, game_bounds_ptr>> bounds_data;
#endif

#ifdef MCGS_USE_DOMINANCE
    std::optional<std::shared_ptr<dominated_moves_t>> dominated_moves;
#endif
};

inline bool db_entry_partisan::operator==(const db_entry_partisan& other) const
{
    if (outcome != other.outcome)
        return false;

#ifdef MCGS_USE_THERM
    if ((thermograph.get() == nullptr) != (other.thermograph.get() == nullptr))
        return false;

    if (thermograph)
        if (!(*thermograph == *other.thermograph))
            return false;
#endif

#ifdef MCGS_USE_BOUNDS
    if (bounds_data.has_value() != other.bounds_data.has_value())
        return false;

    if (bounds_data.has_value())
    {
        const bound_scale& scale1 = std::get<0>(*bounds_data);
        const bound_scale& scale2 = std::get<0>(*other.bounds_data);
        if (scale1 != scale2)
            return false;

        const game_bounds_ptr& ptr1 = std::get<1>(*bounds_data);
        const game_bounds_ptr& ptr2 = std::get<1>(*other.bounds_data);

        if ((ptr1.get() == nullptr) != (ptr2.get() == nullptr))
            return false;

        if (ptr1 && *ptr1 != *ptr2)
            return false;
    }
#endif

#ifdef MCGS_USE_DOMINANCE
    if (dominated_moves.has_value() != other.dominated_moves.has_value())
        return false;

    if (dominated_moves && (*dominated_moves != *other.dominated_moves))
        return false;
#endif

    return true;
}

inline bool db_entry_partisan::operator!=(const db_entry_partisan& other) const
{
    return !(*this == other);
}

//////////////////////////////////////// serializer<db_entry_partisan>
template <>
struct serializer<db_entry_partisan>
{
    inline static void save(obuffer& os, const db_entry_partisan& entry)
    {
        serializer_save(os, entry.outcome);

#ifdef MCGS_USE_THERM
        serializer_save(os, entry.thermograph);
#endif

#ifdef MCGS_USE_BOUNDS
        serializer_save(os, entry.bounds_data);
#endif

#ifdef MCGS_USE_DOMINANCE
        serializer_save(os, entry.dominated_moves);
#endif

    }

    inline static db_entry_partisan load(ibuffer& is)
    {
        db_entry_partisan entry;

        serializer_load(is, entry.outcome);

#ifdef MCGS_USE_THERM
        serializer_load(is, entry.thermograph);
#endif

#ifdef MCGS_USE_BOUNDS
        serializer_load(is, entry.bounds_data);
#endif

#ifdef MCGS_USE_DOMINANCE
        serializer_load(is, entry.dominated_moves);
#endif

        return entry;
    }
};

////////////////////////////////////////////////// struct db_entry_impartial
struct db_entry_impartial
{
    db_entry_impartial() : nim_value(-1) {}
    bool operator==(const db_entry_impartial& other) const;
    bool operator!=(const db_entry_impartial& other) const;

    int nim_value;
};

inline bool db_entry_impartial::operator==(
    const db_entry_impartial& other) const
{
    return nim_value == other.nim_value;
}

inline bool db_entry_impartial::operator!=(const db_entry_impartial& other) const
{
    return !(*this == other);
}

//////////////////////////////////////// serializer<db_entry_impartial>
template <>
struct serializer<db_entry_impartial>
{
    inline static void save(obuffer& os, const db_entry_impartial& entry)
    {
        os.write_i32(entry.nim_value);
    }

    inline static db_entry_impartial load(ibuffer& is)
    {
        db_entry_impartial entry;
        entry.nim_value = is.read_i32();
        return entry;
    }
};

////////////////////////////////////////////////// class database
#define DB_MAP_T std::unordered_map

class database
{
public:
    database();

    void update_metadata_string(const std::string& config_string);

    void set_partisan(const game& g, const db_entry_partisan& entry);
    void set_partisan(const sumgame& sum, const db_entry_partisan& entry);

    void set_impartial(const game& g, const db_entry_impartial& entry);

    std::optional<db_entry_partisan> get_partisan(const game& g) const;
    std::optional<db_entry_partisan> get_partisan(const sumgame& sum) const;

    /*
        TODO pointer is invalidated by modifying DB
        Also these should be const, but sumgame_move_generator needs to
        create a shared_ptr/weak_ptr to something in the entry...
    */
    db_entry_partisan* get_partisan_ptr(const game& g);
    db_entry_partisan* get_partisan_ptr(const sumgame& sum);

    std::optional<db_entry_impartial> get_impartial(const game& g) const;

    void register_type(const std::string& type_name, game_type_t runtime_type);

    void save(const std::string& filename) const;
    void load(const std::string& filename);

    void clear();
    bool empty() const;

    unsigned int get_max_generation_depth() const;

    bool is_equal(const database& other) const;

    // silent=true silences printing to stdout

//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////
public:
    void generate_entries_partisan(i_db_game_generator& gen,
                                   bool silent = false);
private:
    void _generate_entry_single_partisan(sumgame& sum, unsigned int depth,
                                         bool silent);

    void _generate_entry_single_partisan_impl(sumgame& sum, unsigned int depth,
                                              bool silent);

//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////

public:
    void generate_entries_impartial(i_db_game_generator& gen, bool silent = false);

private:
    friend std::ostream& operator<<(std::ostream& os, const database& db);

    // Terminal layers
    typedef DB_MAP_T<hash_t, db_entry_partisan> terminal_layer_partisan_t;
    typedef DB_MAP_T<hash_t, db_entry_impartial> terminal_layer_impartial_t;

    // Trees
    typedef DB_MAP_T<game_type_t, terminal_layer_partisan_t> tree_partisan_t;
    typedef DB_MAP_T<game_type_t, terminal_layer_impartial_t> tree_impartial_t;

    std::unique_ptr<sumgame> _sum; // sumgame for solving games
    mutable std::unique_ptr<global_hash> _global_hash;
    uint64_t _game_count;          // count incremented by generate_entries()

    sumgame& _get_sumgame();
    global_hash& _get_global_hash() const;

    void _generate_entry_single_impartial(impartial_game* ig, bool silent);

    hash_t _get_db_hash(const game& g) const;
    hash_t _get_db_hash(const sumgame& sum) const;

    static std::optional<game_type_t> _get_sum_game_type(const sumgame& sum);
    static void _db_print_sum(std::ostream& os, const sumgame& sum);

    std::string _metadata_string;
    tree_partisan_t _tree_partisan;
    tree_impartial_t _tree_impartial;

    unsigned int _max_generation_depth;

    type_mapper _mapper;
};

////////////////////////////////////////////////// database methods
inline void database::register_type(const std::string& type_name,
                                    game_type_t runtime_type)
{
    _mapper.register_type(type_name, runtime_type);
}

inline bool database::empty() const
{
    return _tree_partisan.empty() && _tree_impartial.empty();
}


inline unsigned int database::get_max_generation_depth() const
{
    return _max_generation_depth;
}


inline sumgame& database::_get_sumgame()
{
    if (_sum.get() == nullptr)
        _sum.reset(new sumgame(BLACK));

    return *_sum;
}

inline global_hash& database::_get_global_hash() const
{
    if (_global_hash.get() == nullptr)
        _global_hash.reset(new global_hash());

    return *_global_hash;
}

