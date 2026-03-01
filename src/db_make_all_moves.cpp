#include "db_make_all_moves.h"
#include "cgt_basics.h"
#include "cgt_move.h"
#include "database.h"
#include "global_database.h"
#include "grid_generator.h"
#include "sumgame.h"
#include "domineering.h"
#include "utilities.h"
#include <memory>


#ifdef MCGS_USE_DB_MOVES_TEST
struct domineering_move_t
{
    int_pair coord1;
    int_pair coord2;
};

static std::ostream& operator<<(std::ostream& os, const domineering_move_t& m)
{
    os << "Domineering move: ";
    os << m.coord1 << ", ";
    os << m.coord2;
    return os;
}

using namespace std;

////////////////////////////////////////////////// helpers
namespace {

domineering_move_t move_to_domineering_move(::move m)
{
    domineering_move_t dm;
    cgt_move::move4_unpack_coords(m, dm.coord1, dm.coord2);
    return dm;
}

void print_move_map(const map<hash_t, set<::move>>& move_map)
{
    cout << "vvv" << endl;

    for (const pair<const hash_t, set<::move>>& p : move_map)
    {
        cout << p.first << " ->" << endl;

        for (const ::move m : p.second)
        {
            domineering_move_t dm = move_to_domineering_move(m);
            cout << dm << endl;
        }
    }

    cout << "^^^" << endl;
}

void print_move_set(const set<::move>& move_set)
{
    cout << "{" << endl;
    for (const ::move m : move_set)
    {
        domineering_move_t dm = move_to_domineering_move(m);
        cout << "\t" << dm << endl;
    }
    cout << "}" << endl;
}

map<hash_t, const game*> make_game_map(const sumgame& sum)
{
    map<hash_t, const game*> game_map;

    const int n_games = sum.num_total_games();

    for (int i = 0; i < n_games; i++)
    {
        const game* sg = sum.subgame_const(i);
        if (!sg->is_active())
            continue;

        const hash_t hash = sg->get_local_hash();
        game_map[hash] = sg;
    }

    return game_map;

}

void test_sum_for_player(const sumgame& sum, bw player)
{
    assert(is_black_white(player));

    database& db = get_global_database();

    const db_entry_partisan* entry = db.get_partisan_ptr(sum);

    if (entry == nullptr)
        return;

    map<hash_t, set<::move>> db_map =
        (player == BLACK) ? entry->all_black_moves : entry->all_white_moves;

    map<hash_t, set<::move>> local_map = db_make_all_moves(sum, player);

    db_enc_or_dec_moves(sum, local_map, DB_MOVE_ACTION_ENCODE);

    cout << "=========" << endl;
    cout << "For: " << color_to_player_char(player) << endl;
    sum.print_simple(cout);
    cout << endl;
    //cout << local_map << endl;
    //cout << db_map << endl;
    print_move_map(local_map);
    print_move_map(db_map);

    assert(local_map == db_map);
}

set<::move> get_moves(const game* g, bw player)
{
    assert(is_black_white(player));

    set<::move> moves;

    unique_ptr<move_generator> gen(g->create_move_generator(player));

    while (*gen)
    {
        const ::move m = gen->gen_move();

        moves.insert(m);
        ++(*gen);
    }

    return moves;
}

void db_enc_or_dec_moves(const game* g, set<::move>& move_set,
                         db_move_action_enum action)
{
    set<::move> set_new;

    for (::move m : move_set)
    {
        optional<::move> m_new;

        switch (action)
        {
            case DB_MOVE_ACTION_ENCODE:
            {
                m_new = g->encode_grid_move_to_db(m);
                break;
            }
            case DB_MOVE_ACTION_DECODE:
            {
                m_new = g->decode_grid_move_from_db(m);
                break;
            }
        }

        assert(m_new.has_value());
        set_new.insert(*m_new);
    }

   move_set = std::move(set_new);
}

} // namespace

//////////////////////////////////////////////////
map<hash_t, set<::move>> db_make_all_moves(const sumgame& sum, bw player)
{
    assert(is_black_white(player));

    map<hash_t, set<::move>> move_map;

    unique_ptr<sumgame_move_generator> gen(sum.create_sum_move_generator(player));

    while (*gen)
    {
        const sumgame_move sm = gen->gen_sum_move();

        const game* sg = sum.subgame_const(sm.subgame_idx);
        assert(sg->is_active());

        const hash_t hash = sg->get_local_hash();
        const ::move m = sm.m;

        move_map[hash].insert(m);

        ++(*gen);
    }

    return move_map;
}

void db_enc_or_dec_moves(const sumgame& sum, map<hash_t, set<::move>>& move_map,
                         db_move_action_enum action)
{
    const map<hash_t, const game*> game_map = make_game_map(sum);

    for (pair<const hash_t, set<::move>>& p : move_map)
    {
        set<::move> new_set;

        const auto result = game_map.find(p.first);
        assert(result != game_map.end());
        const game* sg = result->second;
        assert(sg != nullptr);

        for (::move m : p.second)
        {
            optional<::move> m_new;

            switch (action)
            {
                case DB_MOVE_ACTION_ENCODE:
                {
                    m_new = sg->encode_grid_move_to_db(m);
                    break;
                }
                case DB_MOVE_ACTION_DECODE:
                {
                    m_new = sg->decode_grid_move_from_db(m);
                    break;
                }
            }

            assert(m_new.has_value());
            new_set.insert(*m_new);
        }

        p.second = std::move(new_set);
    }
}

void test_db_all_moves()
{
    grid_generator ggen({4, 4}, {BORDER, EMPTY}, false);

    sumgame sum(BLACK);

    while (ggen)
    {
        assert(sum.num_total_games() == 0);

        domineering g(ggen.gen_board(), ggen.get_shape());

        sum.add(&g);

        sum.split_and_normalize();

        test_sum_for_player(sum, BLACK);
        test_sum_for_player(sum, WHITE);

        sum.undo_split_and_normalize();
        sum.pop(&g);

        ++ggen;
    }

}
#endif
