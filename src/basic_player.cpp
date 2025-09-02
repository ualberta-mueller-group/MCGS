#include "basic_player.h"

#include <cstdio>
#include <cstdlib>
#include <ios>
#include <limits>
#include <unordered_map>
#include <map>
#include <iostream>
#include <sstream>
#include "cgt_basics.h"
#include "file_parser.h"
#include "kayles.h"
#include "sumgame.h"
#include "utilities.h"

////////////////////////////////////////////////// Helper types

namespace {
//////////////////////////////////////// struct player_move
enum player_move_enum
{
    PLAYER_MOVE_OK = 0,
    PLAYER_MOVE_EOF, // i.e. user pressed Ctrl D
};

struct player_move
{
    player_move(const std::optional<sumgame_move>& sum_move);
    static player_move eof();

    std::optional<sumgame_move> sum_move;
    player_move_enum status;
};

player_move::player_move(const std::optional<sumgame_move>& sum_move)
    : sum_move(sum_move),
      status(PLAYER_MOVE_OK)
{
}

player_move player_move::eof()
{
    player_move pm({});
    pm.status = PLAYER_MOVE_EOF;
    return pm;
}

} // namespace

////////////////////////////////////////////////// Helper functions

using namespace std;

namespace {

//////////////////////////////////////// screen and I/O stuff
void press_enter()
{
    cout << "(press enter)" << flush;

    const std::ios::fmtflags f_old = cin.flags();
    const std::ios::fmtflags f_new = f_old & (~std::ios::skipws);

    cin.setf(f_new);

    char temp;

    do
    {
        temp = cin.get();
    }
    while (temp != '\n' && !cin.eof());

    cin.setf(f_old);
    assert(cin.flags() == f_old);
}

/*
    has_value() IFF not EOF. Throws on error.

    Must not pass empty options
*/
template <class T>
optional<int> get_choice(const vector<T>& options)
{
    assert(!options.empty());

    // Print and count choices
    int n_choices = 0;
    for (const T& opt : options)
    {
        cout << n_choices << ": ";
        cout << opt << '\n';

        n_choices++;
    }
    cout << endl;

    // Get choice
    string user_input;

    const int min_choice = 0;
    const int max_choice = n_choices - 1;

    while (true)
    {
        cout << "Choice [" << min_choice << " - " << max_choice << "]: ";

        cin >> user_input;

        if (cin.eof())
            return {};

        THROW_ASSERT(!cin.bad());

        if (!is_int(user_input))
            continue;

        int choice = atoi(user_input.c_str());

        if (!in_range(choice, min_choice, n_choices))
            continue;

        return choice;
    }
}

void clear_screen()
{
#if IS_WINDOWS
    system("cls");
#else
    system("clear");
#endif
}

//////////////////////////////////////// move getting functions
optional<sumgame_move> get_mcgs_move(sumgame& sum, bw player)
{
    assert(is_black_white(player));
    assert_restore_sumgame ars(sum);

    return sum.get_winning_or_random_move(player);
}

player_move get_player_move(sumgame &sum, bw player)
{
    assert(is_black_white(player));
    assert_restore_sumgame ars(sum);

    // Find all subgames which have moves for to_play
    vector<game*> available_games;
    vector<string> available_games_strings;
    map<int, int> choice_idx_to_sumgame_idx;

    {
        int choice_idx = 0;

        const int n_games = sum.num_total_games();
        for (int i = 0; i < n_games; i++)
        {
            game* g = sum.subgame(i);

            if (!g->is_active() || !g->has_moves_for(player))
                continue;

            available_games.push_back(g);

            stringstream str;
            str << *g;
            available_games_strings.emplace_back(str.str());

            choice_idx_to_sumgame_idx[choice_idx] = i;
            choice_idx++;
        }

        assert(available_games.size() == available_games_strings.size());
        assert(available_games.size() == choice_idx_to_sumgame_idx.size());
    }

    // No subgames have moves
    if (available_games.empty())
        return player_move({});

    // Get the subgame choice
    cout << "Choose a subgame" << endl;
    optional<int> game_choice_idx = get_choice(available_games_strings);

    if (!game_choice_idx.has_value())
        return player_move::eof();

    // Find all moves within the subgame
    game* chosen_game = available_games[game_choice_idx.value()];

    int sumgame_idx = -1;
    vector<::move> moves;
    vector<string> moves_strings;

    {
        // Get sumgame idx
        auto it = choice_idx_to_sumgame_idx.find(game_choice_idx.value());
        assert(it != choice_idx_to_sumgame_idx.end());
        sumgame_idx = it->second;

        // Generate options
        unique_ptr<move_generator> gen(chosen_game->create_move_generator(player));
        while (*gen)
        {
            assert_restore_game arg(*chosen_game);

            moves.emplace_back(gen->gen_move());
            ++(*gen);

            const ::move& m = moves.back();

            // Play, print, undo
            chosen_game->play(m, player);
            stringstream str;
            str << *chosen_game;
            chosen_game->undo_move();

            moves_strings.push_back(str.str());
        }
    }

    assert(!moves.empty());
    assert(moves.size() == moves_strings.size());

    optional<int> move_choice = get_choice(moves_strings);

    if (!move_choice.has_value())
        return player_move::eof();

    const ::move& chosen_move = moves[move_choice.value()];

    return player_move(sumgame_move(sumgame_idx, chosen_move));
}

//////////////////////////////////////// playing logic
optional<bw> get_choice_color()
{
    const vector<string> options {"Black", "White"};
    optional<int> choice = get_choice(options);

    if (!choice.has_value())
        return {};

    return choice.value() == 0 ? BLACK : WHITE;
}

void play_single(sumgame& sum)
{
    assert_restore_sumgame ars(sum);
    const bw original_player = sum.to_play();

    // [ GAME PREAMBLE ]

    // Clear screen
    clear_screen();

    // Print game
    cout << sum << endl;

    // CHOICE: player color
    cout << "Choose your color:" << endl;
    optional<bw> player_color_opt = get_choice_color();
    if (!player_color_opt.has_value())
        return;

    // CHOICE: first player
    cout << "Choose first player:" << endl;
    optional<bw> first_player_opt = get_choice_color();
    if (!first_player_opt.has_value())
        return;

    // [ GAME LOOP ]
    const bw player_color = player_color_opt.value();
    const bw mcgs_color = opponent(player_color);

    bw current_player = first_player_opt.value();

    int move_depth = 0;

    while (true)
    {
        clear_screen();

        if (current_player == mcgs_color)
        {
            optional<sumgame_move> sm = get_mcgs_move(sum, mcgs_color);

            if (!sm.has_value())
            {
                cout << "You win ";
                press_enter();
                break;
            }

            cout << sum << endl;

            cout << "MCGS moves to:" << endl;

            sum.play_sum(sm.value(), mcgs_color);
            move_depth++;

            cout << sum << endl;
            press_enter();
        }
        else
        {
            assert(current_player == player_color);
            player_move pm = get_player_move(sum, player_color);

            if (pm.status == PLAYER_MOVE_EOF)
            {
                cout << "Aborting..." << endl;
                break;
            }

            optional<sumgame_move>& sm = pm.sum_move;
            if (!sm.has_value())
            {
                cout << "MCGS wins ";
                press_enter();
                break;
            }

            cout << "Result:" << endl;
            sum.play_sum(sm.value(), player_color);
            move_depth++;
            cout << sum << endl;

            press_enter();
        }

        current_player = opponent(current_player);
    }

    while (move_depth > 0)
    {
        sum.undo_move();
        move_depth--;
    }

    sum.set_to_play(original_player);
}

bool has_kayles(const vector<game*>& games)
{
    for (game* g : games)
        if (g->game_type() == game_type<kayles>())
            return true;

    return false;
}

} // namespace



void play_games(file_parser& parser)
{
    game_case gc;
    sumgame sum(BLACK);

    while (parser.parse_chunk(gc))
    {
        assert(sum.num_total_games() == 0);
        vector<game*>& games = gc.games;

        THROW_ASSERT(!has_kayles(games),
                     "Kayles not currently supported by player!");

        assert(sum.num_total_games() == 0);
        sum.add(games);
        {
            assert_restore_sumgame ars(sum);
            play_single(sum);
        }
        sum.pop(games);

        gc.cleanup_games();
    }

    assert(sum.num_total_games() == 0);
}
