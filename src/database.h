#pragma once
#include <string>
#include <unordered_map>
#include "cgt_basics.h"
#include "game.h"
#include "sumgame.h"
#include "db_game_generator.h"
#include "type_mapper.h"

////////////////////////////////////////////////// struct db_entry_partizan
struct db_entry_partizan
{
    db_entry_partizan(): outcome(outcome_class::U)
    {
    }

    outcome_class outcome;
};

//////////////////////////////////////// serializer<db_entry_partizan>
template <>
struct serializer<db_entry_partizan>
{
    inline static void save(obuffer& os, const db_entry_partizan& entry)
    {
        os.write_u8(entry.outcome);
    }

    inline static db_entry_partizan load(ibuffer& is)
    {
        db_entry_partizan entry;
        entry.outcome = static_cast<outcome_class>(is.read_u8());
        return entry;
    }
};

////////////////////////////////////////////////// struct db_entry_impartial
struct db_entry_impartial
{
    db_entry_impartial(): nim_value(-1)
    {
    }

    int nim_value;
};

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

    void set_partizan(const game& g, const db_entry_partizan& entry);
    void set_impartial(const game& g, const db_entry_impartial& entry);

    std::optional<db_entry_partizan> get_partizan(const game& g) const;
    std::optional<db_entry_impartial> get_impartial(const game& g) const;

    void register_type(const std::string& type_name, game_type_t runtime_type);

    void save(const std::string& filename) const;
    void load(const std::string& filename);

    void clear();
    bool empty() const;

    void generate_entries(db_game_generator& gen);

    template <class T>
    void generate_entries_for_game();

private:
    friend std::ostream& operator<<(std::ostream& os, const database& db);

    // Terminal layers
    typedef DB_MAP_T<hash_t, db_entry_partizan> terminal_layer_partizan_t;
    typedef DB_MAP_T<hash_t, db_entry_impartial> terminal_layer_impartial_t;

    // Trees
    typedef DB_MAP_T<game_type_t, terminal_layer_partizan_t> tree_partizan_t;
    typedef DB_MAP_T<game_type_t, terminal_layer_impartial_t> tree_impartial_t;

    std::unique_ptr<sumgame> _sum;
    uint64_t _game_count;

    sumgame& _get_sumgame();
    void _generate_entry_single(game* g);

    tree_partizan_t _tree_partizan;
    tree_impartial_t _tree_impartial;

    type_mapper _mapper;
};

////////////////////////////////////////////////// database methods
inline void database::register_type(const std::string& type_name, game_type_t runtime_type)
{
    _mapper.register_type(type_name, runtime_type);
}

inline bool database::empty() const
{
    return _tree_partizan.empty() && _tree_impartial.empty();
}

template <class T>
void database::generate_entries_for_game()
{
    /*
        TODO these static asserts should be more descriptive...

        The type trait isn't really useful if it doesn't result in a
        nice error message
    */
    static_assert(has_create_db_game_generator_v<T>);

    db_game_generator* gen = T::create_db_game_generator();
    generate_entries(*gen);
    delete gen;
}

inline sumgame& database::_get_sumgame()
{
    if (_sum.get() == nullptr)
        _sum.reset(new sumgame(BLACK));

    return *_sum;
}

