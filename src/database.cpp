/*
    TODO: These save and load functions are not machine/compiler-independent
    (because game_type_t is an unsigned int).

    STL container serializer templates probably should all be removed, or,
    probably just the generic integer serializer template? It is very easy
    to write unsafe code with this...
*/

#include "database.h"
#include <iostream>
#include <memory>

#include "sumgame.h"
#include "grid_utils.h"
#include "clobber_1xn.h"

using namespace std;

namespace {
outcome_class bools_to_outcome_class(bool black_wins, bool white_wins)
{
    if (!black_wins && !white_wins) // 00
        return P;
    if (!black_wins && white_wins) // 01
        return R;
    if (black_wins && !white_wins) // 10
        return L;
    if (black_wins && white_wins) // 11
        return N;

    assert(false);
}

} //

//////////////////////////////////////////////////
void db_test()
{
}

////////////////////////////////////////////////// implementation
database::database(const std::string& filename): _filename(filename)
{
}

void database::set_partizan(const game& g, const db_entry_partizan& entry)
{
    const game_type_t gt = g.game_type();
    const hash_t hash = g.get_local_hash();
    auto it = _tree_partizan[gt].emplace(hash, entry);

    assert(it.second); // not already found
}

void database::set_impartial(const game& g, const db_entry_impartial& entry)
{
    const game_type_t gt = g.game_type();
    const hash_t hash = g.get_local_hash();
    auto it = _tree_impartial[gt].emplace(hash, entry);

    assert(it.second); // not already found
}

std::optional<db_entry_partizan> database::get_partizan(const game& g) const
{
    const game_type_t gt = g.game_type();
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
    const game_type_t gt = g.game_type();
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

void database::save() const
{
    obuffer os(_filename);

    serializer<tree_partizan_t>::save(os, _tree_partizan);
    serializer<tree_impartial_t>::save(os, _tree_impartial);

    os.close();
}

void database::load()
{
    assert(_tree_partizan.empty());
    assert(_tree_impartial.empty());

    ibuffer is(_filename);

    _tree_partizan = serializer<tree_partizan_t>::load(is);
    _tree_impartial = serializer<tree_impartial_t>::load(is);

    is.close();
}

void database::clear()
{
    _tree_partizan.clear();
    _tree_impartial.clear();
}

void database::generate_entries(db_game_generator& gen)
{
    sumgame s(BLACK);

    while (gen)
    {
        game* g = gen.gen_game();
        ++gen;

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

        delete g;
    }

    assert(s.num_total_games() == 0);
}
