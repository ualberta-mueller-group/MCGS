#include "gen_experiments.h"
#include <cstdint>
#include <fstream>
#include <optional>
#include <algorithm>
#include <sstream>
#include <vector>
#include <cassert>
#include <exception>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <cstddef>

// 1D
#include "clobber_1xn.h"
#include "global_options.h"
#include "grid.h"
#include "histogram.h"
#include "nogo_1xn.h"
#include "elephants.h"

// 2D
#include "clobber.h"
#include "nogo.h"

// Other includes
#include "hashing.h"
#include "cgt_basics.h"
#include "strip.h"
#include "throw_assert.h"
#include "type_table.h"
#include "random_generator.h"

using namespace std;

namespace {

////////////////////////////////////////////////// Types
struct generated_game
{
    string board;
    bw player;

    size_t x_axis;
    hash_t hash;

    game_type_t type;
};

typedef generated_game (gen_func_t)();

////////////////////////////////////////////////// Forward declarations
string board_to_string(const vector<int>& board);
unordered_map<game_type_t, string> init_game_names();

////////////////////////////////////////////////// Variables
optional<random_generator> rng;
optional<ofstream> outfile;
optional<ofstream> histogram_file;
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

[[ maybe_unused ]] const string& get_game_name(const game& g) 
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

size_t count_subgames_for(const game& g)
{
    size_t n_subgames = 0;

    split_result sr = g.split();

    if (sr.has_value())
    {
        for (game* g : *sr)
        {
            delete g;
            n_subgames++;
        }

        return n_subgames;
    }

    // Either 0 or 1 depending on move count
    const size_t b_moves = count_moves_for(g, BLACK);
    const size_t w_moves = count_moves_for(g, WHITE);
    const size_t total_moves = b_moves + w_moves;

    return total_moves == 0 ? 0 : 1;
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
    str << "x::" << gg.x_axis << " ";
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
    histogram_file = ofstream("experiments_histogram.txt");
    THROW_ASSERT(outfile->is_open());
    THROW_ASSERT(histogram_file->is_open());

    THROW_ASSERT(game_type_to_name.empty());
    game_type_to_name = init_game_names();
}

generated_game gen_large_clobber_1xn()
{
    // move count: 0-14
    generated_game gen_game;

    const uint16_t size = rng->get_u16(16, 35);
    const uint16_t n_empties = rng->get_u16(0, 3);

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

    gen_game.x_axis = move_count;
    gen_game.hash = hash;

    gen_game.type = type;

    return gen_game;
}

optional<generated_game> gen_large_nogo_1xn_impl()
{
    // MOVES: 0-14
    generated_game gen_game;

    const uint16_t size = rng->get_u16(16, 35);

    const uint16_t max_stones_per_player = 10;

    const uint16_t n_black_stones = rng->get_u16(0, max_stones_per_player);
    const uint16_t n_white_stones = rng->get_u16(0, max_stones_per_player);
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

        gen_game.x_axis = move_count;
        gen_game.hash = hash;

        gen_game.type = type;

        return gen_game;
    }
    catch (exception& e)
    {
        return {};
    }
}

generated_game gen_large_nogo_1xn()
{
    while (true)
    {
        optional<generated_game> gen_game = gen_large_nogo_1xn_impl();

        if (gen_game.has_value())
            return *gen_game;
    }
}

optional<generated_game> gen_large_elephants_impl()
{
    generated_game gen_game;

    const uint16_t n_black_stones = rng->get_u16(0, 10);
    const uint16_t n_white_stones = rng->get_u16(0, 10);
    const uint16_t n_total_stones = n_black_stones + n_white_stones;

    const uint16_t size = n_total_stones * 3;

    THROW_ASSERT(size >= n_total_stones);

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

    //size_t move_count = count_moves_for(g, player);
    hash_t hash = get_hash(g, player);
    game_type_t type = game_type<elephants>();

    // Assign fields
    gen_game.board = board_string;
    gen_game.player = player;

    gen_game.x_axis = n_total_stones;
    gen_game.hash = hash;

    gen_game.type = type;

    return gen_game;
}

generated_game gen_large_elephants()
{
    while (true)
    {
        optional<generated_game> gen_game = gen_large_elephants_impl();

        if (gen_game.has_value())
            return *gen_game;
    }
}

generated_game gen_large_clobber()
{
    generated_game gen_game;

    const uint16_t rows = rng->get_u16(2, 2);
    const uint16_t cols = rng->get_u16(7, 12);
    const int_pair shape(rows, cols);

    const uint16_t total_tiles = rows * cols;

    const uint16_t n_empties = rng->get_u16(2, 3);
    //const uint16_t n_empties = 0;

    // Board
    vector<int> board(total_tiles, 0);

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

    // Player
    bw player = get_random_bw();

    // Move count, hash, type
    clobber g(board, shape);

    //size_t move_count = count_moves_for(g, player);
    hash_t hash = get_hash(g, player);
    game_type_t type = game_type<clobber>();


    string board_string = g.board_as_string();

    // Assign fields
    gen_game.board = board_string;
    gen_game.player = player;

    gen_game.x_axis = cols;
    gen_game.hash = hash;

    gen_game.type = type;

    return gen_game;
}

generated_game gen_large_clobber_1xn_subgames()
{
    generated_game gen_game;

    //const uint16_t size = rng->get_u16(16, 35);
    const uint16_t size = rng->get_u16(24, 24);
    const uint16_t n_empties = rng->get_u16(0, 10);

    // Board
    vector<int> board(size, 0);

    for (int& tile : board)
        tile = get_random_bw();

    for (uint16_t i = 0; i < n_empties; i++)
        board.push_back(EMPTY);

    shuffle(board.begin(), board.end(), rng->get_rng());

    string board_string = board_to_string(board);

    // Player
    bw player = get_random_bw();

    // Move count, hash, type
    clobber_1xn g(board_string);

    //size_t move_count = count_moves_for(g, player);
    hash_t hash = get_hash(g, player);
    game_type_t type = game_type<clobber_1xn>();

    // Assign fields
    gen_game.board = board_string;
    gen_game.player = player;

    gen_game.x_axis = count_subgames_for(g);
    gen_game.hash = hash;

    gen_game.type = type;

    return gen_game;
}


void gen_impl(uint64_t max_attempts, uint64_t bucket_size, gen_func_t& gen_func,
              int diagram_id, size_t min_x, size_t max_x)
{
    // Variables
    histogram hist(max_x);
    unordered_set<hash_t> seen_hashes;

    // Lambdas
    auto game_usable = [&](const generated_game& gg) -> bool
    {
        const bool seen = seen_hashes.find(gg.hash) != seen_hashes.end();

        const bool hist_ok =
            (min_x <= gg.x_axis) &&
            (gg.x_axis <= max_x) &&
            (hist.get_count(gg.x_axis) < bucket_size);

        return !seen && hist_ok;
    };

    auto mark_game_used = [&](const generated_game& gg) -> void
    {
        // Hash
        auto it = seen_hashes.insert(gg.hash);
        THROW_ASSERT(it.second);

        // Histogram
        hist.count(gg.x_axis);
    };

    // Main code

    // Try "large" games
    for (uint64_t attempt = 0; attempt < max_attempts; attempt++)
    {
        generated_game gen_game = gen_func();

        if (!game_usable(gen_game))
            continue;

        mark_game_used(gen_game);
        write_game(gen_game, diagram_id);
    }

    // Now use remaining "small" games

    cout << hist << endl;
    *histogram_file << hist << '\n' << endl;
}



} // namespace

////////////////////////////////////////////////// Main exported function
void gen_experiments()
{
    init();

    // EXPERIMENT VALUES
    //const uint64_t max_attempts = 24000000;
    //const uint64_t bucket_size = 2000;

    const uint64_t max_attempts = 24000000;
    const uint64_t bucket_size = 2000;


    //const uint64_t max_attempts = 1000000;
    //const uint64_t bucket_size = 50;

    int next_diagram_id = 0;


    // clobber_1xn
    gen_impl(max_attempts, bucket_size, gen_large_clobber_1xn, next_diagram_id,
             0, 13);

    next_diagram_id++;

    // nogo_1xn
    gen_impl(max_attempts, bucket_size, gen_large_nogo_1xn, next_diagram_id,
             0, 15);

    next_diagram_id++;

    // elephants (was 13)
    gen_impl(max_attempts, bucket_size, gen_large_elephants, next_diagram_id,
             0, 14);

    next_diagram_id++;

    // clobber
    gen_impl(max_attempts, bucket_size, gen_large_clobber, next_diagram_id,
             7, 20);

    next_diagram_id++;

    // clobber_1xn subgames
    gen_impl(max_attempts, bucket_size, gen_large_clobber_1xn_subgames,
             next_diagram_id, 1, 30);

    next_diagram_id++;
}
