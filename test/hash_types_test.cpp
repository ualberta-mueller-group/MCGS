#include "hash_types_test.h"
#include <iostream>
#include "hashing.h"
#include <unordered_set>
#include <optional>

#include "transposition.h"
#include "clobber.h"
#include "clobber_1xn.h"


using namespace std;

namespace {
namespace random_table_test {

// Check that global tables don't collide over some range
void test_global_tables()
{
    unordered_set<hash_t> elements;

    vector<random_table*> tables;
    tables.push_back(&get_global_random_table(RANDOM_TABLE_DEFAULT));
    tables.push_back(&get_global_random_table(RANDOM_TABLE_TYPE));
    tables.push_back(&get_global_random_table(RANDOM_TABLE_MODIFIER));
    tables.push_back(&get_global_random_table(RANDOM_TABLE_PLAYER));

    for (random_table* rt : tables)
    {
        const size_t N = rt->current_size();
        if (N > 1024)
        {
            cerr << "WARNING: random table size greater than 1024. "
                "Reconsider test in " << __FILE__ << ": " << __LINE__
                << endl;
        }

        for (int color = 0; color < 256; color++)
        {
            assert(N > 0);
            for (size_t pos = 0; pos < N; pos++)
            {
                hash_t element = rt->get_zobrist_val(pos, color);
                auto it = elements.insert(element);
                assert(it.second);
            }
        }
    }
}

// Check for collisions in single table, over some range
void test_collision_multiple_positions()
{
    unordered_set<hash_t> table_elements;

    random_table table(8, 5087);

    for (size_t pos = 0; pos < 8; pos++)
    {
        for (int color = 0; color < (1 << 16); color++)
        {
            hash_t element = table.get_zobrist_val(pos, color);
            auto it = table_elements.insert(element);
            assert(it.second);
        }
    }
}

// Check for collisions within single position
void test_collision_single_position()
{
    unordered_set<hash_t> table_elements;

    random_table table(1, 5659);

    for (int color = 0; color < (1 << 22); color++)
    {
        hash_t element = table.get_zobrist_val(0, color);
        auto it = table_elements.insert(element);
        assert(it.second);
    }
}

// Check that table resizing doesn't overwrite values
void test_resize()
{
    random_table rt(3, 13);
    const size_t N_1 = rt.current_size();
    assert(N_1 == 3);

    // get random_table elements before resize
    vector<hash_t> element_sequence;

    for (size_t pos = 0; pos < N_1; pos++)
    {
        for (int color = 0; color < 256; color++)
        {
            hash_t element = rt.get_zobrist_val(pos, color);
            element_sequence.push_back(element);
        }
    }

    assert(rt.current_size() == N_1);

    // Now resize the table by accessing past the end
    rt.get_zobrist_val(4, 0);
    const size_t N_2 = rt.current_size();
    assert(N_2 == 6);

    // Now check that the sequence is the same, and that all elements are unique
    size_t sequence_idx = 0;
    const size_t N_SEQUENCE = element_sequence.size();

    unordered_set<hash_t> element_set;

    for (size_t pos = 0; pos < N_2; pos++)
    {
        for (int color = 0; color < 256; color++)
        {
            hash_t element = rt.get_zobrist_val(pos, color);
            if (sequence_idx < N_SEQUENCE)
            {
                assert(element_sequence[sequence_idx] == element);
                sequence_idx++;
            }

            auto it = element_set.insert(element);
            assert(it.second);
        }
    }

    assert(rt.current_size() == N_2);
}

} // namespace random_table_test

namespace local_hash_test {

// Check that "toggle" functions actually toggle
void test_toggle()
{
    local_hash h;
    assert(h.get_value() == 0);

    // Toggle type
    h.toggle_type(1);
    assert(h.get_value() != 0);
    h.toggle_type(1);
    assert(h.get_value() == 0);

    // Toggle value
    h.toggle_value(0, 21);
    const hash_t v1 = h.get_value();
    assert(v1 != 0);

    h.toggle_value(1, 64);
    const hash_t v2 = h.get_value();
    assert(v2 != 0);

    h.toggle_value(1, 64);
    assert(h.get_value() == v1);
    h.toggle_value(0, 21);
    assert(h.get_value() == 0);
}

// Check that reset actually works, and that hashes are reproducible
void test_reset()
{
    local_hash h;
    assert(h.get_value() == 0);

    h.toggle_value(0, 37);
    const hash_t v1 = h.get_value();

    h.toggle_type(6);
    const hash_t v2 = h.get_value();
    assert(v1 != v2);

    h.reset();
    assert(h.get_value() == 0);

    h.toggle_value(0, 37);
    assert(h.get_value() == v1);

    h.toggle_type(6);
    assert(h.get_value() == v2);
}

// Check that there are no collisions over some range
void test_collision()
{
    unordered_set<hash_t> hashes;

    local_hash h;

    for (game_type_t type = 0; type < 5; type++)
    {
        h.reset();
        h.toggle_type(type);
        for (int color1 = 0; color1 < 64; color1++)
        {
            if (color1 > 0)
                h.toggle_value(0, color1 - 1);
            h.toggle_value(0, color1);

            for (int color2 = 0; color2 < 64; color2++)
            {
                if (color2 > 0)
                    h.toggle_value(1, color2 - 1);
                h.toggle_value(1, color2);

                hash_t hash = h.get_value();
                auto it = hashes.insert(hash);
                assert(it.second);
            }
        }
    }
}

} // namespace local_hash_test

namespace global_hash_test {

// Check adding and removing games, and that subgame order matters
void test_add_remove()
{
    clobber_1xn c1("..");
    clobber_1xn c2("...");

    global_hash gh;

    gh.reset();
    gh.set_to_play(BLACK);
    const hash_t v0 = gh.get_value();

    gh.reset();
    gh.set_to_play(BLACK);
    gh.add_subgame(0, &c1);
    const hash_t v1 = gh.get_value();

    gh.reset();
    gh.set_to_play(BLACK);
    gh.add_subgame(1, &c2);
    const hash_t v2 = gh.get_value();

    gh.reset();
    gh.set_to_play(BLACK);
    gh.add_subgame(0, &c1);
    gh.add_subgame(1, &c2);
    const hash_t v3 = gh.get_value();

    gh.reset();
    gh.set_to_play(BLACK);
    gh.add_subgame(0, &c2);
    gh.add_subgame(1, &c1);
    const hash_t v4 = gh.get_value();


    unordered_set<hash_t> hashes {v0, v1, v2, v3, v4};
    assert(hashes.size() == 5);

    gh.reset();
    gh.set_to_play(BLACK);
    gh.add_subgame(0, &c1);
    gh.add_subgame(1, &c2);
    assert(gh.get_value() == v3);

    gh.remove_subgame(0, &c1);
    assert(gh.get_value() == v2);
    gh.add_subgame(0, &c1);
    assert(gh.get_value() == v3);
    gh.remove_subgame(1, &c2);
    assert(gh.get_value() == v1);
    gh.remove_subgame(0, &c1);
    assert(gh.get_value() == v0);
}

// Check that reset works, and that value is reproducible after reset
void test_reset()
{
    global_hash gh;

    gh.set_to_play(BLACK);
    const hash_t v_black = gh.get_value();
    gh.set_to_play(WHITE);
    const hash_t v_white = gh.get_value();
    assert(v_black != v_white);

    gh.reset();

    gh.set_to_play(WHITE);
    assert(gh.get_value() == v_white);
    gh.set_to_play(BLACK);
    assert(gh.get_value() == v_black);
}

// Check for collisions for some range of repeated games
void test_repeated_games()
{
    clobber clob_2d("...");
    clobber_1xn clob_1d("...");

    global_hash gh_2d;
    global_hash gh_1d;
    global_hash gh_mixed;

    unordered_set<hash_t> hashes;

    auto check_hash = [&](global_hash& gh) -> void
    {
        {
            gh.set_to_play(BLACK);
            auto it_black = hashes.insert(gh.get_value());
            assert(it_black.second);
        }
        {
            gh.set_to_play(WHITE);
            auto it_white = hashes.insert(gh.get_value());
            assert(it_white.second);
        }
    };

    for (size_t i = 0; i < 400; i++)
    {
        gh_2d.add_subgame(i, &clob_2d);
        gh_1d.add_subgame(i, &clob_1d);

        gh_mixed.add_subgame(2 * i, &clob_2d);
        gh_mixed.add_subgame(2 * i + 1, &clob_1d);

        check_hash(gh_2d);
        check_hash(gh_1d);
        check_hash(gh_mixed);
    }
}

// Check setting to_play
void test_to_play()
{
    global_hash gh;

    gh.set_to_play(BLACK);
    const hash_t v_black = gh.get_value();
    gh.set_to_play(WHITE);
    const hash_t v_white = gh.get_value();

    assert(v_black != v_white);
}

} // namespace global_hash_test
namespace ttable_test {

struct test_entry
{
    int val1;
    char val2;

    bool operator==(const test_entry& rhs) const
    {
        return (val1 == rhs.val1) && (val2 == rhs.val2);
    }
};

typedef ttable<test_entry> tt_test;

// Basic usage test
void test_basic()
{
    tt_test tt(4, 0);
    hash_t hash = 13;

    tt_test::search_result sr = tt.search(hash);
    assert(!sr.entry_valid());
}

void test_store_get()
{
    tt_test tt(4, 0);

    test_entry ent = {41, 'D'};

    hash_t hash = 13;

    optional<test_entry> get1 = tt.get(hash);
    assert(!get1.has_value());

    tt.store(hash, ent);

    optional<test_entry> get2 = tt.get(hash);
    assert(get2.has_value());
    assert(ent == get2.value());

    for (hash_t i = 0; i < 4096; i++)
    {
        optional<test_entry> get3 = tt.get(i);
        if (i == hash)
            assert(get3.has_value());
        else
            assert(!get3.has_value());
    }
}

} // namespace ttable_test

} // namespace

void hash_types_test_all()
{
    random_table_test::test_global_tables();
    random_table_test::test_collision_multiple_positions();
    random_table_test::test_collision_single_position();
    random_table_test::test_resize();

    local_hash_test::test_toggle();
    local_hash_test::test_reset();
    local_hash_test::test_collision();

    global_hash_test::test_add_remove();
    global_hash_test::test_reset();
    global_hash_test::test_repeated_games();
    global_hash_test::test_to_play();

    ttable_test::test_basic();
    ttable_test::test_store_get();
}
