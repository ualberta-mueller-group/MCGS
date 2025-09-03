#include "basic_player.h"

#include <poll.h>
#include <cstdio>
#include <cstdlib>
#include <ios>
#include <limits>
#include <string>
#include <sys/poll.h>
#include <unordered_map>
#include <map>
#include <iostream>
#include <sstream>
#include "cgt_basics.h"
#include "file_parser.h"
#include "global_options.h"
#include "kayles.h"
#include "strip.h"
#include "sumgame.h"
#include "utilities.h"
#include "clobber_1xn.h"

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
/*
enum color_enum
{
    COLOR_RED = 0,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_MAGENTA,
    COLOR_CYAN,
    COLOR_WHITE,
    COLOR_DEFAULT,
};

ostream& set_color(ostream& os, color_enum color)
{
    assert(COLOR_RED <= color && color <= COLOR_DEFAULT);

    if (global::player_color())
        os << "\x1b[1;" << (31 + color) << 'm';

    return os;
}
*/

bool disable_skipws(std::ios_base& str)
{
    const std::ios::fmtflags oldskipws = str.flags() & std::ios::skipws;
    str.unsetf(std::ios::skipws);

    return oldskipws != 0;
}

void restore_skipws(std::ios_base& str, bool oldskipws)
{
    if (oldskipws)
        str.setf(std::ios::skipws);
}

bool press_enter()
{
    // Wait for newline
    cout << "(press enter)" << flush;

    char c = 0;

    while (!cin.eof() && c != '\n')
    {
        THROW_ASSERT(!cin.bad());
        c = cin.get();
    }

    THROW_ASSERT(!cin.bad());
    return (c == '\n');
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
    cout << flush;

    // Get choice
    string user_input;

    const int min_choice = 0;
    const int max_choice = n_choices - 1;

    while (true)
    {
        cout << "Choice [" << min_choice << " - " << max_choice << "]: ";

        user_input.clear();

        THROW_ASSERT(!cin.bad());
        getline(cin, user_input);
        THROW_ASSERT(!cin.bad());

        if (cin.eof())
            return {};

        if (!is_int(user_input))
            continue;

        int choice = atoi(user_input.c_str());

        if (!in_range(choice, min_choice, n_choices))
            continue;

        cout << endl;
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


//////////////////////////////////////// sumgame stuff
void print_sum(const sumgame& sum)
{
    const int n = sum.num_total_games();
    for (int i = 0; i < n; i++)
    {
        const game* g = sum.subgame(i);
        if (!g->is_active())
            continue;

        cout << *g << " ";
    }

    cout << '\n' << endl;
}

void play_on_sum(sumgame& sum, const sumgame_move& sum_move, bw player)
{
    assert(is_black_white(player));

    // Save current sumgame options
    const bool play_normalize = global::play_normalize();
    const bool play_split = global::play_split();

    // Disable them
    global::play_normalize.set(false);
    global::play_split.set(false);

    // Play the move
    sum.play_sum(sum_move, player);

    // Restore options
    global::play_normalize.set(play_normalize);
    global::play_split.set(play_split);
}

void undo_on_sum(sumgame& sum)
{
    // Save current sumgame options
    const bool play_normalize = global::play_normalize();
    const bool play_split = global::play_split();

    // Disable them
    global::play_normalize.set(false);
    global::play_split.set(false);

    // Undo the move
    sum.undo_move();

    // Restore options
    global::play_normalize.set(play_normalize);
    global::play_split.set(play_split);
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
    cout << "Choose a subgame to move on:" << endl;
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

    cout << "Choose a move within the subgame:" << endl;
    optional<int> move_choice = get_choice(moves_strings);

    if (!move_choice.has_value())
        return player_move::eof();

    const ::move& chosen_move = moves[move_choice.value()];

    return player_move(sumgame_move(sumgame_idx, chosen_move));
}

//////////////////////////////////////// playing logic
optional<bw> get_choice_color()
{
    const vector<string> options {"Black (B)", "White (W)"};
    optional<int> choice = get_choice(options);

    if (!choice.has_value())
        return {};

    return choice.value() == 0 ? BLACK : WHITE;
}

enum pre_post_enum
{
    PRE_POST_CONTINUE = 0,
    PRE_POST_BACK,
};

optional<pre_post_enum> get_choice_pre_post_action()
{
    const vector<string> options = {
        "Continue",
        "Back",
    };

    optional<int> choice_opt = get_choice(options);

    if (!choice_opt.has_value())
        return {};

    const int choice = choice_opt.value();

    assert(PRE_POST_CONTINUE <= choice && choice <= PRE_POST_BACK);
    return static_cast<pre_post_enum>(choice);
}

// true IFF play again
bool play_single(sumgame& sum)
{
    assert_restore_sumgame ars(sum);
    const bw original_player = sum.to_play();

    // Game setup

    // Clear screen, print game
    clear_screen();
    print_sum(sum);

    // CHOICE: player color
    cout << "Choose your color:" << endl;
    optional<bw> player_color_opt = get_choice_color();
    if (!player_color_opt.has_value())
        return false;

    // CHOICE: first player
    cout << "Choose first player:" << endl;
    optional<bw> first_player_opt = get_choice_color();
    if (!first_player_opt.has_value())
        return false;

    // Game initialization
    const bw player_color = player_color_opt.value();
    const char player_color_char = color_char(player_color);

    const bw mcgs_color = opponent(player_color);
    const char mcgs_color_char = color_char(mcgs_color);

    bw current_player = first_player_opt.value();

    int move_depth = 0;
    bool should_replay = false;

    // Helper functions
    auto undo = [&]() -> bool
    {
        if (move_depth == 0)
            return false;

        undo_on_sum(sum);
        move_depth--;
        current_player = opponent(current_player);

        return true;
    };

    auto play = [&](const sumgame_move& sm) -> void
    {
        play_on_sum(sum, sm, current_player);
        move_depth++;
        current_player = opponent(current_player);
    };

    auto print_turn = [&]() -> void
    {
        if (current_player == mcgs_color)
            cout << "MCGS's turn (" << mcgs_color_char << ").";
        else
        {
            assert(current_player == player_color);
            cout << "Your turn (" << player_color_char << ").";
        }

        cout << " Moves played so far: " << move_depth << endl;
        cout << endl;
    };

    // Main game loop
    while (true)
    {
        clear_screen();
        print_turn();
        print_sum(sum);

        // Pre-move action
        {
            optional<pre_post_enum> pre_action = get_choice_pre_post_action();
            if (!pre_action.has_value())
                break;

            if (pre_action.value() == PRE_POST_BACK)
            {
                bool did_undo = undo();

                if (did_undo)
                    continue;
                should_replay = true;
                break;
            }
            else
                assert(pre_action.value() == PRE_POST_CONTINUE);
        }

        // Get and play move
        if (current_player == mcgs_color)
        {
            optional<sumgame_move> sm = get_mcgs_move(sum, mcgs_color);

            if (!sm.has_value())
            {
                cout << "You win" << endl;
                press_enter();
                break;
            }

            play(sm.value());

            cout << "MCGS moves to:" << endl << endl;
            print_sum(sum);
        }
        else
        {
            player_move pm = get_player_move(sum, player_color);

            if (pm.status == PLAYER_MOVE_EOF)
            {
                cout << "Aborting..." << endl;
                break;
            }

            const optional<sumgame_move>& sm = pm.sum_move;
            if (!sm.has_value())
            {
                cout << "MCGS wins" << endl;
                press_enter();
                break;
            }

            play(sm.value());

            cout << "You moved to: " << endl << endl;
            print_sum(sum);
        }

        // Post-move action
        optional<pre_post_enum> post_action = get_choice_pre_post_action();
        if (!post_action.has_value())
            break;

        if (post_action.value() == PRE_POST_BACK)
        {
            bool did_undo = undo();
            assert(did_undo);
            continue;
        }

        assert(post_action.value() == PRE_POST_CONTINUE);
    }

    while (move_depth > 0)
        undo();

    sum.set_to_play(original_player);

    return should_replay;
}

bool has_kayles(const vector<game*>& games)
{
    for (game* g : games)
        if (g->game_type() == game_type<kayles>())
            return true;

    return false;
}

} // namespace


//////////////////////////////////////////////////
void play_games(file_parser& parser)
{
    /*
        Disable skipws and save old state

        This is necessary for press_enter(), otherwise you must write some
        non-whitespace character before presing enter...

        NOTE: Use "getline" instead of "cin >>" after doing this
    */
    const bool oldskipws = disable_skipws(cin);

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

        bool play = true;

        while (play)
        {
            assert_restore_sumgame ars(sum);
            play = play_single(sum);
        }

        sum.pop(games);

        gc.cleanup_games();
    }

    assert(sum.num_total_games() == 0);

    // Restore old cin state
    restore_skipws(cin, oldskipws);
}
