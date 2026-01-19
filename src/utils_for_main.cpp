#include "utils_for_main.h"

#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <cassert>

#include "cgt_basics.h"
#include "cli_options.h"
#include "game.h"
#include "csv_row.h"
#include "search_utils.h"
#include "sumgame.h"
#include "test_case.h"
#include "get_winning_moves.h"
#include "file_parser.h"
#include "utilities.h"

using namespace std;

namespace {
string get_winning_moves_string(vector<string>& winning_moves)
{
    if (winning_moves.empty())
        return "None";

    sort_winning_moves(winning_moves);

    const string joined = string_join(winning_moves, " ");
    return joined;
}

void print_winning_moves_for_player(sumgame& sum, ebw player)
{
    assert_restore_sumgame ars(sum);
    vector<string> moves = get_winning_moves(sum, player);

    cout << player_name_bw_imp(player) << " winning moves:\n";
    cout << get_winning_moves_string(moves) << endl;
}

} // namespace

string get_games_string(const vector<game*>& games)
{
    if (games.empty())
        return "\t<no games specified>\n";

    stringstream stream;

    for (const game* g : games)
        stream << '\t' << *g << '\n';

    return stream.str();
}

void run_test_from_main(shared_ptr<i_test_case> test_case,
                        const cli_options& opts)
{
    const vector<game*>& games = test_case->get_games();
    const csv_row& row = test_case->get_csv_row();

    cout << "Test type: " << row.get_command_type_string() << endl;
    cout << get_games_string(games) << flush;
    cout << "Player: " << print_optional(row.player, "<N/A>") << endl;

    if (row.expected_result.has_value())
        cout << "Expected: " << row.expected_result.value() << endl;

    if (opts.dry_run)
        cout << "Not running search..." << endl;
    else
    {
        test_case->run(0);

        if (row.result.has_value())
            cout << "Got: " << row.result.value() << endl;

        cout << "Time (ms): " << row.get_time_ms_string() << endl;
        cout << "Status: " << row.get_status_string() << endl;
    }

    assert(row.comments.has_value());
    if (!row.comments->empty())
        cout << "\"" << row.comments.value() << "\"" << endl;
}


void print_winning_moves_by_chunk(shared_ptr<file_parser> parser)
{
    assert(parser.get() != nullptr);

    sumgame sum(BLACK);

    bool first_case = true;

    while (parser->parse_chunk())
    {
        if (!first_case)
            cout << endl;

        first_case = false;

        assert(sum.is_empty());
        vector<game*> games = parser->get_games();
        const string games_string = get_games_string(games);
        sum.add(games);

        cout << games_string << flush;

        if (sum.all_impartial())
        {
            print_winning_moves_for_player(sum, EMPTY);
        }
        else
        {
            print_winning_moves_for_player(sum, BLACK);
            print_winning_moves_for_player(sum, WHITE);
        }

        sum.pop(games);
        for (game* g : games)
            delete g;
    }
}

