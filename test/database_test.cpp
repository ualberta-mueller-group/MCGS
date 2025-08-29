#include "database_test.h"


#include <memory>
#include <cassert>
#include <optional>

#include "cgt_basics.h"
#include "clobber_1xn.h"
#include "grid_generator.h"
#include "sumgame.h"
#include "gridlike_db_game_generator.h"
#include "nogo_1xn.h"
#include "database.h"
#include "utilities.h"

using namespace std;

namespace {
outcome_class get_outcome(sumgame& sum)
{
    sum.set_to_play(BLACK);
    bool black_win = sum.solve();

    sum.set_to_play(WHITE);
    bool white_win = sum.solve();

    return bools_to_outcome_class(black_win, white_win);
}

// set/get, clear, empty
void test_basic()
{
    database db;
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
    assert(!db.get_partizan(g1).has_value());
    assert(!db.get_partizan(g2).has_value());
    assert(!db.get_partizan(g3).has_value());

    // Insert entry for g1, should be available for g1 and g2
    {

        db_entry_partizan entry;
        entry.outcome = outcome_class::L;
        db.set_partizan(g1, entry);

        assert(!db.empty());

        optional<db_entry_partizan> query1 = db.get_partizan(g1);
        optional<db_entry_partizan> query2 = db.get_partizan(g2);
        optional<db_entry_partizan> query3 = db.get_partizan(g3);
        optional<db_entry_partizan> query4 = db.get_partizan(g4);

        assert(!db.empty());
        assert(query1.has_value() && query1->outcome == outcome_class::L);
        assert(query2.has_value() && query2->outcome == outcome_class::L);
        assert(!query3.has_value() && !query4.has_value());
    }

    // Now insert another entry for g3
    {
        db_entry_partizan entry;
        entry.outcome = outcome_class::N;
        db.set_partizan(g3, entry);

        assert(!db.empty());

        optional<db_entry_partizan> query1 = db.get_partizan(g1);
        optional<db_entry_partizan> query2 = db.get_partizan(g2);
        optional<db_entry_partizan> query3 = db.get_partizan(g3);
        optional<db_entry_partizan> query4 = db.get_partizan(g4);

        assert(!db.empty());
        assert(query1.has_value() && query1->outcome == outcome_class::L);
        assert(query2.has_value() && query2->outcome == outcome_class::L);
        assert(query3.has_value() && query3->outcome == outcome_class::N);
        assert(!query4.has_value());
    }

    // Insert entry for same board but different game
    {
        db_entry_partizan entry;
        entry.outcome = outcome_class::P;
        db.set_partizan(g4, entry);

        assert(!db.empty());

        optional<db_entry_partizan> query1 = db.get_partizan(g1);
        optional<db_entry_partizan> query2 = db.get_partizan(g2);
        optional<db_entry_partizan> query3 = db.get_partizan(g3);
        optional<db_entry_partizan> query4 = db.get_partizan(g4);

        assert(!db.empty());
        assert(query1.has_value() && query1->outcome == outcome_class::L);
        assert(query2.has_value() && query2->outcome == outcome_class::L);
        assert(query3.has_value() && query3->outcome == outcome_class::N);
        assert(query4.has_value() && query4->outcome == outcome_class::P);
    }

    db.clear();
    assert(db.empty());

    optional<db_entry_partizan> query1 = db.get_partizan(g1);
    optional<db_entry_partizan> query2 = db.get_partizan(g2);
    optional<db_entry_partizan> query3 = db.get_partizan(g3);
    optional<db_entry_partizan> query4 = db.get_partizan(g4);

    assert(!query1.has_value());
    assert(!query2.has_value());
    assert(!query3.has_value());
    assert(!query4.has_value());
}

void test_generate()
{
    database db;
    DATABASE_REGISTER_TYPE(db, clobber_1xn);

    assert(db.empty());

    // Generate some linear clobber entries
    {
        gridlike_db_game_generator<clobber_1xn, grid_generator_clobber> gen(5);
        db.generate_entries(gen, true);
    }

    assert(!db.empty());

    // Check entries are present and correct
    sumgame sum(BLACK);

    {
        gridlike_db_game_generator<clobber_1xn, grid_generator_clobber> gen(6);

        while (gen)
        {
            unique_ptr<game> g(gen.gen_game());
            ++gen;

            split_result sr = g->split();
            if (sr.has_value())
            {
                for (game* sg : *sr)
                    delete sg;

                continue;
            }

            g->normalize();

            clobber_1xn* g_clobber = dynamic_cast<clobber_1xn*>(g.get());
            assert(g_clobber != nullptr);

            const int len = g_clobber->size();

            optional<db_entry_partizan> entry = db.get_partizan(*g_clobber);

            if (len > 5)
                assert(!entry.has_value());
            else
            {
                assert(sum.num_total_games() == 0);
                sum.add(g.get());
                outcome_class oc = get_outcome(sum);
                sum.pop(g.get());

                assert(entry.has_value());
                assert(entry->outcome == oc);
            }
        }
    }

    assert(sum.num_total_games() == 0);
}

} // namespace

void database_test_all()
{
    test_basic();
    test_generate();
}

