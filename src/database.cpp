/*
    TODO: These save and load functions are not machine/compiler-independent
    (because game_type_t is an unsigned int).

    STL container serializer templates probably should all be removed, or,
    probably just the generic integer serializer template? It is very easy
    to write unsafe code with this...
*/
#include <cassert>
#include <optional>
#include <string>
#include <memory>
#include <iostream>
#include "database.h"
#include "sumgame.h"
#include "iobuffer.h"
#include "serializer.h"
#include "db_game_generator.h"
#include "throw_assert.h"
#include "type_table.h"
#include "clobber_1xn.h"
#include "utilities.h"

using namespace std;

////////////////////////////////////////////////// database methods
database::database() : _sum(nullptr), _game_count(0)
{
}

void database::set_partizan(const game& g, const db_entry_partizan& entry)
{
    const game_type_t gt = _mapper.translate_type(g.game_type());
    THROW_ASSERT(gt > 0);

    const hash_t hash = g.get_local_hash();
    auto it = _tree_partizan[gt].emplace(hash, entry);

    THROW_ASSERT(it.second); // not already found
}

void database::set_impartial(const game& g, const db_entry_impartial& entry)
{
    const game_type_t gt = _mapper.translate_type(g.game_type());
    THROW_ASSERT(gt > 0);

    const hash_t hash = g.get_local_hash();
    auto it = _tree_impartial[gt].emplace(hash, entry);

    THROW_ASSERT(it.second); // not already found
}

std::optional<db_entry_partizan> database::get_partizan(const game& g) const
{
    const game_type_t gt = _mapper.translate_type(g.game_type());
    if (gt == 0)
        return {};

    auto it1 = _tree_partizan.find(gt);
    if (it1 == _tree_partizan.end())
        return {};

    const terminal_layer_partizan_t& layer = it1->second;
    const hash_t hash = g.get_local_hash();
    auto it2 = layer.find(hash);

    if (it2 == layer.end())
        return {};

    return it2->second;
}

std::optional<db_entry_impartial> database::get_impartial(const game& g) const
{
    const game_type_t gt = _mapper.translate_type(g.game_type());
    if (gt == 0)
        return {};

    auto it1 = _tree_impartial.find(gt);
    if (it1 == _tree_impartial.end())
        return {};

    const terminal_layer_impartial_t& layer = it1->second;
    const hash_t hash = g.get_local_hash();
    auto it2 = layer.find(hash);

    if (it2 == layer.end())
        return {};

    return it2->second;
}

void database::save(const std::string& filename) const
{
    obuffer os(filename);

    serializer<tree_partizan_t>::save(os, _tree_partizan);
    serializer<tree_impartial_t>::save(os, _tree_impartial);
    serializer<type_mapper>::save(os, _mapper);

    os.close();
}

void database::load(const std::string& filename)
{
    assert(_tree_partizan.empty());
    assert(_tree_impartial.empty());

    ibuffer is(filename);

    _tree_partizan = serializer<tree_partizan_t>::load(is);
    _tree_impartial = serializer<tree_impartial_t>::load(is);
    _mapper = serializer<type_mapper>::load(is);

    is.close();
}

void database::clear()
{
    _tree_partizan.clear();
    _tree_impartial.clear();
    _mapper.clear();
}

void database::generate_entries(db_game_generator& gen, bool silent)
{
    while (gen)
    {
        std::unique_ptr<game> g(gen.gen_game());
        ++gen;

        // If game splits, handle subgames
        split_result sr = g->split();
        if (sr.has_value())
        {
            for (game* sg : *sr)
            {
                sg->normalize();
                _generate_entry_single(sg, silent);
                delete sg;
            }

            continue;
        }

        // Normalize, handle g
        g->normalize();
        _generate_entry_single(g.get(), silent);
    }
}

void database::_generate_entry_single(game* g, bool silent)
{
    if (get_partizan(*g).has_value())
        return;

    // bool print_game = true;
    bool print_game = !silent && ((_game_count % 128) == 0);

    if (print_game)
        cout << "Game # " << _game_count << ": " << *g << std::flush;
    _game_count++;

    sumgame& s = _get_sumgame();
    assert(s.num_total_games() == 0);

    s.add(g);

    s.set_to_play(BLACK);
    bool black_wins = s.solve();

    s.set_to_play(WHITE);
    bool white_wins = s.solve();

    s.pop(g);

    outcome_class oc = bools_to_outcome_class(black_wins, white_wins);

    db_entry_partizan entry;
    entry.outcome = oc;

    set_partizan(*g, entry);
    assert(s.num_total_games() == 0);

    if (print_game)
        cout << " DONE" << endl;
}

//////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, const database& db)
{
    os << db._mapper << '\n';

    os << "Partizan game types: " << db._tree_partizan.size() << '\n';
    for (const auto& it : db._tree_partizan)
    {
        os << "Game type: " << it.first << ' ';
        os << "Count: " << it.second.size() << '\n';
    }

    os << '\n';

    //os << "Impartial game types: " << db._tree_impartial.size() << '\n';
    //for (const auto& it : db._tree_impartial)
    //{
    //    os << "Game type: " << it.first << ' ';
    //    os << "Count: " << it.second.size() << '\n';
    //}

    return os;
}
