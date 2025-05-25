#include "hash_types_test.h"
#include <iostream>
#include "hashing.h"
#include <unordered_set>

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
} // namespace


void hash_types_test_all()
{
    cout << __FILE__ << endl;

    random_table_test::test_global_tables();
    random_table_test::test_collision_multiple_positions();
    random_table_test::test_collision_single_position();
    random_table_test::test_resize();

    local_hash_test::test_toggle();
    local_hash_test::test_reset();
    local_hash_test::test_collision();
}
