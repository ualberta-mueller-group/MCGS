#include "basic_player.h"

/*
    TODO this file is really awful. This should be a class, etc
*/

#include <cstdlib>
#include <filesystem>
#include <ios>
#include <string>
#include <map>
#include <iostream>
#include <cassert>
#include <sstream>
#include <fstream>
#include <optional>
#include <vector>
#include <memory>
#include <exception>

#include "game.h"
#include "string_to_int.h"
#include "throw_assert.h"
#include "cgt_basics.h"
#include "file_parser.h"
#include "global_options.h"
#include "kayles.h"
#include "strip.h"
#include "sumgame.h"
#include "utilities.h"
#include "clobber_1xn.h"

////////////////////////////////////////////////// Global stream stuff
namespace {
static std::ostream* str_log = nullptr; // logging txt file

// intermediate stream flushed to both cout and log file
static std::stringstream str_both;

static bool old_cin_skipws;     // cin state before init_streams() changes it
static bool init_state = false; // was init_streams() called already?

// Allow press_enter() to work correctly without typing non-whitespace chars
void disable_skipws()
{
    const std::ios::fmtflags oldskipws = std::cin.flags() & std::ios::skipws;
    std::cin.unsetf(std::ios::skipws);
    old_cin_skipws = oldskipws != 0;
}

void restore_skipws()
{
    if (old_cin_skipws)
        std::cin.setf(std::ios::skipws);
}

// Flush intermediate stream to both output streams
void flush_str_both()
{
    const std::string data = str_both.str();
    str_both = std::stringstream();

    std::cout << data << std::flush;

    if (str_log)
        *str_log << data << std::flush;
}

bool press_enter()
{
    // Wait for newline
    str_both << "(press enter)";
    flush_str_both();

    char c = 0;

    while (!std::cin.eof() && c != '\n')
    {
        THROW_ASSERT(!std::cin.bad());
        c = std::cin.get();
    }

    THROW_ASSERT(!std::cin.bad());

    // Pressing enter should print a newline to the log
    if (str_log)
        *str_log << std::endl;

    return (c == '\n');
}

void init_streams(const std::string& log_name)
{
    assert(init_state == false);
    init_state = true;

    // Check this before changing any stream states
    THROW_ASSERT(!std::filesystem::exists(log_name),
                 "Log file \"" + log_name + "\" already exists!");

    // Set up stdout
    disable_skipws();

    if (log_name.empty())
    {
        std::cerr << "Warning: no log file specified for player" << std::endl;
        press_enter();
    }

    assert(str_log == nullptr);
    if (!log_name.empty())
    {
        std::ofstream* log = new std::ofstream(log_name);
        THROW_ASSERT(log->is_open());
        str_log = log;
    }

    str_both = std::stringstream();
}

void finalize_streams()
{
    assert(init_state == true);
    init_state = false;

    restore_skipws();

    flush_str_both();

    if (str_log)
    {
        delete str_log;
        str_log = nullptr;
    }
}

} // namespace

////////////////////////////////////////////////// Helper types

namespace {
//////////////////////////////////////// struct player_move
enum player_move_enum
{
    PLAYER_MOVE_OK = 0, // Either contains move, or no move exists
    PLAYER_MOVE_EOF,    // i.e. user pressed Ctrl D
};

struct player_move
{
    // Two constructors
    player_move(const std::optional<sumgame_move>& sum_move);
    static player_move eof();

    std::optional<sumgame_move> sum_move;
    player_move_enum status;
};

player_move::player_move(const std::optional<sumgame_move>& sum_move)
    : sum_move(sum_move), status(PLAYER_MOVE_OK)
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
enum color_enum
{
    COLOR_RED = 31,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_MAGENTA,
    COLOR_CYAN,
    COLOR_WHITE,
    COLOR_RESET = 0,
};

ostream& set_color(ostream& os, color_enum color)
{
    assert(                                          //
        color == COLOR_RESET ||                      //
        (COLOR_RED <= color && color <= COLOR_WHITE) //
            );                                       //

    if (global::player_color())
        os << "\x1b[" << color << 'm';

    return os;
}

/*
    Print enumerated options, and get a choice from the user.

    has_value() IFF not EOF. Throws on error.

    Must not pass empty options!
*/
template <class T>
optional<int> get_choice(const vector<T>& options)
{
    assert(!options.empty());

    // Print and count choices
    int n_choices = 0;
    for (const T& opt : options)
    {
        str_both << n_choices << ": ";
        str_both << opt << '\n';

        n_choices++;
    }
    flush_str_both();

    // Get choice
    string user_input;

    const int min_choice = 0;
    const int max_choice = n_choices - 1;

    string prompt_string;

    {
        stringstream str;
        str << "Choice [" << min_choice << " - " << max_choice << "]: ";
        prompt_string = str.str();
    }

    // Only print once to log file
    if (str_log)
        *str_log << prompt_string << flush;

    while (true)
    {
        cout << prompt_string << flush;

        user_input.clear();

        THROW_ASSERT(!cin.bad());
        getline(cin, user_input);
        THROW_ASSERT(!cin.bad());

        if (cin.eof())
            return {};

        int choice;
        try
        {
            choice = str_to_i(user_input);
        }
        catch (const exception& exc)
        {
            continue;
        }

        if (!in_range(choice, min_choice, n_choices))
            continue;

        // Make sure choice is shown in the log and not just the screen
        if (str_log)
            *str_log << choice << endl;

        str_both << endl;
        flush_str_both();

        return choice;
    }
}

template <class T>
optional<int> get_choice(const vector<T>&& options)
{
    return get_choice(options);
}

/*
    Get a choice of move from the user, in the `game::print_move` format
    (possibly with subgame index prefixes).

    Returns nullptr IFF EOF. Throws on error.

    Must not call with empty map!
*/
optional<size_t> get_move_choice(const vector<pair<string, sumgame_move>>& moves)
{
    assert(!moves.empty());

    const string prompt_string = "Choose move (or \"?\" to list moves): ";

    if (str_log)
        *str_log << prompt_string << flush;

    // Only print moves to the log file once
    bool listed_moves_in_log = false;

    string user_input;
    while (true)
    {
        cout << prompt_string << flush;
        user_input.clear();

        THROW_ASSERT(!cin.bad());
        getline(cin, user_input);
        THROW_ASSERT(!cin.bad());

        if (cin.eof())
            return {};

        if (user_input == "?")
        {
            string moves_string = "Available moves:";

            for (const pair<string, sumgame_move>& move_pair : moves)
                moves_string += " " + move_pair.first;

            cout << moves_string << endl;

            if (!listed_moves_in_log && str_log)
            {
                listed_moves_in_log = true;
                *str_log << user_input << '\n';
                *str_log << moves_string << endl;
            }

            continue;
        }

        optional<size_t> result;

        const size_t n_moves = moves.size();
        for (size_t i = 0; i < n_moves; i++)
        {
            if (moves[i].first == user_input)
            {
                result = i;
                break;
            }
        }

        if (!result.has_value())
            continue;

        if (str_log)
            *str_log << user_input << endl;
        str_both << endl;
        flush_str_both();

        return result;
    }
}

// Clear screen and draw top bar
void new_screen()
{
#if IS_WINDOWS
    system("cls");
#else
    system("clear");
#endif

    str_both << "==============================" << endl;
    flush_str_both();
}

//////////////////////////////////////// sumgame stuff
string get_sum_string(const sumgame& sum)
{
    stringstream str;

    const int n = sum.num_total_games();
    int n_active = 0;
    for (int i = 0; i < n; i++)
    {
        const game* g = sum.subgame(i);
        if (!g->is_active())
            continue;

        n_active++;
        str << *g << " ";
    }

    if (n_active == 0)
        str << "<Empty sum>";

    return str.str();
}

void print_sum_string(const string& sum_str)
{
    flush_str_both();
    set_color(cout, COLOR_BLUE);

    str_both << sum_str << '\n' << endl;
    flush_str_both();

    set_color(cout, COLOR_RESET);
}

void print_sum(const sumgame& sum)
{
    string sum_str = get_sum_string(sum);
    print_sum_string(sum_str);
}

// Disable normalize/split before playing a move
hash_t play_on_sum(sumgame& sum, const sumgame_move& sum_move, bw player,
                   bool split_norm)
{
    assert(is_black_white(player));

    // Save current sumgame options
    const bool play_normalize = global::play_normalize();
    const bool play_split = global::play_split();

    if (!split_norm)
    {
        // Disable them
        global::play_normalize.set(false);
        global::play_split.set(false);
    }

    // Play the move
    sum.play_sum(sum_move, player);

    if (!split_norm)
    {
        // Restore options
        global::play_normalize.set(play_normalize);
        global::play_split.set(play_split);
    }

    return sum.get_global_hash();
}

// Disable normalize/split before undoing a move
void undo_on_sum(sumgame& sum, bool split_norm)
{
    // Save current sumgame options
    const bool play_normalize = global::play_normalize();
    const bool play_split = global::play_split();

    if (!split_norm)
    {
        // Disable them
        global::play_normalize.set(false);
        global::play_split.set(false);
    }

    // Undo the move
    sum.undo_move();

    if (!split_norm)
    {
        // Restore options
        global::play_normalize.set(play_normalize);
        global::play_split.set(play_split);
    }
}

bool has_moves_for(const sumgame& sum, bw player)
{
    assert(is_black_white(player));

    const int n_games = sum.num_total_games();
    for (int i = 0; i < n_games; i++)
    {
        const game* g = sum.subgame_const(i);
        if (!g->is_active() || !g->has_moves_for(player))
            continue;

        return true;
    }

    return false;
}

// Winning move or random move. Otherwise no moves exist
optional<sumgame_move> get_mcgs_move(sumgame& sum, bw player)
{
    assert(is_black_white(player));
    assert_restore_sumgame ars(sum);

    return sum.get_winning_or_random_move(player);
}

player_move get_player_move(sumgame& sum, bw player)
{
    assert(is_black_white(player));
    assert_restore_sumgame ars(sum);

    vector<pair<string, sumgame_move>> sum_moves;
    unordered_set<string> move_strings;

    // Generate moves

    const bool include_prefix = sum.num_active_games() > 1;
    // Subgame index excluding inactive games
    int subgame_idx_local = 0;

    // Disable PITM
    const bool restore_pitm = global::pitm();
    global::pitm.set(false);

    const int n_games = sum.num_total_games();
    for (int i = 0; i < n_games; i++)
    {
        const game* g = sum.subgame_const(i);
        assert(g != nullptr);

        if (!g->is_active())
            continue;

        unique_ptr<move_generator> gen(g->create_move_generator(player));

        const string prefix_string = to_string(subgame_idx_local) + ":";

        for (; *gen; ++(*gen))
        {
            const ::move m = gen->gen_move();
            stringstream string_str;
            g->print_move(string_str, m, player);

            string move_string;
            if (include_prefix)
                move_string = prefix_string + string_str.str();
            else
                move_string = string_str.str();

            // Is this a unique move?
            const auto inserted = move_strings.insert(move_string);
            if (inserted.second)
                sum_moves.emplace_back(move_string, sumgame_move(i, m));
        }

        subgame_idx_local++;
    }

    // Restore PITM
    global::pitm.set(restore_pitm);

    // No subgames have moves
    if (sum_moves.empty())
        return player_move({});

    // Get choice
    optional<size_t> move_choice = get_move_choice(sum_moves);

    if (!move_choice.has_value())
        return player_move::eof();

    const sumgame_move& sm = sum_moves[*move_choice].second;
    return player_move(sm);
}

// To choose player color and first player
optional<bw> get_choice_color()
{
    const vector<string> options {"Black (B)", "White (W)"};
    optional<int> choice = get_choice(options);

    if (!choice.has_value())
        return {};

    return choice.value() == 0 ? BLACK : WHITE;
}

//////////////////////////////////////// playing logic
// Before and after move actions
enum pre_post_enum
{
    PRE_POST_CONTINUE = 0,
    PRE_POST_BACK,
};

enum end_game_enum
{
    END_GAME_END = 0,
    END_GAME_BACK,
    END_GAME_RESTART,
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

optional<end_game_enum> get_choice_end_game()
{
    str_both << "Play again:" << endl;
    flush_str_both();

    const vector<string> options =
    {
        "End",
        "Back",
        "Restart",
    };

    optional<int> choice = get_choice(options);

    if (!choice.has_value())
        return {};

    return static_cast<end_game_enum>(choice.value());
}

// true IFF should play again
bool play_single(sumgame& sum)
{
    assert_restore_sumgame ars(sum);
    const bw original_player = sum.to_play();

    // Game setup

    // Clear screen, print game
    new_screen();
    print_sum(sum);

    // CHOICE: player color
    str_both << "Choose your color:" << endl;
    flush_str_both();
    optional<bw> player_color_opt = get_choice_color();
    if (!player_color_opt.has_value())
        return false;

    // CHOICE: first player
    str_both << "Choose first player:" << endl;
    flush_str_both();
    optional<bw> first_player_opt = get_choice_color();
    if (!first_player_opt.has_value())
        return false;

    // Game initialization
    const bw player_color = player_color_opt.value();
    const char player_color_char = color_to_player_char(player_color);

    const bw mcgs_color = opponent(player_color);
    const char mcgs_color_char = color_to_player_char(mcgs_color);

    bw current_player = first_player_opt.value();

    int move_depth = 0;

    bool should_replay = false;

    //////////////////////// Helper functions
    auto undo_fn = [&]() -> bool
    {
        if (move_depth == 0)
            return false; // should return and call play_single() again

        undo_on_sum(sum, true);
        move_depth--;
        current_player = opponent(current_player);

        return true;
    };

    auto get_player_string = [&](bw color) -> string
    {
        if (color == player_color)
            return "You";
        return "MCGS";
    };

    auto play = [&](const sumgame_move& sm) -> void
    {
        const hash_t hash_no_sn = play_on_sum(sum, sm, current_player, false);
        const string str_no_sn = get_sum_string(sum);
        undo_on_sum(sum, false);

        const hash_t hash_with_sn = play_on_sum(sum, sm, current_player, true);
        const string str_with_sn = get_sum_string(sum);

        const string& player_string = get_player_string(current_player);

        str_both << player_string << " moved to:" << endl;
        print_sum_string(str_no_sn);

        if (hash_no_sn != hash_with_sn)
        {
            str_both << "After split/normalize, is equivalent to:" << endl;
            print_sum_string(str_with_sn);
        }

        flush_str_both();
        move_depth++;
        current_player = opponent(current_player);
    };

    auto print_turn = [&]() -> void
    {
        if (current_player == mcgs_color)
            str_both << "MCGS's turn (" << mcgs_color_char << ").";
        else
        {
            assert(current_player == player_color);
            str_both << "Your turn (" << player_color_char << ").";
        }

        str_both << " Moves played so far: " << move_depth << endl;
        str_both << endl;
        flush_str_both();
    };

    auto print_end = [&]() -> void
    {
        if (current_player == mcgs_color)
            str_both << "No moves. You win\n\n";
        else
            str_both << "No moves. MCGS wins\n\n";

        flush_str_both();
    };

    auto get_move = [&]() -> optional<sumgame_move>
    {
        optional<sumgame_move> move_opt;

        if (current_player == mcgs_color)
        {
            move_opt = get_mcgs_move(sum, mcgs_color);
            assert(move_opt.has_value());
        }
        else
        {
            assert(current_player == player_color);
            player_move pm = get_player_move(sum, player_color);

            if (pm.status == PLAYER_MOVE_EOF)
                return {};

            move_opt = pm.sum_move;
            assert(move_opt.has_value());
        }

        return move_opt;
    };


#define RESTART_MACRO() \
    { \
        should_replay = true; \
        break; \
    } \

#define UNDO_MACRO() \
    { \
        if (undo_fn()) \
            continue; \
        else \
            RESTART_MACRO(); \
    } \

#define CHECKED_OPTIONAL(non_opt_type, ident, opt_call) \
    non_opt_type ident; \
    { \
        const std::optional<non_opt_type>& opt = opt_call; \
        if (!opt.has_value()) \
            break; \
        ident = opt.value(); \
    }

    // Main game loop
    while (true)
    {
        const bool has_moves = has_moves_for(sum, current_player);

        new_screen();
        print_turn();
        print_sum(sum);

        // End of game
        if (!has_moves)
        {
            print_end();
            CHECKED_OPTIONAL(end_game_enum, end_choice, get_choice_end_game());

            if (end_choice == END_GAME_END) // End
                break;
            if (end_choice == END_GAME_BACK) // Back
                UNDO_MACRO();
            if (end_choice == END_GAME_RESTART) // Restart
                RESTART_MACRO();
        }

        assert(has_moves);

        // Pre-move action
        CHECKED_OPTIONAL(pre_post_enum, pre_action, get_choice_pre_post_action());
        if (pre_action == PRE_POST_BACK)
            UNDO_MACRO();

        // Get and play move
        CHECKED_OPTIONAL(sumgame_move, sum_move, get_move());
        play(sum_move);
        //print_moved_to();

        // Post-move action
        CHECKED_OPTIONAL(pre_post_enum, post_action, get_choice_pre_post_action());
        if (post_action == PRE_POST_BACK)
            UNDO_MACRO();
    }

    if (cin.eof())
    {
        str_both << "Aborting..." << endl;
        flush_str_both();
    }

    while (move_depth > 0)
        undo_fn();

    sum.set_to_play(original_player);

    return should_replay;
}

bool has_kayles(const vector<game*>& games)
{
    for (const game* g : games)
        if (g->game_type() == game_type<kayles>())
            return true;

    return false;
}

} // namespace

//////////////////////////////////////////////////
void play_games(file_parser& parser, const string& log_name)
{
    init_streams(log_name);

    sumgame sum(BLACK);

    while (parser.parse_chunk())
    {
        assert(sum.num_total_games() == 0);
        vector<game*> games = parser.get_games();

        THROW_ASSERT(!has_kayles(games),
                     "Kayles not currently supported by player!");

        assert(sum.num_total_games() == 0);
        sum.add(games);

        bool play = true;

        while (play)
        {
            assert_restore_sumgame ars(sum);
            play = play_single(sum) && !cin.eof();
        }

        sum.pop(games);

        for (game* g : games)
            delete g;

        if (cin.eof())
            break;
    }


    finalize_streams();
    assert(sum.num_total_games() == 0);
    set_color(cout, COLOR_RESET);
}
