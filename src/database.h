#pragma once
#include <string>
#include <unordered_map>
#include "cgt_basics.h"
#include "game.h"

////////////////////////////////////////////////// test function
void db_test();

////////////////////////////////////////////////// db_entry_partizan
struct db_entry_partizan
{
    db_entry_partizan(): outcome(outcome_class::U)
    {
    }

    outcome_class outcome;
};

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


////////////////////////////////////////////////// db_entry_impartial
struct db_entry_impartial
{
    db_entry_impartial(): nim_value(-1)
    {
    }

    int nim_value;
};

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
    database(const std::string& filename);

    void set_partizan(const game& g, const db_entry_partizan& entry);
    void set_impartial(const game& g, const db_entry_impartial& entry);

    std::optional<db_entry_partizan> get_partizan(const game& g) const;
    std::optional<db_entry_impartial> get_impartial(const game& g) const;

    void save() const;
    void load();

    void clear();

private:
public:
    // Terminal layers
    typedef DB_MAP_T<hash_t, db_entry_partizan> terminal_layer_partizan_t;
    typedef DB_MAP_T<hash_t, db_entry_impartial> terminal_layer_impartial_t;

    // Trees
    typedef DB_MAP_T<game_type_t, terminal_layer_partizan_t> tree_partizan_t;
    typedef DB_MAP_T<game_type_t, terminal_layer_impartial_t> tree_impartial_t;

private:
    const std::string _filename;

    tree_partizan_t _tree_partizan;
    tree_impartial_t _tree_impartial;
};
