/*
    TODO this is impractical in its current form for a few reasons:

    1. Generating and ranking all games takes some time due to split + normalize
    2. The number of games generated as a result of using shapes is huge for
    grid games

    For linear Clobber, all possible sums at or below the maximum rank should be
    generated naturally by the grid_generator. This is not the case for
    2D clobber:

    i.e. the largest rank for 4x4 is 16, but this sum won't appear:

  (rank 7)     (rank 7)
    .X..         ..X.
    XXXX    +    ..X.
    .X..         ..X.
    .X..         XXXX


*/
#include "rank_games.h"
#include "all_game_headers.h"
#include "cgt_basics.h"
#include "histogram.h"
#include "utilities.h"
#include <cstdint>
#include <string>
#include <unordered_set>

using namespace std;

namespace {

histogram hist;
unordered_set<hash_t> game_hashes;

unordered_map<uint64_t, shared_ptr<ofstream>> rank_to_ofstream;
unordered_map<uint64_t, shared_ptr<vector<string>>> rank_to_string_vec;

ofstream& get_ofstream(uint64_t rank)
{
    auto it = rank_to_ofstream.find(rank);

    if (it != rank_to_ofstream.end())
        return *it->second;

    const std::string file_name = "rank_" + std::to_string(rank) + ".txt";
    auto it2 = rank_to_ofstream.emplace(rank, new ofstream(file_name));

    assert(it2.second);
    return *it2.first->second;
}

std::vector<string>& get_string_vec(uint64_t rank)
{
    auto it = rank_to_string_vec.find(rank);

    if (it != rank_to_string_vec.end())
        return *it->second;

    auto it2 = rank_to_string_vec.emplace(rank, new vector<string>);

    assert(it2.second);
    return *it2.first->second;
}

uint64_t rank_grid_game(const grid* g)
{
    assert(g != nullptr);
    uint64_t rank = 0;

    const vector<int>& board = g->board_const();

    for (int stone : board)
        if (is_black_white(stone))
            rank++;

    return rank;
}

uint64_t rank_strip_game(const strip* g)
{
    assert(g != nullptr);
    uint64_t rank = 0;

    const vector<int>& board = g->board_const();

    for (int stone : board)
        if (is_black_white(stone))
            rank++;

    return rank;
}

void handle_game(const game* g)
{
    const hash_t hash = g->get_local_hash();

    {
        auto inserted = game_hashes.insert(hash);
        if (!inserted.second)
            return;
    }

    optional<uint64_t> rank;

    {
        const grid* g_grid = dynamic_cast<const grid*>(g);
        if (g_grid != nullptr)
            rank = rank_grid_game(g_grid);
        else
        {
            const strip* g_strip = dynamic_cast<const strip*>(g);
            if (g_strip != nullptr)
                rank = rank_strip_game(g_strip);
        }
    }

    assert(rank.has_value());

    hist.count(*rank);

    vector<string>& string_vec = get_string_vec(*rank);

    stringstream str;
    str << *g << '\n';
    string_vec.emplace_back(str.str());
}

void make_shapes(int remaining_budget, vector<int>& current,
                 vector<vector<int>>& shapes)
{
    assert(remaining_budget >= 0);

    int max_legal = remaining_budget;

    if (!current.empty())
        max_legal = min(remaining_budget, current.back());

    for (int choice = 1; choice <= max_legal; choice++)
    {
        current.push_back(choice);

        const int next_budget = remaining_budget - choice;
        assert(next_budget >= 0);

        if (next_budget == 0)
            shapes.push_back(current);
        else
            make_shapes(next_budget, current, shapes);

        current.pop_back();
    }
}

struct shape_data
{
    const vector<string>& string_vec;

    const size_t size;
    size_t idx;
};

void print_for_shape(const vector<int>& shape, uint64_t shape_rank)
{
    vector<shape_data> data;

    for (int chunk : shape)
    {
        const uint64_t rank = as_unsigned_unsafe(chunk);

        auto it = rank_to_string_vec.find(rank);
        if (it == rank_to_string_vec.end())
            return;

        const vector<string>& string_vec = *it->second;

        data.push_back({string_vec, string_vec.size(), 0});
    }

    assert(data.size() == shape.size());
    cout << shape << endl;

    auto increment = [&]() -> bool
    {
        for (shape_data& sd : data)
        {
            sd.idx++;

            if (sd.idx >= sd.size)
                sd.idx = 0;
            else
                return true;
        }

        return false;
    };

    while (true)
    {
        cout << shape_rank << ' ';
        for (const shape_data& sd : data)
            cout << sd.string_vec[sd.idx] << ' ';
        cout << '\n';

        if (!increment())
            break;
    }

}

} // namespace


//////////////////////////////////////////////////
void rank_games(i_db_game_generator& gen)
{
    while (gen)
    {
        game* g = gen.gen_game();
        ++gen;

        split_result sr = g->split();

        if (sr)
        {
            for (game* sg : *sr)
            {
                sg->normalize();
                handle_game(sg);
                delete sg;
            }
        }
        else
        {
            g->normalize();
            handle_game(g);
        }

        delete g;
    }


    uint64_t max_rank = 0;
    for (const pair<const uint64_t, shared_ptr<vector<string>>>& p : rank_to_string_vec)
        max_rank = max(max_rank, p.first);

    for (uint64_t rank = 1; rank <= max_rank; rank++)
    {
        vector<vector<int>> shapes;
        vector<int> temp_shape;
        make_shapes(rank, temp_shape, shapes);

        for (const vector<int>& shape : shapes)
            print_for_shape(shape, rank);
    }

    cout << hist << endl;

}
