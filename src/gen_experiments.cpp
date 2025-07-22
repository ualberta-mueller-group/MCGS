#include "gen_experiments.h"
#include <cstdint>
#include <fstream>
#include <optional>
#include <algorithm>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

// 1D
#include "clobber_1xn.h"
#include "global_options.h"
#include "histogram.h"
#include "nogo_1xn.h"
#include "elephants.h"

// 2D
#include "clobber.h"
#include "nogo.h"

// Other includes
#include "grid_utils.h"
#include "hashing.h"
#include "cgt_basics.h"
#include "random.h"
#include "strip.h"
#include "throw_assert.h"
#include "random_generator.h"

using namespace std;

namespace {

////////////////////////////////////////////////// Types
struct generated_game
{
    string board;
    bw player;

    size_t move_count;
    hash_t hash;

    game_type_t type;
};

struct generation_parameters
{
    // Universal
    size_t min_move_count;
    size_t max_move_count;

    uint16_t large_board_min_size;
    uint16_t large_board_max_size;

    // clobber_1xn
    uint16_t min_empties;
    uint16_t max_empties;

    // nogo_1xn
    uint16_t max_black_stones;
    uint16_t max_white_stones;
};

typedef generated_game (gen_func_t)(const generation_parameters&);

////////////////////////////////////////////////// Forward declarations
string board_to_string(const vector<int>& board);
unordered_map<game_type_t, string> init_game_names();

////////////////////////////////////////////////// Variables
optional<random_generator> rng;
optional<ofstream> outfile;
unordered_map<game_type_t, string> game_type_to_name;

////////////////////////////////////////////////// Helper functions
hash_t get_hash(const game& g, bw player)
{
    static global_hash gh;
    THROW_ASSERT(is_black_white(player));

    gh.reset();
    gh.add_subgame(0, &g);
    gh.set_to_play(player);

    return gh.get_value();
}

inline const string& get_game_name(game_type_t type)
{
    auto it = game_type_to_name.find(type);
    THROW_ASSERT(it != game_type_to_name.end());
    return it->second;
}

const string& get_game_name(const game& g)
{
    return get_game_name(g.game_type());
}

size_t count_moves_for(const game& g, bw player)
{
    THROW_ASSERT(is_black_white(player));
    move_generator* mg = g.create_move_generator(player);

    size_t n = 0;
    while (*mg)
    {
        n++;
        ++(*mg);
    }

    delete mg;
    return n;
}

inline bw get_random_bw()
{
    return (rng->get_u8() % 2) == 0 ? BLACK : WHITE;
}

unordered_map<game_type_t, string> init_game_names()
{
    unordered_map<game_type_t, string> name_map;

    name_map[game_type<clobber_1xn>()] = "clobber_1xn";
    name_map[game_type<nogo_1xn>()] = "nogo_1xn";
    name_map[game_type<elephants>()] = "elephants";

    name_map[game_type<clobber>()] = "clobber";
    name_map[game_type<nogo>()] = "nogo";

    return name_map;
}

/*
    Comment format:

    - Diagram ID (int)
    - Game type (string)
    - X value (int)
*/
void write_game(const generated_game& gg, int diagram_id)
{
    const string& game_name = get_game_name(gg.type);
    const char player_char = color_char(gg.player);

    stringstream str;

    str << "/*";
    str << "diagram::" << diagram_id << " ";
    str << "game::" << game_name << " ";
    str << "x::" << gg.move_count << " ";
    str << "*/ ";

    str << "[" << game_name << "] " << gg.board << " {" << player_char << "}";

    const string line = str.str();
    THROW_ASSERT(line.find('\n') == string::npos);

    *outfile << line << "\n";
}

string board_to_string(const vector<int>& board)
{
    string board_string;
    board_string.reserve(board.size());

    for (const int& tile : board)
        board_string.push_back(color_to_clobber_char(tile));

    return board_string;
}

void init()
{
    rng = random_generator(global::experiment_seed());

    outfile = ofstream("experiments.test2");
    THROW_ASSERT(outfile->is_open());

    THROW_ASSERT(game_type_to_name.empty());
    game_type_to_name = init_game_names();
}

generated_game gen_large_clobber_1xn(const generation_parameters& param)
{
    generated_game gen_game;

    const uint16_t size = rng->get_u16(param.large_board_min_size,
                                       param.large_board_max_size);

    const uint16_t n_empties = rng->get_u16(param.min_empties,
                                            param.max_empties);


    // Board
    vector<int> board(size, 0);

    for (int& tile : board)
        tile = get_random_bw();

    vector<size_t> empty_choices;
    empty_choices.reserve(board.size());

    for (size_t i = 0; i < board.size(); i++)
        empty_choices.push_back(i);

    shuffle(empty_choices.begin(), empty_choices.end(), rng->get_rng());

    for (uint16_t i = 0; i < n_empties; i++)
    {
        board[empty_choices.back()] = EMPTY;
        empty_choices.pop_back();
    }

    string board_string = board_to_string(board);

    // Player
    bw player = get_random_bw();

    // Move count, hash, type
    clobber_1xn g(board_string);

    size_t move_count = count_moves_for(g, player);
    hash_t hash = get_hash(g, player);
    game_type_t type = game_type<clobber_1xn>();

    // Assign fields
    gen_game.board = board_string;
    gen_game.player = player;

    gen_game.move_count = move_count;
    gen_game.hash = hash;

    gen_game.type = type;

    return gen_game;
}

optional<generated_game> gen_large_nogo_1xn_impl(
    const generation_parameters& param)
{
    generated_game gen_game;

    const uint16_t size = rng->get_u16(param.large_board_min_size,
                                       param.large_board_max_size);

    const uint16_t n_black_stones = rng->get_u16(0, param.max_black_stones);
    const uint16_t n_white_stones = rng->get_u16(0, param.max_white_stones);
    const uint16_t n_total_stones = n_black_stones + n_white_stones;

    if (n_total_stones >= size)
        return {};

    // Board
    vector<int> board(size, EMPTY);

    size_t next_stone = 0;
    for (uint16_t i = 0; i < n_black_stones; i++)
        board[next_stone++] = BLACK;

    for (uint16_t i = 0; i < n_white_stones; i++)
        board[next_stone++] = WHITE;

    assert(next_stone <= board.size());

    shuffle(board.begin(), board.end(), rng->get_rng());

    string board_string = board_to_string(board);


    // Player
    bw player = get_random_bw();

    // Move count, hash, type
    try
    {
        nogo_1xn g(board_string);

        if (!g.is_legal())
            return {};

        size_t move_count = count_moves_for(g, player);
        hash_t hash = get_hash(g, player);
        game_type_t type = game_type<nogo_1xn>();

        // Assign fields
        gen_game.board = board_string;
        gen_game.player = player;

        gen_game.move_count = move_count;
        gen_game.hash = hash;

        gen_game.type = type;

        return gen_game;
    }
    catch (exception& e)
    {
        return {};
    }
}

generated_game gen_large_nogo_1xn(const generation_parameters& param)
{
    while (true)
    {
        optional<generated_game> gen_game = gen_large_nogo_1xn_impl(param);

        if (gen_game.has_value())
            return *gen_game;
    }
}

optional<generated_game> gen_large_elephants_impl(const generation_parameters& param)
{
    generated_game gen_game;

    const uint16_t size = rng->get_u16(param.large_board_min_size,
                                       param.large_board_max_size);

    //const uint16_t n_black_stones = rng->get_u16(0, param.max_black_stones);
    //const uint16_t n_white_stones = rng->get_u16(0, param.max_white_stones);
    //const uint16_t n_total_stones = n_black_stones + n_white_stones;

    const uint16_t n_black_stones = param.max_black_stones;
    const uint16_t n_white_stones = param.max_white_stones;
    const uint16_t n_total_stones = n_black_stones + n_white_stones;

    if (n_total_stones >= size)
        return {};

    // Board
    vector<int> board(size, EMPTY);

    size_t next_stone = 0;
    for (uint16_t i = 0; i < n_black_stones; i++)
        board[next_stone++] = BLACK;

    for (uint16_t i = 0; i < n_white_stones; i++)
        board[next_stone++] = WHITE;

    assert(next_stone <= board.size());

    shuffle(board.begin(), board.end(), rng->get_rng());

    string board_string = board_to_string(board);


    // Player
    bw player = get_random_bw();

    // Move count, hash, type
    elephants g(board_string);

    size_t move_count = count_moves_for(g, player);
    hash_t hash = get_hash(g, player);
    game_type_t type = game_type<elephants>();

    // Assign fields
    gen_game.board = board_string;
    gen_game.player = player;

    gen_game.move_count = move_count;
    gen_game.hash = hash;

    gen_game.type = type;

    return gen_game;
}

generated_game gen_large_elephants(const generation_parameters& param)
{
    while (true)
    {
        optional<generated_game> gen_game = gen_large_elephants_impl(param);

        if (gen_game.has_value())
            return *gen_game;
    }
}

generation_parameters default_clobber_1xn_parameters()
{
    generation_parameters gp;

    //gp.min_move_count = 0;
    //gp.max_move_count = 20;

    gp.min_move_count = 0;
    gp.max_move_count = 14;

    gp.large_board_min_size = 16;
    gp.large_board_max_size = 35;

    gp.min_empties = 0;
    gp.max_empties = 3;

    return gp;
}

generation_parameters default_nogo_1xn_parameters()
{
    generation_parameters gp;

    //gp.min_move_count = 0;
    //gp.max_move_count = 20;

    gp.min_move_count = 0;
    gp.max_move_count = 14;

    gp.large_board_min_size = 16;
    gp.large_board_max_size = 35;

    gp.max_black_stones = 10;
    gp.max_white_stones = 10;

    return gp;
}

generation_parameters default_elephants_parameters()
{
    generation_parameters gp;

    //gp.min_move_count = 0;
    //gp.max_move_count = 20;

    gp.min_move_count = 0;
    gp.max_move_count = 8;

    gp.large_board_min_size = 16;
    gp.large_board_max_size = 46;

    gp.max_black_stones = 8;
    gp.max_white_stones = 8;

    return gp;
}

void gen_impl(const generation_parameters& param, uint64_t max_attempts,
              uint64_t bucket_size, gen_func_t& gen_func, int diagram_id)
{
    // Variables
    histogram hist(param.max_move_count);
    unordered_set<hash_t> seen_hashes;

    // Lambdas
    auto game_usable = [&](const generated_game& gg) -> bool
    {
        const bool seen = seen_hashes.find(gg.hash) != seen_hashes.end();

        const bool hist_ok =
            (param.min_move_count <= gg.move_count) &&
            (gg.move_count <= param.max_move_count) &&
            (hist.get_count(gg.move_count) < bucket_size);

        return !seen && hist_ok;
    };

    auto mark_game_used = [&](const generated_game& gg) -> void
    {
        // Hash
        auto it = seen_hashes.insert(gg.hash);
        THROW_ASSERT(it.second);

        // Histogram
        hist.count(gg.move_count);
    };

    // Main code

    // Try "large" games
    for (uint64_t attempt = 0; attempt < max_attempts; attempt++)
    {
        generated_game gen_game = gen_func(param);

        if (!game_usable(gen_game))
            continue;

        mark_game_used(gen_game);
        write_game(gen_game, diagram_id);
    }

    // Now use remaining "small" games

    cout << hist << endl;
}



} // namespace

////////////////////////////////////////////////// Main exported function
void gen_experiments()
{
    init();

    const uint64_t max_attempts = 1000000;
    const uint64_t bucket_size = 100;

    int next_diagram_id = 0;

    // clobber_1xn
    //gen_impl(default_clobber_1xn_parameters(), max_attempts, bucket_size,
    //         gen_large_clobber_1xn, next_diagram_id);

    next_diagram_id++;

    // nogo_1xn
    //gen_impl(default_nogo_1xn_parameters(), max_attempts, bucket_size,
    //         gen_large_nogo_1xn, next_diagram_id);

    next_diagram_id++;

    // elephants
    gen_impl(default_elephants_parameters(), max_attempts, bucket_size,
             gen_large_elephants, next_diagram_id);

    next_diagram_id++;
}
