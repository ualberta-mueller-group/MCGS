/*
    TODO: These save and load functions are not machine/compiler-independent
    (because game_type_t is an unsigned int).

    STL container serializer templates probably should all be removed, or,
    probably just the generic integer serializer template? It is very easy
    to write unsafe code with this...
*/
#include <cassert>
#include <chrono>
#include <ctime>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
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
#include "version_info.h"

using namespace std;

////////////////////////////////////////////////// helper functions
namespace {

string get_time_string()
{
    string time_string;

    const std::chrono::time_point time = std::chrono::system_clock::now();
    const std::time_t time_converted = std::chrono::system_clock::to_time_t(time);

    const char* time_ctime = std::ctime(&time_converted);
    assert(time_ctime != nullptr);

    while (true)
    {
        const char c = *time_ctime;
        ++time_ctime;

        if (c == 0 || c == '\n')
            break;

        time_string.push_back(c);
    }

    return time_string;
}

} // namespace

////////////////////////////////////////////////// database methods
database::database() : _sum(nullptr), _game_count(0)
{
}

/*
    - MCGS version
    - Date created
    - Game config string
*/
void database::update_metadata_string(const string& config_string)
{
    _metadata_string.clear();

    // TODO commit hash instead?
    _metadata_string += "Version: \"" + string(MCGS_VERSION_STRING) + "\"\n";

    // Date
    _metadata_string += "Date created: \"" + get_time_string() + "\"\n";

    // Game config
    _metadata_string += "DB config string: \"" + config_string + "\"";
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

    serializer<string>::save(os, _metadata_string);
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

    _metadata_string = serializer<string>::load(is);
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

void database::generate_entries(i_db_game_generator& gen, bool silent)
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
    const unordered_map<game_type_t, string>& disk_type_to_name_map =
        db._mapper.get_disk_type_to_name_map();

    os << "# of Partizan game types: " << db._tree_partizan.size() << '\n';

    for (const pair<const game_type_t, database::terminal_layer_partizan_t>& p :
         db._tree_partizan)
    {
        const game_type_t disk_type = p.first;
        const database::terminal_layer_partizan_t& layer = p.second;

        auto it = disk_type_to_name_map.find(disk_type);
        THROW_ASSERT(it != disk_type_to_name_map.end());

        const string& game_name = it->second;

        os << "\tGame type: \"" << game_name << "\" ";
        os << "Count: " << layer.size() << '\n';
    }

    os << "=== Database metadata string begin ===" << "\n";
    os << db._metadata_string << "\n";
    os << "=== Database metadata string end ===" << "\n";

    return os;
}
