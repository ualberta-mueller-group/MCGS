#include "print_moves.h"

#include <cassert>
#include <memory>
#include <string>
#include <vector>

#include "cgt_basics.h"
#include "get_winning_moves.h"
#include "search_utils.h"
#include "utils_for_main.h"
#include "game.h"
#include "sumgame.h"

using namespace std;

////////////////////////////////////////////////// Utilities
namespace {
string moves_to_string(const vector<string>& moves)
{
    if (moves.empty())
        return "None";

    const string joined = string_join(moves, " ");
    return joined;
}

string sum_to_string(const sumgame& sum)
{
    const int n_games = sum.num_total_games();

    vector<game*> active_games;

    for (int i = 0; i < n_games; i++)
    {
        game* g = sum.subgame(i);
        if (g->is_active())
            active_games.push_back(g);
    }

    return get_games_string(active_games);
}

} // namespace

////////////////////////////////////////////////// Main functions
string sumgame_move_to_string(const sumgame& sum, const sumgame_move& sm, ebw player,
                       bool with_subgame_idx)
{
    const game* g = sum.subgame_const(sm.subgame_idx);

    stringstream stream;

    if (with_subgame_idx)
        stream << sm.subgame_idx << ':';

    g->print_move(stream, sm.m, player);

    const string str = stream.str();
    assert(!string_contains_whitespace(str));

    return str;
}

vector<string> get_sum_moves(const sumgame& sum, bw player)
{
    assert(is_black_white(player));

    vector<string> sum_moves;

    const bool with_subgame_idx = sum.num_active_games() > 1;
    unique_ptr<sumgame_move_generator> mg(sum.create_sum_move_generator(player));

    while (*mg)
    {
        const sumgame_move sm = mg->gen_sum_move();
        ++(*mg);

        sum_moves.emplace_back(
            sumgame_move_to_string(sum, sm, player, with_subgame_idx));
    }

    return sum_moves;
}

vector<string> get_subgame_moves(const game* g, bw player)
{
    assert(is_black_white(player));
    vector<string> moves;

    unique_ptr<move_generator> mg(g->create_move_generator(player));

    while (*mg)
    {
        const ::move m = mg->gen_move();
        ++(*mg);

        stringstream str;
        g->print_move(str, m, player);

        moves.emplace_back(str.str());
    }

    return moves;
}

void print_winning_moves_for_player(ostream& os, sumgame& sum, ebw player)
{
    assert(is_empty_black_white(player));
    os << player_name_bw_imp(player) << " winning moves:\n";

    assert_restore_sumgame ars(sum);
    vector<string> moves = get_winning_moves(sum, player);
    sort_winning_moves(moves);

    os << moves_to_string(moves) << endl;
}

void print_sum_moves_for_player(ostream& os, const sumgame& sum, bw player)
{
    assert(is_empty_black_white(player));
    os << player_name_bw_imp(player) << " sum moves:\n";

    const vector<string> moves = get_sum_moves(sum, player);
    os << moves_to_string(moves) << endl;
}

void print_subgame_moves_for_player(ostream& os, const game* g, bw player)
{
    assert(is_empty_black_white(player));
    os << player_name_bw_imp(player) << " moves:\n";

    const vector<string> moves = get_subgame_moves(g, player);
    os << moves_to_string(moves) << endl;
}

void print_winning_moves_by_chunk(ostream& os, shared_ptr<file_parser> parser)
{
    assert(parser.get() != nullptr);

    sumgame sum(BLACK);

    bool first_case = true;

    while (parser->parse_chunk())
    {
        if (!first_case)
            os << endl;
        first_case = false;

        assert(sum.is_empty());
        vector<game*> games = parser->get_games();
        const string games_string = get_games_string(games);
        sum.add(games);

        os << "Sum:" << endl;
        os << games_string << flush;

        if (sum.all_impartial())
        {
            print_winning_moves_for_player(os, sum, EMPTY);
        }
        else
        {
            print_winning_moves_for_player(os, sum, BLACK);
            print_winning_moves_for_player(os, sum, WHITE);
        }

        sum.pop(games);
        for (game* g : games)
            delete g;
    }
}

void print_subgame_moves_by_chunk(ostream& os, std::shared_ptr<file_parser> parser)
{
    assert(parser.get() != nullptr);

    bool first_case = true;

    while (parser->parse_chunk())
    {
        if (!first_case)
            os << endl;
        first_case = false;

        vector<game*> games = parser->get_games();

        os << "Sum:" << endl;
        os << get_games_string(games) << flush;

        const size_t n_games = games.size();
        for (size_t subgame_idx = 0; subgame_idx < n_games; subgame_idx++)
        {
            os << endl;

            const game* g = games[subgame_idx];
            os << "Subgame " << subgame_idx << ": " << *g << endl;

            print_subgame_moves_for_player(os, g, BLACK);
            print_subgame_moves_for_player(os, g, WHITE);
        }

        for (game* g : games)
            delete g;
    }
}

void print_sum_moves_by_chunk(ostream& os, std::shared_ptr<file_parser> parser)
{
    assert(parser.get() != nullptr);

    bool first_case = true;

    sumgame sum(BLACK);

    while (parser->parse_chunk())
    {
        if (!first_case)
            os << endl;
        first_case = false;

        vector<game*> games = parser->get_games();

        assert(sum.is_empty());
        sum.add(games);

        os << "Sum:" << endl;
        os << sum_to_string(sum) << endl;

        print_sum_moves_for_player(os, sum, BLACK);
        print_sum_moves_for_player(os, sum, WHITE);

        sum.pop(games);
    }
}

