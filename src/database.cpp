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
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <memory>
#include <iostream>

#include "database.h"
#include "ThGraph.h"
#include "bounds.h"
#include "cgt_basics.h"
#include "cgt_integer_game.h"
#include "db_make_bounds.h"
#include "db_make_dominated_moves.h"
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


uint64_t n_db_games = 0;
uint64_t n_db_games_with_bounds = 0;
uint64_t n_db_bounds_infinitesimal = 0;
uint64_t n_db_bounds_rational = 0;
uint64_t n_db_bounds_equal = 0;

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

////////////////////////////////////////////////// db_entry_partisan methods
bool db_entry_partisan::operator==(const db_entry_partisan& other) const
{
    // Note confusing use of `shared_ptr::operator bool()` in this function

    if (outcome != other.outcome)
        return false;

#ifdef MCGS_USE_THERM
    if (thermograph != other.thermograph)
        return false;

    if (thermograph && !(*thermograph == *other.thermograph))
            return false;
#endif

#ifdef MCGS_USE_BOUNDS
    if (bounds_data != other.bounds_data)
        return false;

    if (bounds_data && (*bounds_data != *other.bounds_data))
        return false;
#endif

#ifdef MCGS_USE_DOMINANCE
    if (dominated_moves != other.dominated_moves)
        return false;

    if (dominated_moves && (*dominated_moves != *other.dominated_moves))
        return false;
#endif

    return true;
}

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
    const game_type_t sum_type = _get_sum_game_type(sum);

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

void database::__register_built_in_types()
{
    DATABASE_REGISTER_TYPE((*this), integer_game);
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

db_entry_partisan* database::get_partisan_ptr(const game& g)
{
    const game_type_t gt = _mapper.translate_type(g.game_type());
    if (gt == 0)
        return nullptr;

    auto it1 = _tree_partisan.find(gt);
    if (it1 == _tree_partisan.end())
        return nullptr;

    terminal_layer_partisan_t& layer = it1->second;
    const hash_t hash = _get_db_hash(g);
    auto it2 = layer.find(hash);

    if (it2 == layer.end())
        return nullptr;

    return &(it2->second);
}

db_entry_partisan* database::get_partisan_ptr(const sumgame& sum)
{
    const optional<game_type_t> sum_type_opt = _get_sum_game_type(sum);
    if (!sum_type_opt.has_value())
        return nullptr;
    const game_type_t sum_type = *sum_type_opt;

    const game_type_t gt = _mapper.translate_type(sum_type);
    if (gt == 0)
        return nullptr;

    auto it1 = _tree_partisan.find(gt);
    if (it1 == _tree_partisan.end())
        return nullptr;

    terminal_layer_partisan_t& layer = it1->second;
    const hash_t hash = _get_db_hash(sum);
    auto it2 = layer.find(hash);

    if (it2 == layer.end())
        return nullptr;

    return &it2->second;
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

void database::dump_to_stream(std::ostream& os) const
{
    os << _mapper << '\n';

    for (const std::pair<const game_type_t, terminal_layer_partisan_t>&
             terminal_layer : _tree_partisan)
    {
        for (const std::pair<const hash_t, db_entry_partisan>& entry_pair :
             terminal_layer.second)
        {
            os << entry_pair << '\n';
        }
    }

    for (const std::pair<const game_type_t, terminal_layer_impartial_t>&
             terminal_layer : _tree_impartial)
    {
        for (const std::pair<const hash_t, db_entry_impartial>& entry_pair :
             terminal_layer.second)
        {
            os << entry_pair << '\n';
        }
    }

    os << std::flush;

}

void database::dump_to_file(const std::string& out_filename) const
{
    std::ofstream of(out_filename);
    THROW_ASSERT(of.is_open());

    dump_to_stream(of);

    of.close();
}

/*
    function gen_main(GEN):
        for G in GEN:
            SUM = sum([G])
            make_entry(SUM)

    function make_entry(S):
        split_and_normalize(S)

        if has_entry(S):
            continue

        optional<sum> SUM2
        for SG of S:
            if !has_entry(SG): // check with _get_db_hash to avoid constructing a sum
                make_entry(SG) // using SUM2

        TG <- make_therm(S) // calls make_entry for children which don't have entries
        oc <- find_outcome_class(S)
        ...

    - `make_therm`-like function MUST always be called first (regardless of any
      CLI options which may disable DB fields in the future). All children must
      be checked i.e. because of thermographs, finding dominated moves, and due
      to how nogo has positions which only show up as a result of splitting (the
      game generator doesn't generate positions with safe stones)

    - TODO: why is DB search depth > 1 for some non-nogo games? (amazons,
      domineering?)
*/

/*
    New (better) algorithm:
    NOTE: This only seems to be slightly better, but also produces a different
        DB according to `compare_databases.h` (???)

    function gen_main(GEN):
        for G in GEN:
            SUM1 = sum([G])
            SUM1.split_and_normalize()

            if (SUM1.num_total_games() > 0):
                for Gi in SUM1:
                    SUM2 = sum([Gi])
                    gen_single(SUM2)

            # NOTE: The loop should count active games and this should be
            # skipped where appropriate
            gen_single(SUM1)

    function gen_single(SUM):
        if have_entry(SUM):
            return

        therm = make_thermograph(SUM)
        ...


    function make_thermograph(SUM):
        black_options = make_option_graphs(SUM, BLACK)
        white_options = make_option_graphs(SUM, WHITE)
        ...

    function make_option_graphs(SUM, PLAYER):
        MG = SUM.make_generator(PLAYER)

        N_ACTIVE_BEFORE = SUM.num_active_games()

        for M in MG:
            SUM.play(M)

            if N_ACTIVE_BEFORE == 1 and SUM.num_active_games() >= 2:
                for Gi in SUM:
                    gen_single(Gi)

            # Either returns the graph or calls gen_single(...)
            option = get_option_graph(SUM)

            SUM.undo(M)

*/
void database::generate_entries_partisan(i_db_game_generator& gen, bool silent)
{
    sumgame sum1(BLACK);
    sumgame sum2(BLACK);

    while (gen)
    {
        assert(sum1.num_total_games() == 0);

        unique_ptr<game> g(gen.gen_game());
        ++gen;

        sum1.add(g.get());

        sum1.split_and_normalize();
        const int n_games = sum1.num_total_games();

        int n_active = 0;
        for (int i = 0; i < n_games; i++)
        {
            game* gi = sum1.subgame(i);
            if (!gi->is_active())
                continue;

            n_active++;

            assert(sum2.num_total_games() == 0);
            sum2.add(gi);

            generate_entry_single_partisan(sum2, 0, silent);

            sum2.pop(gi);
        }

        if (n_active >= 2)
            generate_entry_single_partisan(sum1, 0, silent);

        sum1.undo_split_and_normalize();
        sum1.pop(g.get());
    }
}

void database::generate_entry_single_partisan(sumgame& sum, unsigned int depth,
                                               bool silent)
{
    _max_generation_depth = max(_max_generation_depth, depth);

    if (get_partisan_ptr(sum) != nullptr)
        return;

    assert_restore_sumgame ars(sum);
    const bw restore_player = sum.to_play();

    bool print_game = !silent && ((_game_count % 128) == 0);

    if (print_game)
    {
        cout << "Game # " << _game_count << ": ";
        _db_print_sum(cout, sum);
        cout << std::flush;
    }
    _game_count++;

    db_entry_partisan entry;

    {
        std::stringstream str;
        sum.print_sorted(str);
        entry.game_string = str.str();
    }

#ifdef MCGS_USE_THERM
    entry.thermograph = db_make_thermograph(*this, sum, depth, silent);
#endif

    sum.set_to_play(BLACK);
    bool black_wins = sum.solve();

    sum.set_to_play(WHITE);
    bool white_wins = sum.solve();

    entry.outcome = bools_to_outcome_class(black_wins, white_wins);

#ifdef MCGS_USE_BOUNDS
        entry.bounds_data = db_make_bounds(sum);

        // Debug info
        n_db_games++;
        {
            assert(entry.bounds_data);
            n_db_games_with_bounds++;

            const game_bounds& bounds = *entry.bounds_data;
            const bound_scale scale = bounds.get_scale();

            if (scale == BOUND_SCALE_DYADIC_RATIONAL)
                n_db_bounds_rational++;
            else if (scale == BOUND_SCALE_UP)
                n_db_bounds_infinitesimal++;

            if (bounds.both_valid() && bounds.get_lower_relation() == REL_EQUAL)
                n_db_bounds_equal++;
        }
#endif

#ifdef MCGS_USE_DOMINANCE
        entry.dominated_moves = db_make_dominated_moves(sum);
#endif

    set_partisan(sum, entry);

    sum.set_to_play(restore_player);

    if (print_game)
        cout << " DONE" << endl;
}

void database::generate_entry_single_partisan_impl(sumgame& sum,
                                                    unsigned int depth,
                                                    bool silent)
{
    assert(false);
}

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

game_type_t database::_get_sum_game_type(const sumgame& sum)
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

    if (sum_type.has_value())
        return *sum_type;
    return game_type<integer_game>(); // hack to allow empty sums
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
