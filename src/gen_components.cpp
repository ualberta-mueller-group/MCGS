/*
    Generate all 4-connected components on RxC grid. General sketch:

    - For RxC grid, use real grid of 2R+1,2C+1
    - Main loop: given all unique 


*/

#include "gen_components.h"

#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>

#include "grid.h"
#include "grid_generator.h"
#include "grid_hash.h"
#include "grid_location.h"

using namespace std;


////////////////////////////////////////////////// helpers
namespace {

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

void print_bool_grid(const vector<bool>& board, int_pair shape)
{
    int idx = 0;
    for (int r = 0; r < shape.first; r++)
    {
        for (int c = 0; c < shape.second; c++)
            cout << (board[idx + c] ? '#' : '.');

        cout << '\n';
        idx += shape.second;
    }
}

bool is_one_component(const vector<bool>& board, int_pair shape)
{
    const int area = shape.first * shape.second;

    vector<grid_location> open_queue;
    vector<bool> closed_set(area, false);
    int n_components = 0;

    for (grid_location loc_start(shape); loc_start.valid(); loc_start.increment_position())
    {
        // Find component
        const int point_start = loc_start.get_point();

        if (closed_set[point_start])
            continue;

        closed_set[point_start] = true;
        const bool val_start = board[point_start];

        if (!val_start)
            continue;

        if (n_components > 0)
            return false;

        assert(open_queue.empty());
        open_queue.emplace_back(loc_start);

        while (!open_queue.empty())
        {
            grid_location loc1 = open_queue.back();
            open_queue.pop_back();

            const int point1 = loc1.get_point();

            assert(closed_set[point1]);

            // Expand neighbors
            for (grid_dir dir : GRID_DIRS_CARDINAL)
            {
                grid_location loc2 = loc1;

                if (!loc2.move(dir))
                    continue;

                const int point2 = loc2.get_point();
                if (closed_set[point2])
                    continue;

                closed_set[point2] = true;
                const bool val2 = board[point2];

                if (val2)
                    open_queue.emplace_back(loc2);
           }


        }
        n_components++;
    }

    return n_components < 2;
}

hash_t gen_hash(const vector<bool>& vec, int_pair shape)
{
    grid_hash gh;
    gh.reset(shape);

    for (grid_location loc(shape); loc.valid(); loc.increment_position())
    {
        const int_pair& coord = loc.get_coord();
        const int point = loc.get_point();

        const bool val = vec[point];

        gh.toggle_value(coord.first, coord.second, val);
    }

    return gh.get_value();
}

unordered_set<hash_t> get_hash_set_trivial(int max_r, int max_c)
{
    const int_pair target_shape(max_r, max_c);
    unordered_set<hash_t> hash_set;

    grid_generator_domineering gen(max_r, max_c);

    assert(gen);
    while (gen && gen.get_shape() != target_shape)
        ++gen;

    assert(gen && gen.get_shape() == target_shape);
    for (const char &c : gen.gen_board())
        assert(c == '#' || c == '|');

    vector<bool> board_bool;

    uint64_t total_count = 0;
    for (; gen; ++gen)
    {
        const string& board_string = gen.gen_board();
        const int_pair& board_shape = gen.get_shape();

        if (board_shape != target_shape)
            continue;

        total_count++;

        const int board_area = board_shape.first * board_shape.second;

        if (board_area != board_bool.size())
            board_bool.resize(board_area);

        size_t vec_idx = 0;
        const size_t STR_SIZE = board_string.size();
        for (size_t str_idx = 0; str_idx < STR_SIZE; str_idx++)
        {
            const char c = board_string[str_idx];
            if (c == '|')
                continue;

            assert(c == '.' || c == '#');

            board_bool[vec_idx] = (c == '.');
            vec_idx++;
        }

        const bool is_one = is_one_component(board_bool, board_shape);

        if (is_one)
        {
            const hash_t hash = gen_hash(board_bool, board_shape);
            hash_set.insert(hash);
        }

        //set_color(cout, is_one ? COLOR_GREEN : COLOR_RED);
        //print_bool_grid(board_bool, board_shape);
        //cout << '\n';
        //set_color(cout, COLOR_RESET);
    }

    cout << hash_set.size() << " of " << total_count << endl;
    return hash_set;
}

} // namespace

//////////////////////////////////////////////////
void gen_components()
{
    get_hash_set_trivial(5, 5);



}
