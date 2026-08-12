#include "database_test.h"

#include <cstring>
#include <functional>
#include <memory>
#include <sstream>
#include <unordered_set>
#include <set>
#include <cstdint>
#include <string>
#include <cassert>
#include <unordered_map>
#include <vector>
#include <utility>
#include <algorithm>

#include "bounds.h"
#include "cgt_basics.h"
#include "clobber_1xn.h"
#include "db_game_generator.h"
#include "db_link_t.h"
#include "dominated_moves.h"
#include "game.h"
#include "grid_generator.h"
#include "gridlike_db_game_generator.h"
#include "sumgame.h"
#include "nogo_1xn.h"
#include "all_game_headers.h"
#include "database.h"
#include "sumgame_helpers.h"
#include "thermograph_builder_no_db.h"
#include "ThGraph.h"
#include "utilities.h"


template <>
struct std::hash<std::pair<uint64_t, uint64_t>>
{
    size_t operator()(const std::pair<uint64_t, uint64_t>& p) const noexcept
    {
        return p.first ^ p.second;
    }
};


namespace {

} // namespace

using namespace std;


////////////////////////////////////////////////// Helpers
namespace {

set<::move> get_subgame_move_set(const game& g, bw player)
{
    assert(is_black_white(player));
    set<::move> g_moves;

    unique_ptr<move_generator> gen(g.create_move_generator(player));
    while (*gen)
    {
        const ::move m = gen->gen_move();
        ++(*gen);
        g_moves.insert(m);
    }

    return g_moves;
}

void check_dom_moves_subgame(const game& g, bw player, const db_dom_moves_t& dom_obj)
{
    assert(is_black_white(player));
    assert(dom_obj.get_kind() == DB_DOM_MOVES_KIND_DOMINATED);

    const set<::move>* dom = dom_obj.get_dominated_moves(g.get_local_hash(), player);
    if (dom == nullptr)
        return;

    set<::move> g_moves = get_subgame_move_set(g, player);

    for (const ::move m_enc : *dom)
    {
        const ::move m_dec = g.decode_grid_move_from_db(m_enc);
        assert(g_moves.find(m_dec) != g_moves.end());
    }
}

void check_nondom_moves_subgame(const game& g, bw player, const db_dom_moves_t& dom_obj)
{
    assert(is_black_white(player));
    assert(dom_obj.get_kind() == DB_DOM_MOVES_KIND_NONDOMINATED);

    const vector<::move>* nondom = dom_obj.get_nondominated_moves(g.get_local_hash(), player);
    if (nondom == nullptr)
        return;

    set<::move> g_moves = get_subgame_move_set(g, player);

    for (const ::move m_enc : *nondom)
    {
        const ::move m_dec = g.decode_grid_move_from_db(m_enc);
        assert(g_moves.find(m_dec) != g_moves.end());
    }
}

void check_dom_obj(sumgame& sum, const db_dom_moves_t& dom_obj)
{
    const db_dom_moves_kind kind = dom_obj.get_kind();
    assert(kind != DB_DOM_MOVES_KIND_NONE);

    const int n_games = sum.num_total_games();
    for (int i = 0; i < n_games; i++)
    {
        const game* g = sum.subgame_const(i);
        if (!g->is_active())
            continue;

        switch (kind)
        {
            case DB_DOM_MOVES_KIND_NONE:
                assert(false);
            case DB_DOM_MOVES_KIND_DOMINATED:
            {
                check_dom_moves_subgame(*g, BLACK, dom_obj);
                check_dom_moves_subgame(*g, WHITE, dom_obj);
                break;
            }
            case DB_DOM_MOVES_KIND_NONDOMINATED:
            {
                check_nondom_moves_subgame(*g, BLACK, dom_obj);
                check_nondom_moves_subgame(*g, WHITE, dom_obj);
                break;
            }
        }
    }
}

void check_entry_contents(sumgame& sum,
                          const db_entry_partisan* entry,
                          thermograph_builder_no_db& thermograph_builder)
{
    assert(entry != nullptr);

    // Outcome
    assert(entry->outcome == get_sum_outcome(sum));

    // Thermograph
    assert(entry->thermograph.get() != nullptr);
    shared_ptr<const ThGraph> graph_nodb = thermograph_builder.build_thermograph(sum);
    assert(*graph_nodb == *entry->thermograph);

    // Bounds
    assert(entry->bounds_data.get() != nullptr);
    const game_bounds& bounds = *entry->bounds_data;
    assert(bounds.both_valid());
    const bound_scale scale = bounds.get_scale();
    const bound_t lower = bounds.get_lower();
    const bound_t upper = bounds.get_upper();

    assert(sum_rel_scale_game(sum, flip_relation(bounds.get_lower_relation()), scale, lower));
    assert(sum_rel_scale_game(sum, flip_relation(bounds.get_upper_relation()), scale, upper)); 
    assert(sum_rel_scale_game(sum, REL_GREATER, scale, lower - 1));
    assert(sum_rel_scale_game(sum, REL_LESS, scale, upper + 1));

    // Dominated moves (TODO how???)
    // Complexity (TODO how???)

    assert(entry->dominated_moves.get() != nullptr);
    const db_dom_moves_t& dom_obj = *entry->dominated_moves;
    check_dom_obj(sum, dom_obj);

    // serialized_sum and subgame_links
    vector<hash_t> subgame_hashes;
    vector<hash_t> deserialized_subgame_hashes;
    vector<hash_t> linked_entry_hashes;

    const int n_games = sum.num_total_games();
    for (int i = 0; i < n_games; i++)
    {
        game* g = sum.subgame(i);
        if (!g->is_active())
            continue;
        subgame_hashes.push_back(g->get_local_hash());
    }

    {
        vector<game*> deserialized_games = entry->load_sum();
        for (game* g : deserialized_games)
        {
            deserialized_subgame_hashes.push_back(g->get_local_hash());
            delete g;
        }
    }

    for (const db_link_t& link : entry->subgame_links)
    {
        assert(!link.is_nullptr());
        linked_entry_hashes.push_back(link.get_as_pointer()->first);
    }

    std::sort(subgame_hashes.begin(), subgame_hashes.end(),
              std::less<hash_t>());
    std::sort(linked_entry_hashes.begin(), linked_entry_hashes.end(),
              std::less<hash_t>());
    std::sort(deserialized_subgame_hashes.begin(),
              deserialized_subgame_hashes.end(), std::less<hash_t>());

    assert(subgame_hashes == deserialized_subgame_hashes);
    assert(deserialized_subgame_hashes == linked_entry_hashes);

}

void test_generate_impl(database& db, i_db_game_generator* gen_generate,
                        i_db_game_generator* gen_generate_copy,
                        i_db_game_generator* gen_validate)
{

    assert(gen_generate != gen_generate_copy);

    unordered_set<hash_t> expected_hashes;
    sumgame sum(BLACK);

    while (*gen_generate_copy)
    {
        game* g = gen_generate_copy->gen_game();
        ++(*gen_generate_copy);

        assert(sum.num_total_games() == 0);
        sum.add(g);
        sum.split_and_normalize();

        const hash_t hash = database::get_db_hash(sum);
        expected_hashes.insert(hash);

        sum.undo_split_and_normalize();
        sum.pop(g);
        delete g;
    }

    db_gen_options_t gen_options;
    gen_options.silent = true;
    db.generate_entries_partisan(*gen_generate, gen_options);

    thermograph_builder_no_db thermograph_builder;
    while (*gen_validate)
    {
        game* g = gen_validate->gen_game();
        ++(*gen_validate);

        assert(sum.num_total_games() == 0);
        sum.add(g);
        sum.split_and_normalize();

        const hash_t hash = database::get_db_hash(sum);
        const bool entry_is_expected = (expected_hashes.find(hash) != expected_hashes.end());

        const db_entry_partisan* entry = db.get_partisan_ptr(sum);
        assert(logical_iff(entry_is_expected, entry != nullptr));

        if (entry != nullptr)
            check_entry_contents(sum, entry, thermograph_builder);

        sum.undo_split_and_normalize();
        sum.pop(g);
        delete g;
    }
    db.assert_links_equal(true);

    delete gen_generate;
    delete gen_generate_copy;
    delete gen_validate;
}

i_db_game_generator* make_clobber_1xn_generator(int max_len)
{
    grid_generator* gg =
        new grid_generator(int_pair(1, max_len), {EMPTY, BLACK, WHITE}, true);

    return new gridlike_db_game_generator<clobber_1xn, GRIDLIKE_TYPE_STRIP>(gg);
}

i_db_game_generator* make_nogo_1xn_generator(int max_len)
{
    grid_generator* gg =
        new grid_generator(int_pair(1, max_len), {EMPTY, BLACK, WHITE}, true);

    return new gridlike_db_game_generator<nogo_1xn, GRIDLIKE_TYPE_STRIP>(gg);
}


i_db_game_generator* make_domineering_generator(int max_r, int max_c)
{
    grid_generator* gg =
        new grid_generator(int_pair(max_r, max_c), {EMPTY, BORDER}, false);

    return new gridlike_db_game_generator<domineering, GRIDLIKE_TYPE_GRID>(gg);
}


void test_db_hash_impl(i_db_game_generator& gen,
                       unordered_map<hash_t, string>& game_hashes)
{
    sumgame sum(BLACK);
    sumgame sum_single(BLACK);
    global_hash gh;

    auto check_no_hash_collision = [&](const sumgame& sum,
                                       hash_t db_hash) -> void
    {
        stringstream strstr;
        sum.print_sorted(strstr);
        const string sum_string = strstr.str();

        const auto inserted = game_hashes.try_emplace(db_hash, sum_string);

        // No hash collision
        assert(inserted.second || inserted.first->second == sum_string);
    };

    vector<game*> active_games;
    vector<hash_t> active_hashes;

    vector<game*> active_game_single;
    vector<hash_t> active_hash_single;

    while (gen)
    {
        active_games.clear();
        active_hashes.clear();

        game* g = gen.gen_game();
        ++(gen);

        assert(sum.num_total_games() == 0);
        sum.add(g);
        sum.split_and_normalize();

        const int n_games = sum.num_total_games();
        for (int i = 0; i < n_games; i++)
        {
            game* g = sum.subgame(i);
            if (!g->is_active())
                continue;

            active_games.push_back(g);
            active_hashes.push_back(g->get_local_hash());
        }

        const hash_t sum_hash1 = sum.get_db_hash();
        const hash_t sum_hash2 = database::get_db_hash(sum);
        const hash_t sum_hash3 = gh.get_db_hash_value(active_games);
        const hash_t sum_hash4 = gh.get_db_hash_value(active_hashes);

        assert(sum_hash1 == sum_hash2 && //
               sum_hash2 == sum_hash3 && //
               sum_hash3 == sum_hash4    //
        );

        check_no_hash_collision(sum, sum_hash1);

        for (int i = 0; i < n_games; i++)
        {
            game* g = sum.subgame(i);
            if (!g->is_active())
                continue;

            active_game_single.clear();
            active_hash_single.clear();
            assert(sum_single.num_total_games() == 0);

            active_game_single.push_back(g);
            active_hash_single.push_back(g->get_local_hash());
            sum_single.add(g);

            const hash_t single_hash1 = g->get_local_hash();
            const hash_t single_hash2 = sum_single.get_db_hash();
            const hash_t single_hash3 = database::get_db_hash(sum_single);
            const hash_t single_hash4 = database::get_db_hash(*g);
            const hash_t single_hash5 = gh.get_db_hash_value(active_game_single);
            const hash_t single_hash6 = gh.get_db_hash_value(active_hash_single);

            assert(single_hash1 == single_hash2 && //
                   single_hash2 == single_hash3 && //
                   single_hash3 == single_hash4 && //
                   single_hash4 == single_hash5 && //
                   single_hash5 == single_hash6    //
            );

            check_no_hash_collision(sum_single, single_hash1);

            sum_single.pop(g);
        }

        sum.undo_split_and_normalize();
        sum.pop(g);

        delete g;
    }
}

////////////////////////////////////////////////// Main test functions
void test_db_hash()
{
    unordered_map<hash_t, string> db_hash_to_sum_string;

    vector<i_db_game_generator*> generators
    {
        make_clobber_1xn_generator(7),
        make_nogo_1xn_generator(7),
    };

    for (i_db_game_generator* gen : generators)
    {
        test_db_hash_impl(*gen, db_hash_to_sum_string);
        delete gen;
    }
}

// set/get, clear, empty
void test_basic()
{
    database db;
    db.__register_built_in_types();
    DATABASE_REGISTER_TYPE(db, clobber_1xn);
    DATABASE_REGISTER_TYPE(db, nogo_1xn);

    // Should be empty
    assert(db.empty());

    /*
        Entries get assigned: L, L, N, P
    */
    clobber_1xn g1("XX.O..");
    clobber_1xn g2("XX.O..");
    clobber_1xn g3("XO");
    nogo_1xn g4("XX.O..");

    // Can't find these entries
    assert(db.get_partisan_ptr(g1) == nullptr);
    assert(db.get_partisan_ptr(g2) == nullptr);
    assert(db.get_partisan_ptr(g3) == nullptr);
    assert(db.get_partisan_ptr(g4) == nullptr);

    // Insert entry for g1, should be available for g1 and g2
    {
        db_entry_partisan* entry = db.get_or_allocate_partisan_ptr(g1);
        assert(entry != nullptr && entry->outcome == outcome_class::U);
        entry->outcome = outcome_class::L;

        assert(!db.empty());

        const db_entry_partisan* query1 = db.get_partisan_ptr(g1);
        const db_entry_partisan* query2 = db.get_partisan_ptr(g2);
        const db_entry_partisan* query3 = db.get_partisan_ptr(g3);
        const db_entry_partisan* query4 = db.get_partisan_ptr(g4);

        assert(!db.empty());
        assert(query1 != nullptr && query1->outcome == outcome_class::L);
        assert(query2 != nullptr && query2->outcome == outcome_class::L);
        assert(query3 == nullptr && query4 == nullptr);

        assert(query1 == query2);
    }

    // Now insert another entry for g3
    {
        db_entry_partisan* entry = db.get_or_allocate_partisan_ptr(g3);
        assert(entry != nullptr && entry->outcome == outcome_class::U);
        entry->outcome = outcome_class::N;

        assert(!db.empty());

        const db_entry_partisan* query1 = db.get_partisan_ptr(g1);
        const db_entry_partisan* query2 = db.get_partisan_ptr(g2);
        const db_entry_partisan* query3 = db.get_partisan_ptr(g3);
        const db_entry_partisan* query4 = db.get_partisan_ptr(g4);

        assert(!db.empty());
        assert(query1 != nullptr && query1->outcome == outcome_class::L);
        assert(query2 != nullptr && query2->outcome == outcome_class::L);
        assert(query3 != nullptr && query3->outcome == outcome_class::N);
        assert(query4 == nullptr);

        assert(query1 == query2);
    }

    // Insert entry for same board but different game
    {
        db_entry_partisan* entry = db.get_or_allocate_partisan_ptr(g4);
        assert(entry != nullptr && entry->outcome == outcome_class::U);
        entry->outcome = outcome_class::P;

        assert(!db.empty());

        const db_entry_partisan* query1 = db.get_partisan_ptr(g1);
        const db_entry_partisan* query2 = db.get_partisan_ptr(g2);
        const db_entry_partisan* query3 = db.get_partisan_ptr(g3);
        const db_entry_partisan* query4 = db.get_partisan_ptr(g4);

        assert(!db.empty());
        assert(query1 != nullptr && query1->outcome == outcome_class::L);
        assert(query2 != nullptr && query2->outcome == outcome_class::L);
        assert(query3 != nullptr && query3->outcome == outcome_class::N);
        assert(query4 != nullptr && query4->outcome == outcome_class::P);

        assert(query1 == query2);
    }

    db.clear();
    assert(db.empty());

    const db_entry_partisan* query1 = db.get_partisan_ptr(g1);
    const db_entry_partisan* query2 = db.get_partisan_ptr(g2);
    const db_entry_partisan* query3 = db.get_partisan_ptr(g3);
    const db_entry_partisan* query4 = db.get_partisan_ptr(g4);

    assert(query1 == nullptr);
    assert(query2 == nullptr);
    assert(query3 == nullptr);
    assert(query4 == nullptr);
}

void test_query_game_and_sum()
{
    database db;
    db.__register_built_in_types();
    DATABASE_REGISTER_TYPE(db, clobber_1xn);

    clobber_1xn clob("XOXO");

    db_entry_partisan* entry1 = db.get_or_allocate_partisan_ptr(clob);
    assert(entry1 != nullptr &&                   //
           entry1->outcome == outcome_class::U && //
           entry1->complexity == 0                //
    );

    sumgame sum(BLACK);
    sum.add(&clob);

    const db_entry_partisan* entry2 = db.get_partisan_ptr(sum);
    assert(entry1 == entry2);

    sum.pop(&clob);
}

void test_generate(bool extra_tests)
{
    database db;
    db.__register_built_in_types();
    DATABASE_REGISTER_TYPE(db, clobber_1xn);
    DATABASE_REGISTER_TYPE(db, domineering);
    assert(db.empty());

    if (extra_tests)
    {
        test_generate_impl(db, make_clobber_1xn_generator(7),
                           make_clobber_1xn_generator(7),
                           make_clobber_1xn_generator(11));

        test_generate_impl(db, make_domineering_generator(3, 3),
                           make_domineering_generator(3, 3),
                           make_domineering_generator(4, 4));
    }
    else
    {
        test_generate_impl(db, make_clobber_1xn_generator(5),
                           make_clobber_1xn_generator(5),
                           make_clobber_1xn_generator(6));

        test_generate_impl(db, make_domineering_generator(3, 3),
                           make_domineering_generator(3, 3),
                           make_domineering_generator(4, 4));
    }
}

void test_generate_options_stop_after()
{
    for (const db_gen_stop_after_enum stop_after : DB_GEN_STOP_AFTER_ENUM_ALL)
    {
        database db;
        db.__register_built_in_types();
        DATABASE_REGISTER_TYPE(db, domineering);
        assert(db.empty());

        db_gen_options_t opts;
        opts.silent = true;
        opts.stop_after = stop_after;

        i_db_game_generator* gen = make_domineering_generator(3, 3);
        db.generate_entries_partisan(*gen, opts);
        delete gen;

        domineering g("..|..");
        const db_entry_partisan* entry = db.get_partisan_ptr(g);
        assert(entry != nullptr);

        // Fields that must be present
        assert(entry->disk_game_type != 0);
        assert(entry->outcome != outcome_class::U);
        assert(entry->thermograph);

        // Bounds
        const bool expect_bounds = stop_after >= DB_GEN_STOP_AFTER_BOUNDS;
        assert(expect_bounds == (bool) entry->bounds_data);

        // Dominated moves
        const bool expect_dom = stop_after >= DB_GEN_STOP_AFTER_DOMINATED_MOVES;
        assert(expect_dom == (entry->complexity > 0));
        assert(expect_dom == (bool) entry->dominated_moves);

        // SEG
        const bool expect_seg = stop_after >= DB_GEN_STOP_AFTER_SEG;
        assert(expect_seg ==
               (entry->size_score_type != DB_GEN_SIZE_SCORE_TYPE_NONE));
        assert(expect_seg == (entry->size_score > 0));
        assert(expect_seg == !entry->simplest_equal_entry.is_nullptr());


        assert(expect_seg == (entry->serialized_sum.size() > 0));
        assert(expect_seg == (entry->subgame_links.size() == 1));
        if (!entry->subgame_links.empty())
            assert(&(entry->subgame_links.back().get_as_pointer()->second) == entry);
    }

}

void test_generate_options_size_score()
{
    unordered_set<pair<uint64_t, uint64_t>> size_score_and_complexity_set;

    for (const db_gen_size_score_type size_score_type : DB_GEN_SIZE_SCORE_TYPE_ENUM_ALL)
    {
        if (size_score_type == DB_GEN_SIZE_SCORE_TYPE_NONE)
            continue;

        database db;
        db.__register_built_in_types();
        DATABASE_REGISTER_TYPE(db, domineering);
        assert(db.empty());

        db_gen_options_t opts;
        opts.silent = true;
        opts.size_score_type = size_score_type;

        i_db_game_generator* gen = make_domineering_generator(3, 3);
        db.generate_entries_partisan(*gen, opts);
        delete gen;

        domineering g("...|.##|...");
        const db_entry_partisan* entry = db.get_partisan_ptr(g);
        assert(entry != nullptr);

        assert(entry->size_score_type == size_score_type);

        const auto inserted = size_score_and_complexity_set.emplace(
            entry->size_score, entry->complexity);

        assert(inserted.second);
    }


}

} // namespace

void database_test_all(bool extra_tests)
{
    test_db_hash();
    test_basic();
    test_query_game_and_sum();
    test_generate(extra_tests);
    test_generate_options_stop_after();
    test_generate_options_size_score();
}
