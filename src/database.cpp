#include "database.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <memory>
#include <iostream>

#include "db_link_t.h"
#include "db_make_simplest_equal_game.h"
#include "global_options.h"
#include "serializer.h"
#include "serializer_lib_therm.h" // IWYU pragma: keep
#include "ThGraph.h"
#include "bounds.h"
#include "cgt_integer_game.h"
#include "db_make_bounds.h"
#include "db_make_dominated_moves.h"
#include "db_make_outcome_class.h"
#include "db_make_thermograph.h"
#include "game.h"
#include "impartial_sumgame.h"
#include "impartial_game.h"
#include "sumgame.h"
#include "iobuffer.h"
#include "db_game_generator.h"
#include "db_entry_serializers.h" // IWYU pragma: keep
#include "sumgame_helpers.h"
#include "thermograph_cache.h"
#include "throw_assert.h"
#include "type_table.h"
#include "clobber_1xn.h"
#include "utilities.h"
#include "version_info.h"
#include "impartial_game_wrapper.h"
#include "thermograph_helpers.h"

#ifdef DB_INCLUDE_STRINGS
#include "db_make_sum_string.h"
#endif

using namespace std;

////////////////////////////////////////////////// db_entry_partisan methods
bool db_entry_partisan::operator==(const db_entry_partisan& other) const
{
    // Note confusing use of `shared_ptr::operator bool()` in this function

#ifdef DB_INCLUDE_STRINGS
    if (sum_string != other.sum_string)
        return false;
#endif

    // Game type
    if (disk_game_type != other.disk_game_type)
        return false;

    // Outcome
    if (outcome != other.outcome)
        return false;

    // Thermograph
    if ((bool) thermograph != (bool) other.thermograph)
        return false;

    if (thermograph && !(*thermograph == *other.thermograph))
        return false;

    // Bounds
    if ((bool) bounds_data != (bool) other.bounds_data)
        return false;

    if (bounds_data && (*bounds_data != *other.bounds_data))
        return false;

    // Complexity
    if (complexity != other.complexity)
        return false;

    // Size score
    if (size_score != other.size_score)
        return false;

    // Dominated moves
    if ((bool) dominated_moves != (bool) other.dominated_moves)
        return false;

    if (dominated_moves && (*dominated_moves != *other.dominated_moves))
        return false;

    // Serialized sum
    if (serialized_sum != other.serialized_sum)
        return false;

    // Link
    if (!simplest_equal_entry.equal_as_pointers(other.simplest_equal_entry))
        return false;

    return true;
}

void db_entry_partisan::print(ostream& os, const database& db,
                              bool print_endl) const
{
#ifdef DB_INCLUDE_STRINGS
    os << "\"" << sum_string << "\" ";
#endif

    // Game type
    os << "Game type: `" << disk_game_type << "` ";

    // Outcome
    os << outcome_class_to_string(outcome);

    // Thermograph
    os << " Thermograph: `";

    if (thermograph.get() == nullptr)
        os << "nullptr";
    else
        print_thermograph(os, *thermograph);

    os << "`";

    // Bounds
    os << " Bounds: `";

    if (bounds_data.get() == nullptr)
        os << "nullptr";
    else
        os << *bounds_data;

    os << "`";

    // Complexity
    os << " Complexity: `" << complexity << "`";

    // Size score
    os << " Size score: `" << size_score << "`";

    // Dominated moves
    os << " Dominated moves: `";

    if (dominated_moves.get() == nullptr)
        os << "nullptr";
    else
        os << *dominated_moves;

    os << "`";

    // Serialized sum
    os << " Serialized sum: " << serialized_sum.size() << " bytes";

    // Simplest equal entry
    os << " SEG hash: `";
    if (simplest_equal_entry.get_as_pointer() == nullptr)
        os << "nullptr";
    else
        os << simplest_equal_entry.get_as_pointer()->first;
    os << "`";
    
    // Newline
    if (print_endl)
        os << endl;
}

void db_entry_partisan::save_sum(const sumgame& sum)
{
    vector<game*> active_games;

    const int n_games = sum.num_total_games();
    for (int i = 0; i < n_games; i++)
    {
        game* g = sum.subgame(i);
        if (g->is_active())
            active_games.push_back(g);
    }

    save_sum(active_games);
}

void db_entry_partisan::load_sum(sumgame& sum) const
{
    vector<game*> games = load_sum();

    for (game* g : games)
    {
        assert(g->is_active());
        sum.add(g);
    }
}

void db_entry_partisan::save_sum(const vector<game*>& games)
{
    assert(serialized_sum.empty());

    memory_obuffer os;
    serializer<vector<game*>>::save(os, games, nullptr);

    serialized_sum = std::move(os.get_data());
}

vector<game*> db_entry_partisan::load_sum() const
{
    memory_ibuffer is(serialized_sum);
    return serializer<vector<game*>>::load(is, nullptr);
}

////////////////////////////////////////////////// database methods
database::database()
    : _n_entries_generated(0), _graph_cache(make_unique<thermograph_cache>())
{
}

void database::save(const string& filename) const
{
    file_obuffer os(filename);
    serializer_ctx ctx;

    serializer_save(os, _metadata_string, &ctx);
    serializer_save(os, _mapper, &ctx);
    serializer_save(os, _graph_cache, &ctx);
    serializer_save(os, _max_size_scores, &ctx);

    // Thermographs in the DB entry are saved to disk as `thgraph_id_t`
    serializer<shared_ptr<ThGraph>>::set_thermograph_cache(&ctx,
                                                           &_get_graph_cache());
    serializer_save(os, _terminal_partisan, &ctx);
    serializer_save(os, _tree_impartial, &ctx);

    os.close();
}

void database::load(const string& filename)
{
    assert(_terminal_partisan.empty());
    assert(_tree_impartial.empty());

    file_ibuffer is(filename);
    serializer_ctx ctx;

    serializer_load(is, _metadata_string, &ctx);
    serializer_load(is, _mapper, &ctx);
    serializer_load(is, _graph_cache, &ctx);
    serializer_load(is, _max_size_scores, &ctx);

    // Thermographs in the DB entry are saved to disk as `thgraph_id_t`
    serializer<shared_ptr<ThGraph>>::set_thermograph_cache(&ctx,
                                                           &_get_graph_cache());
    serializer_load(is, _terminal_partisan, &ctx);
    serializer_load(is, _tree_impartial, &ctx);

    _convert_links_to_pointers();

    is.close();
}

void database::dump_to_stream(ostream& os) const
{
    os << _mapper << '\n';

    for (const pair<const hash_t, db_entry_partisan>& entry_pair :
         _terminal_partisan)
    {
        os << "Hash: `";
        os << entry_pair.first;
        os << "` ";

        entry_pair.second.print(os, *this);
        os << '\n';
    }

    for (const pair<const game_type_t, terminal_layer_impartial_t>&
             terminal_layer : _tree_impartial)
    {
        for (const pair<const hash_t, db_entry_impartial>& entry_pair :
             terminal_layer.second)
        {
            os << "Hash: `";
            os << entry_pair.first;
            os << "` ";

            os << entry_pair.second << '\n';
        }
    }

    os << flush;
}

void database::dump_to_file(const string& out_filename) const
{
    ofstream of(out_filename);
    THROW_ASSERT(of.is_open());

    dump_to_stream(of);

    of.close();
}

const db_entry_partisan* database::get_partisan_ptr(const game& g) const
{
    pair<const hash_t, db_entry_partisan>* ptr = _get_partisan_impl(g);

    if (ptr == nullptr)
        return nullptr;

    return &ptr->second;
}

db_entry_partisan* database::get_partisan_ptr(const game& g)
{
    pair<const hash_t, db_entry_partisan>* ptr = _get_partisan_impl(g);

    if (ptr == nullptr)
        return nullptr;

    return &ptr->second;
}

db_entry_partisan* database::get_or_allocate_partisan_ptr(const game& g)
{
    pair<const hash_t, db_entry_partisan>* ptr = _get_or_allocate_partisan_impl(g);
    assert(ptr != nullptr);
    return &ptr->second;
}

const db_entry_partisan* database::get_partisan_ptr(const sumgame& sum) const
{
    pair<const hash_t, db_entry_partisan>* ptr = _get_partisan_impl(sum);

    if (ptr == nullptr)
        return nullptr;

    return &ptr->second;
}

db_entry_partisan* database::get_partisan_ptr(const sumgame& sum)
{
    pair<const hash_t, db_entry_partisan>* ptr = _get_partisan_impl(sum);

    if (ptr == nullptr)
        return nullptr;

    return &ptr->second;
}

db_entry_partisan* database::get_or_allocate_partisan_ptr(const sumgame& sum)
{
    pair<const hash_t, db_entry_partisan>* ptr = _get_or_allocate_partisan_impl(sum);
    assert(ptr != nullptr);
    return &ptr->second;
}


db_entry_partisan* database::get_partisan_ptr(const db_link_t& link)
{
    pair<const hash_t, db_entry_partisan>* pair_ptr = get_partisan_ptr_pair(link);

    if (pair_ptr == nullptr)
        return nullptr;

    return &pair_ptr->second;
}

pair<const hash_t, db_entry_partisan>* database::get_partisan_ptr_pair(
    const sumgame& sum)
{
    return _get_partisan_impl(sum);
}

pair<const hash_t, db_entry_partisan>* database::get_partisan_ptr_pair(
    const game& g)
{
    return _get_partisan_impl(g);
}

pair<const hash_t, db_entry_partisan>* database::get_partisan_ptr_pair(
    hash_t hash)
{
    auto pair_iterator = _terminal_partisan.find(hash);
    if (pair_iterator == _terminal_partisan.end())
        return nullptr;

    pair<const hash_t, db_entry_partisan>& p = *pair_iterator;
    return &p;
}

pair<const hash_t, db_entry_partisan>* database::get_partisan_ptr_pair(
    const db_link_t& link)
{
    return link.get_as_pointer();
}

db_link_t database::get_partisan_link(const sumgame& sum)
{
    pair<const hash_t, db_entry_partisan>* ptr = get_partisan_ptr_pair(sum);
    THROW_ASSERT(ptr != nullptr);

    db_link_t link;
    link.set_as_pointer(ptr);
    return link;
}

db_link_t database::get_partisan_link(std::pair<const hash_t, db_entry_partisan>* ptr)
{
    THROW_ASSERT(ptr != nullptr);

    db_link_t link;
    link.set_as_pointer(ptr);
    return link;
}

optional<db_entry_impartial> database::get_impartial(const game& g) const
{
    const game_type_t runtime_type = _get_game_db_type(g);

    const game_type_t disk_type = _mapper.translate_type(runtime_type);
    if (disk_type == 0)
        return {};

    auto terminal_layer_iterator = _tree_impartial.find(disk_type);
    if (terminal_layer_iterator == _tree_impartial.end())
        return {};

    const terminal_layer_impartial_t& layer = terminal_layer_iterator->second;

    const hash_t hash = get_db_hash(g);

    const auto entry_iterator = layer.find(hash);
    if (entry_iterator == layer.end())
        return {};

    const db_entry_impartial& entry = entry_iterator->second;

    return entry;
}

void database::set_impartial(const game& g, const db_entry_impartial& entry)
{
    const game_type_t runtime_type = _get_game_db_type(g);

    const game_type_t disk_type = _mapper.translate_type(runtime_type);
    THROW_ASSERT(disk_type > 0); // game type not registered!

    terminal_layer_impartial_t& layer = _tree_impartial[disk_type];

    const hash_t hash = get_db_hash(g);
    const auto inserted_entry_iterator = layer.emplace(hash, entry);
    THROW_ASSERT(inserted_entry_iterator.second); // not already found
}

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

            generate_single_partisan_entry(sum2, silent);

            sum2.pop(gi);
        }

        if (n_active >= 2)
            generate_single_partisan_entry(sum1, silent);

        sum1.undo_split_and_normalize();
        sum1.pop(g.get());
    }
}

void database::generate_single_partisan_entry(sumgame& sum, bool silent)
{
    db_entry_partisan* entry = get_or_allocate_partisan_ptr(sum);
    assert(entry != nullptr);

    /*
        TODO `get_or_allocate...` should instead report whether or not the
        entry was just allocated, to avoid generating an entry twice. For now
        we catch this by checking that `entry->thermograph` is nullptr after
        generating the thermograph, but immediately before storing it in the
        entry.
    */
    if (entry->thermograph)
        return;

    assert_restore_sumgame ars(sum);
    const bw restore_player = sum.to_play();

    const bool print_game = !silent && ((_n_entries_generated % 128) == 0);

    if (print_game)
    {
        cout << "Game # " << _n_entries_generated << ": ";
        _db_print_sum(cout, sum);
        cout << flush;
    }
    _n_entries_generated++;

    // Serialized sum
    entry->save_sum(sum);

    // Disk game type
    {
        const game_type_t runtime_type = _get_sum_db_type(sum);
        const game_type_t disk_type = _mapper.translate_type(runtime_type);
        THROW_ASSERT(disk_type != 0);
        entry->disk_game_type = disk_type;
    }

    // Thermograph
    {
        ThGraph* graph = db_make_thermograph(*this, sum, silent);
        graph->Check();

        assert(!entry->thermograph); // Ensure we don't generate the entry twice

        // `graph` is invalidated after next line
        entry->thermograph = _get_graph_cache().insert_and_release(graph);
        assert(entry->thermograph);
    }

#ifdef DB_INCLUDE_STRINGS
    // Sum string
    entry->sum_string = db_make_sum_string(sum);
#endif

    // Outcome
    entry->outcome = db_make_outcome_class(*this, *entry);
    assert(entry->outcome != outcome_class::U);

    // Bounds
    entry->bounds_data = db_make_bounds(*this, sum, *entry);
    assert(entry->bounds_data && entry->bounds_data->both_valid());

    // Dominated moves, complexity
    db_make_dominated_moves(sum, *entry, *this);
    assert(entry->dominated_moves);

    // Size score, simplest equal entry
    db_make_simplest_equal_game(sum, *entry, *this);

    sum.set_to_play(restore_player);

    if (print_game)
        cout << " DONE" << endl;
}

void database::generate_entries_impartial(i_db_game_generator& gen, bool silent)
{
    while (gen)
    {
        /*
           g can be partisan OR impartial. If partisan, we must wrap it
           in an impartial_game_wrapper
        */
        unique_ptr<game> g(gen.gen_game());
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

                _generate_single_impartial_entry(sg_impartial, silent);
                delete sg;
            }

            continue;
        }

        // Normalize, handle ig
        ig->normalize();
        _generate_single_impartial_entry(ig, silent);
    }
}

void database::refine_partisan_links()
{
    for (pair<const hash_t, db_entry_partisan>& entry_pair : _terminal_partisan)
        db_refine_simplest_equal_game(entry_pair, *this);

    cout << db_n_links_refined << "/" << _terminal_partisan.size() << " ";
    cout << " links refined (";
    cout << db_n_links_refined / static_cast<double>(_terminal_partisan.size());
    cout << ")" << endl;
}

void database::report_size_score(game_type_t disk_type, uint64_t size_score)
{
    assert(disk_type > 0);
    if (disk_type >= _max_size_scores.size())
        _max_size_scores.resize(disk_type + 1, 0);

    uint64_t& max_score = _max_size_scores[disk_type];
    max_score = max(max_score, size_score);
}

uint64_t database::get_max_size_score(game_type_t disk_type) const
{
    assert(disk_type > 0);
    if (disk_type >= _max_size_scores.size())
        return 0;

    return _max_size_scores[disk_type];
}

void database::clear()
{
    _metadata_string.clear();
    _mapper.clear();
    _graph_cache = make_unique<thermograph_cache>();
    _max_size_scores.clear();
    _terminal_partisan.clear();
    _tree_impartial.clear();
}

bool database::empty() const
{
    return _terminal_partisan.empty() && _tree_impartial.empty();
}

bool database::is_equal(const database& other) const
{
    if (_mapper != other._mapper)
        return false;

    if (_get_graph_cache() != other._get_graph_cache())
        return false;

    if (_terminal_partisan != other._terminal_partisan)
        return false;

    if (_tree_impartial != other._tree_impartial)
        return false;

    return true;
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
    _metadata_string +=
        "Date created: \"" + get_current_time_as_string() + "\"\n";

    // Game config
    _metadata_string += "DB config string: \"" + config_string + "\"";
}

hash_t database::get_db_hash(const game& g, global_hash& gh)
{
    return gh.get_global_hash_value(&g, EMPTY);
}

hash_t database::get_db_hash(const sumgame& sum)
{
    return sum.get_global_hash_for_player(EMPTY);
}

hash_t database::get_db_hash(const game& g) const
{
    global_hash& gh = _get_global_hash();
    return get_db_hash(g, gh);
}

#define PRINT_FRAC(val1, val2)                                                 \
    do                                                                         \
    {                                                                          \
        cout << val1 << "/" << val2;                                           \
        if (val2 > 0)                                                          \
            cout << " (" << val1 / static_cast<double>(val2) << ")";           \
    } while (0)

void database::assert_links_equal()
{
    cout << "Disabling `global::use_seg`" << endl;
    global::use_seg.set(false);
    sumgame sum(BLACK);

    uint64_t n_entries_with_links = 0;

    uint64_t n_singles = 0;
    uint64_t n_singles_with_links = 0;

    uint64_t n_sums = 0;
    uint64_t n_sums_with_links = 0;

    uint64_t n_non0_with_link = 0;
    uint64_t n_non0_with_link_differing_types = 0;

    for (const pair<const hash_t, db_entry_partisan>& entry_pair :
         _terminal_partisan)
    {
        const hash_t entry_hash = entry_pair.first;
        const db_entry_partisan& entry = entry_pair.second;

        assert(sum.num_total_games() == 0);

        vector<game*> entry_games = entry.load_sum();
        if (entry_games.size() == 1)
            n_singles++;
        if (entry_games.size() > 1)
            n_sums++;

        sum.add(entry_games);

        cout << "Validating: ";
        sum.print_simple(cout);
        cout << endl;

        assert(entry_hash == database::get_db_hash(sum));

        db_entry_partisan* linked_entry = get_partisan_ptr(entry.simplest_equal_entry);
        if (linked_entry != nullptr && linked_entry != &entry)
        {
            n_entries_with_links++;

            if (entry_games.size() == 1)
                n_singles_with_links++;
            if (entry_games.size() > 1)
                n_sums_with_links++;

            if (entry.outcome != outcome_class::P)
            {
                n_non0_with_link++;
                if (entry.disk_game_type != linked_entry->disk_game_type)
                    n_non0_with_link_differing_types++;
            }


            vector<game*> linked_games = linked_entry->load_sum();

            vector<game*> inverse_linked_games;
            for (game* g : linked_games)
                inverse_linked_games.push_back(g->inverse());

            sum.add(inverse_linked_games);
            assert(sum_rel_zero(sum, REL_EQUAL));
            sum.pop(inverse_linked_games);

            for (game* g: linked_games)
                delete g;
            for (game* g: inverse_linked_games)
                delete g;
        }

        sum.pop(entry_games);
        for (game* g : entry_games)
            delete g;

    }

    cout << "Serialized sums and entry links OK" << endl;

    PRINT_FRAC(n_entries_with_links, _terminal_partisan.size());
    cout << " entries with links" << endl;

    PRINT_FRAC(n_singles_with_links, n_singles);
    cout << " singles with links" << endl;

    PRINT_FRAC(n_sums_with_links, n_sums);
    cout << " sums with links" << endl;

    PRINT_FRAC(n_non0_with_link_differing_types, n_non0_with_link);
    cout << " # non0 w/ link and differing type / # non0 w/ link" << endl;


    assert(sum.num_total_games() == 0);
}

void database::register_type(const string& type_name,
                                    game_type_t runtime_type)
{
    _mapper.register_type(type_name, runtime_type);
}

void database::__register_built_in_types()
{
    DATABASE_REGISTER_TYPE((*this), integer_game);
}

sumgame& database::_get_sumgame()
{
    if (_sum.get() == nullptr)
        _sum.reset(new sumgame(BLACK));

    return *_sum;
}

global_hash& database::_get_global_hash() const
{
    if (_global_hash.get() == nullptr)
        _global_hash.reset(new global_hash());

    return *_global_hash;
}

thermograph_cache& database::_get_graph_cache() const
{
    // Should be created by database constructor
    assert(_graph_cache.get() != nullptr);
    return *_graph_cache;
}

void database::_generate_single_impartial_entry(impartial_game* ig, bool silent)
{
    if (get_impartial(*ig).has_value())
        return;

    const bool print_game = !silent && ((_n_entries_generated % 128) == 0);

    if (print_game)
        cout << "Game # " << _n_entries_generated << ": " << *ig << flush;
    _n_entries_generated++;

    sumgame& s = _get_sumgame();
    assert(s.num_total_games() == 0);

    s.add(ig);

    const int nim_value = search_impartial_sumgame(s);
    assert(nim_value >= 0);

    s.pop(ig);

    db_entry_impartial entry;
#ifdef DB_INCLUDE_STRINGS
    entry.sum_string = ig->to_string();
#endif
    entry.nim_value = nim_value;

    set_impartial(*ig, entry);
    assert(s.num_total_games() == 0);

    if (print_game)
        cout << " DONE" << endl;
}

void database::_convert_links_to_pointers()
{
    THROW_ASSERT(get_partisan_ptr_pair(hash_t(0)) == nullptr);

    for (pair<const hash_t, db_entry_partisan>& entry_pair : _terminal_partisan)
    {
        db_entry_partisan& entry = entry_pair.second;

        const hash_t link_hash = entry.simplest_equal_entry.get_as_hash();
        pair<const hash_t, db_entry_partisan>* link_ptr = nullptr;

        if (link_hash != 0)
            link_ptr = get_partisan_ptr_pair(link_hash);

        entry.simplest_equal_entry.set_as_pointer(link_ptr);
    }
}

game_type_t database::_get_sum_db_type(const sumgame& sum)
{
    optional<game_type_t> sum_type;

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

game_type_t database::_get_game_db_type(const game& g)
{
    return g.game_type();
}

void database::_db_print_sum(ostream& os, const sumgame& sum)
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

template <class Game_Or_Sum_T>
pair<const hash_t, db_entry_partisan>* database::_get_partisan_impl(
    const Game_Or_Sum_T& g) const
{
    static_assert(is_same_v<game, Game_Or_Sum_T> ||
                  is_same_v<sumgame, Game_Or_Sum_T>);

    constexpr bool IS_SUM = is_same_v<sumgame, Game_Or_Sum_T>;

    game_type_t runtime_type;
    if constexpr (IS_SUM)
        runtime_type = _get_sum_db_type(g);
    else
        runtime_type = _get_game_db_type(g);

    const game_type_t disk_type = _mapper.translate_type(runtime_type);
    if (disk_type == 0)
        return nullptr;

    const hash_t hash = get_db_hash(g);

    auto entry_iterator = _terminal_partisan.find(hash);
    if (entry_iterator == _terminal_partisan.end())
        return nullptr;

    const pair<const hash_t, db_entry_partisan>& p = *entry_iterator;

    pair<const hash_t, db_entry_partisan>& p_nonconst =
        const_cast<pair<const hash_t, db_entry_partisan>&>(p);

    return &p_nonconst;
}

template <class Game_Or_Sum_T>
pair<const hash_t, db_entry_partisan>* database::_get_or_allocate_partisan_impl(
    const Game_Or_Sum_T& g)
{
    static_assert(is_same_v<game, Game_Or_Sum_T> ||
                  is_same_v<sumgame, Game_Or_Sum_T>);

    constexpr bool IS_SUM = is_same_v<sumgame, Game_Or_Sum_T>;

    game_type_t runtime_type;
    if constexpr (IS_SUM)
        runtime_type = _get_sum_db_type(g);
    else
        runtime_type = _get_game_db_type(g);

    const game_type_t disk_type = _mapper.translate_type(runtime_type);
    THROW_ASSERT(disk_type > 0); // game type not registered!

    const hash_t hash = get_db_hash(g);

#warning TODO ensure this doesn't default construct when already present!!!
    auto entry_iterator = _terminal_partisan.try_emplace(hash);

    pair<const hash_t, db_entry_partisan>& p = *entry_iterator.first;

    return &p;
}

ostream& operator<<(ostream& os, const database& db)
{
    const unordered_map<game_type_t, string>& disk_type_to_name_map =
        db._mapper.get_disk_type_to_name_map();

#warning TODO printing partisan DB stuff!
    //os << "# of Partisan game types: " << db._tree_partisan.size() << '\n';
    os << "# of Impartial game types: " << db._tree_impartial.size() << '\n';

    //for (const pair<const game_type_t, database::terminal_layer_partisan_t>& p :
    //     db._tree_partisan)
    //{
    //    const game_type_t disk_type = p.first;
    //    const database::terminal_layer_partisan_t& layer = p.second;

    //    auto it = disk_type_to_name_map.find(disk_type);
    //    THROW_ASSERT(it != disk_type_to_name_map.end());

    //    const string& game_name = it->second;

    //    os << "\tGame type: \"" << game_name << "\" ";
    //    os << "Count: " << layer.size() << '\n';
    //}

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

