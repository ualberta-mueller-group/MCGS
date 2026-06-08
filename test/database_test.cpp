#include "database_test.h"

#include <memory>
#include <unordered_set>
#include <set>
#include <cassert>
#include <vector>

#include "bounds.h"
#include "cgt_basics.h"
#include "clobber_1xn.h"
#include "db_game_generator.h"
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

void check_entry_contents(sumgame& sum, const db_entry_partisan* entry, thermograph_builder_no_db& thermograph_builder)
{
    assert(entry != nullptr);

    // Outcome
    assert(entry->outcome == get_sum_outcome(sum));

    // Thermograph
    assert(entry->thermograph.get() != nullptr);
    shared_ptr<ThGraph> graph_nodb = thermograph_builder.build_thermograph(sum);
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

    db.generate_entries_partisan(*gen_generate, true);

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

i_db_game_generator* make_domineering_generator(int max_r, int max_c)
{
    grid_generator* gg =
        new grid_generator(int_pair(max_r, max_c), {EMPTY, BORDER}, false);

    return new gridlike_db_game_generator<domineering, GRIDLIKE_TYPE_GRID>(gg);
}

////////////////////////////////////////////////// Main test functions

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

} // namespace

void database_test_all(bool extra_tests)
{
    test_basic();
    test_query_game_and_sum();
    test_generate(extra_tests);
}
