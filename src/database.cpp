/*
    TODO: These save and load functions are not machine/compiler-independent
    (because game_type_t is an unsigned int).

    STL container serializer templates probably should all be removed, or,
    probably just the generic integer serializer template? It is very easy
    to write unsafe code with this...
*/
#include <algorithm>
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
#include "SgBlackWhite.h"
#include "ThGraph.h"
#include "cgt_basics.h"
#include "db_make_thermograph.h"
#include "game.h"
#include "hashing.h"
#include "impartial_sumgame.h"
#include "impartial_game.h"
#include "sumgame.h"
#include "iobuffer.h"
#include "serializer.h"
#include "db_game_generator.h"
#include "throw_assert.h"
#include "type_table.h"
#include "clobber_1xn.h"
#include "utilities.h"
#include "version_info.h"
#include "impartial_game_wrapper.h"

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
database::database() : _sum(nullptr), _game_count(0), _max_generation_depth(0)
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

void database::set_partisan(const game& g, const db_entry_partisan& entry)
{
    const game_type_t gt = _mapper.translate_type(g.game_type());
    THROW_ASSERT(gt > 0);

    const hash_t hash = _get_db_hash(g);
    auto it = _tree_partisan[gt].emplace(hash, entry);

    THROW_ASSERT(it.second); // not already found
}

void database::set_partisan(const sumgame& sum, const db_entry_partisan& entry)
{
    const optional<game_type_t> sum_type_opt = _get_sum_game_type(sum);
    THROW_ASSERT(sum_type_opt.has_value()); // no storing empty sums
    game_type_t sum_type = *sum_type_opt;

    const game_type_t gt = _mapper.translate_type(sum_type);
    THROW_ASSERT(gt > 0);

    const hash_t hash = _get_db_hash(sum);
    auto it = _tree_partisan[gt].emplace(hash, entry);

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

std::optional<db_entry_partisan> database::get_partisan(const game& g) const
{
    const game_type_t gt = _mapper.translate_type(g.game_type());
    if (gt == 0)
        return {};

    auto it1 = _tree_partisan.find(gt);
    if (it1 == _tree_partisan.end())
        return {};

    const terminal_layer_partisan_t& layer = it1->second;
    const hash_t hash = _get_db_hash(g);
    auto it2 = layer.find(hash);

    if (it2 == layer.end())
        return {};

    return it2->second;
}

std::optional<db_entry_partisan> database::get_partisan(const sumgame& sum) const
{
    const optional<game_type_t> sum_type_opt = _get_sum_game_type(sum);
    if (!sum_type_opt.has_value())
        return {};
    const game_type_t sum_type = *sum_type_opt;

    const game_type_t gt = _mapper.translate_type(sum_type);
    if (gt == 0)
        return {};

    auto it1 = _tree_partisan.find(gt);
    if (it1 == _tree_partisan.end())
        return {};

    const terminal_layer_partisan_t& layer = it1->second;
    const hash_t hash = _get_db_hash(sum);
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
    serializer<tree_partisan_t>::save(os, _tree_partisan);
    serializer<tree_impartial_t>::save(os, _tree_impartial);
    serializer<type_mapper>::save(os, _mapper);

    os.close();
}

void database::load(const std::string& filename)
{
    assert(_tree_partisan.empty());
    assert(_tree_impartial.empty());

    ibuffer is(filename);

    _metadata_string = serializer<string>::load(is);
    _tree_partisan = serializer<tree_partisan_t>::load(is);
    _tree_impartial = serializer<tree_impartial_t>::load(is);
    _mapper = serializer<type_mapper>::load(is);

    is.close();
}

void database::clear()
{
    _tree_partisan.clear();
    _tree_impartial.clear();
    _mapper.clear();
}

bool database::is_equal(const database& other) const
{
    if (_tree_partisan != other._tree_partisan)
        return false;

    if (_tree_impartial != other._tree_impartial)
        return false;

    if (_mapper != other._mapper)
        return false;

    return true;
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////
void database::generate_entries_partisan(i_db_game_generator& gen, bool silent)
{
    sumgame sum(BLACK);

    while (gen)
    {
        unique_ptr<game> g(gen.gen_game());
        ++gen;

        assert(sum.is_empty());
        sum.add(g.get());

        {
            assert_restore_sumgame ars(sum);
            _generate_entry_single_partisan(sum, 0, silent);
        }

        sum.pop(g.get());
    }
}

void database::_generate_entry_single_partisan(sumgame& sum, unsigned int depth,
                                               bool silent)
{
    assert_restore_sumgame ars(sum);

    /*
        TODO granularity of this could be improved?
    */
    const hash_t hash_pre_sn = sum.get_global_hash_for_player(EMPTY);
    sum.split_and_normalize();
    const hash_t hash_post_sn = sum.get_global_hash_for_player(EMPTY);

    if (hash_pre_sn != hash_post_sn || sum.num_active_games() > 1)
    {
        sumgame sum2(BLACK); // for single subgames

        // Handle subgames
        const int n_games = sum.num_total_games();
        for (int i = 0; i < n_games; i++)
        {
            game* sg = sum.subgame(i);
            if (!sg->is_active())
                continue;

            assert(sum2.is_empty());
            sum2.add(sg);
            _generate_entry_single_partisan_impl(sum2, depth, silent);
            sum2.pop(sg);
        }
    }

    _generate_entry_single_partisan_impl(sum, depth, silent);

    sum.undo_split_and_normalize();
}

void database::_generate_entry_single_partisan_impl(sumgame& sum,
                                                    unsigned int depth,
                                                    bool silent)
{
    _max_generation_depth = max(_max_generation_depth, depth);

    if (sum.is_empty() || get_partisan(sum).has_value())
        return;

    const bw restore_player = sum.to_play();

    auto visit_children_for_player = [this, &sum, &depth, &silent](bw player) -> void
    {
        const bw restore_player = sum.to_play();

        assert(is_black_white(player));
        sum.set_to_play(player);
        unique_ptr<sumgame_move_generator> gen(sum.create_sum_move_generator(player));

        while (*gen)
        {
            const sumgame_move sm = gen->gen_sum_move();
            ++(*gen);

            assert(sum.to_play() == player);
            sum.play_sum(sm, player);

            _generate_entry_single_partisan(sum, depth + 1, silent);

            sum.undo_move();
            assert(sum.to_play() == player);
        }

        sum.set_to_play(restore_player);
    };

    visit_children_for_player(BLACK);
    visit_children_for_player(WHITE);

    bool print_game = !silent && ((_game_count % 128) == 0);

    if (print_game)
    {
        cout << "Game # " << _game_count << ": ";
        _db_print_sum(cout, sum);
        cout << std::flush;
    }
    _game_count++;

    sum.set_to_play(BLACK);
    bool black_wins = sum.solve();

    sum.set_to_play(WHITE);
    bool white_wins = sum.solve();

    outcome_class oc = bools_to_outcome_class(black_wins, white_wins);
#ifdef MCGS_USE_THERM
    unique_ptr<ThGraph> thermograph(db_make_thermograph(*this, sum, 0));
#endif

    db_entry_partisan entry;
    entry.outcome = oc;
#ifdef MCGS_USE_THERM
    entry.thermograph = *thermograph;
#endif

    set_partisan(sum, entry);

    sum.set_to_play(restore_player);

    if (print_game)
        cout << " DONE" << endl;
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////

//    if (get_partisan(sum).has_value())
//        return;
//
//    // bool print_game = true;
//    bool print_game = !silent && ((_game_count % 128) == 0);
//
//    if (print_game)
//    {
//        //cout << "Game # " << _game_count << ": " << *g << std::flush;
//
//        cout << "Game # " << _game_count << ": ";
//        _db_print_sum(cout, sum);
//        cout << std::flush;
//    }
//    _game_count++;
//
//    sum.set_to_play(BLACK);
//    bool black_wins = sum.solve();
//
//    sum.set_to_play(WHITE);
//    bool white_wins = sum.solve();
//
//    outcome_class oc = bools_to_outcome_class(black_wins, white_wins);
//#ifdef MCGS_USE_THERM
//    unique_ptr<ThGraph> thermograph(db_make_thermograph(*this, sum));
//#endif
//
//    db_entry_partisan entry;
//    entry.outcome = oc;
//#ifdef MCGS_USE_THERM
//    entry.thermograph = *thermograph;
//#endif
//
//    set_partisan(sum, entry);
//
//    if (print_game)
//        cout << " DONE" << endl;




void database::generate_entries_impartial(i_db_game_generator& gen, bool silent)
{
    while (gen)
    {
        /*
           g can be partisan OR impartial. If partisan, we must wrap it
           in an impartial_game_wrapper
        */
        std::unique_ptr<game> g(gen.gen_game());
        ++gen;

        assert(logical_iff(                                   //
            g->is_impartial(),                                //
            dynamic_cast<impartial_game*>(g.get()) != nullptr //
            ));                                               //

        if (!g->is_impartial())
        {
            game* g_temp = g.release();
            g.reset(new impartial_game_wrapper(g_temp, true));
            // g now holds the wrapper, which now owns the game originally in g
        }

        assert(dynamic_cast<impartial_game*>(g.get()) != nullptr);
        impartial_game* ig = static_cast<impartial_game*>(g.get());

        // If game splits, handle subgames
        split_result sr = ig->split();
        if (sr.has_value())
        {
            for (game* sg : *sr)
            {
                assert(sg->is_impartial() &&                        //
                       dynamic_cast<impartial_game*>(sg) != nullptr //
                );

                impartial_game* sg_impartial = static_cast<impartial_game*>(sg);
                sg_impartial->normalize();

                _generate_entry_single_impartial(sg_impartial, silent);
                delete sg;
            }

            continue;
        }

        // Normalize, handle ig
        ig->normalize();
        _generate_entry_single_impartial(ig, silent);
    }
}


void database::_generate_entry_single_impartial(impartial_game* ig, bool silent)
{
    if (get_impartial(*ig).has_value())
        return;

    // bool print_game = true;
    bool print_game = !silent && ((_game_count % 128) == 0);

    if (print_game)
        cout << "Game # " << _game_count << ": " << *ig << std::flush;
    _game_count++;

    sumgame& s = _get_sumgame();
    assert(s.num_total_games() == 0);

    s.add(ig);

    int nim_value = search_impartial_sumgame(s);
    assert(nim_value >= 0);

    s.pop(ig);

    db_entry_impartial entry;
    entry.nim_value = nim_value;


    set_impartial(*ig, entry);
    assert(s.num_total_games() == 0);

    if (print_game)
        cout << " DONE" << endl;
}

hash_t database::_get_db_hash(const game& g) const
{
    global_hash& gh = _get_global_hash();
    return gh.get_global_hash_value(&g, EMPTY);
}

hash_t database::_get_db_hash(const sumgame& sum) const
{
    return sum.get_global_hash_for_player(EMPTY);
}

std::optional<game_type_t> database::_get_sum_game_type(const sumgame& sum)
{
    std::optional<game_type_t> sum_type;

    const int n_games = sum.num_total_games();
    for (int i = 0; i < n_games; i++)
    {
        const game* sg = sum.subgame_const(i);
        if (!sg->is_active())
            continue;

        const game_type_t sg_type = sg->game_type();

        // No mixed type sums for now...
        THROW_ASSERT(
            LOGICAL_IMPLIES(sum_type.has_value(), *sum_type == sg_type));

        sum_type = sg_type;
    }

    return sum_type;
}

void database::_db_print_sum(std::ostream& os, const sumgame& sum)
{
    bool not_first_game = false;

    const int n_games = sum.num_total_games();
    for (int i = 0; i < n_games; i++)
    {
        const game* sg = sum.subgame_const(i);
        if (!sg->is_active())
            continue;

        if (not_first_game)
            os << " + ";
        not_first_game = true;

        os << *sg;
    }
}


//////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, const database& db)
{
    const unordered_map<game_type_t, string>& disk_type_to_name_map =
        db._mapper.get_disk_type_to_name_map();

    os << "# of Partisan game types: " << db._tree_partisan.size() << '\n';
    os << "# of Impartial game types: " << db._tree_impartial.size() << '\n';

    for (const pair<const game_type_t, database::terminal_layer_partisan_t>& p :
         db._tree_partisan)
    {
        const game_type_t disk_type = p.first;
        const database::terminal_layer_partisan_t& layer = p.second;

        auto it = disk_type_to_name_map.find(disk_type);
        THROW_ASSERT(it != disk_type_to_name_map.end());

        const string& game_name = it->second;

        os << "\tGame type: \"" << game_name << "\" ";
        os << "Count: " << layer.size() << '\n';
    }

    for (const pair<const game_type_t, database::terminal_layer_impartial_t>& p :
         db._tree_impartial)
    {
        const game_type_t disk_type = p.first;
        const database::terminal_layer_impartial_t& layer = p.second;

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
