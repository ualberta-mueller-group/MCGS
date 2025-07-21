#include "gen_experiments.h"
#include <cstdint>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

// 1D
#include "cgt_basics.h"
#include "clobber_1xn.h"
#include "grid_utils.h"
#include "hashing.h"
#include "nogo_1xn.h"
#include "elephants.h"

// 2D
#include "clobber.h"
#include "nogo.h"
#include "random.h"
#include "strip.h"
#include "throw_assert.h"

using namespace std;

namespace {

string board_to_string(const vector<int>& board);
////////////////////

unordered_map<game_type_t, std::string> game_type_to_name;

inline const std::string& get_game_name(game_type_t type)
{
    auto it = game_type_to_name.find(type);
    THROW_ASSERT(it != game_type_to_name.end());
    return it->second;
}

const std::string& get_game_name(const game& g)
{
    return get_game_name(g.game_type());
}

size_t count_moves_for(const game& g, bw player)
{
    assert(is_black_white(player));
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
    return (get_random_u8() % 2) == 0 ? BLACK : WHITE;
}

void init_game_names()
{
    THROW_ASSERT(game_type_to_name.empty());

    game_type_to_name[game_type<clobber_1xn>()] = "clobber_1xn";
    game_type_to_name[game_type<nogo_1xn>()] = "nogo_1xn";
    game_type_to_name[game_type<elephants>()] = "elephants";

    game_type_to_name[game_type<clobber>()] = "clobber";
    game_type_to_name[game_type<nogo>()] = "nogo";
}

void write_game(ofstream& outfile, game_type_t type, const vector<int>& board,
                int diagram_id, int x_value, bw player)
{
    const string& game_name = get_game_name(type);
    const char player_char = color_char(player);

    string game_string = board_to_string(board);

    stringstream str;

    str << "/*";
    str << "diagram::" << diagram_id << " ";
    str << "game::" << game_name << " ";
    str << "x::" << x_value << " ";
    str << "*/ ";

    str << "[" << game_name << "] " << game_string << " {" << player_char << "}";

    string line = str.str();
    THROW_ASSERT(line.find('\n') == string::npos);

    outfile << line << '\n';
}

string board_to_string(const vector<int>& board)
{
    string board_string;
    board_string.reserve(board.size());

    for (const int& tile : board)
        board_string.push_back(color_to_clobber_char(tile));

    return board_string;
}

} // namespace


/*
    Comment format:

    - Diagram ID (int)
    - Game type (string)
    - X value (int)
*/
void gen_experiments()
{
    init_game_names();

    ofstream outfile("experiments.test2");
    THROW_ASSERT(outfile.is_open());

    // Parameters
    const size_t min_moves = 1;
    const size_t max_moves = 15;

    const uint16_t max_board_size = 32;

    const uint64_t target_games = 2000;

    uint64_t games_found = 0;
    unordered_set<hash_t> seen_hashes;
    vector<int> board;

    global_hash gh;

    auto unique_game = [&](const game& g, bw player) -> bool
    {
        gh.reset();
        gh.add_subgame(0, &g);
        gh.set_to_play(player);
        const hash_t hash = gh.get_value();

        return seen_hashes.insert(hash).second;
    };

    while (games_found < target_games)
    {
        const uint16_t size = get_random_u16() % max_board_size;
        const bw player = get_random_bw();

        board.resize(size);

        for (int& tile : board)
            tile = get_random_bw();

        clobber_1xn clob(board);

        //const size_t b_moves = count_moves_for(clob, BLACK);
        //const size_t w_moves = count_moves_for(clob, WHITE);

        const size_t move_count = count_moves_for(clob, player);


        if (!(min_moves <= move_count && move_count <= max_moves))
            continue;

        if (unique_game(clob, player))
        {
            games_found++;
            cout << games_found << "/" << target_games << endl;

            write_game(outfile, clob.game_type(), board, 0, move_count, player);
        }
    }

    outfile.close();
}
