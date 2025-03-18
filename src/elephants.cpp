#include "elephants.h"
#include "cgt_basics.h"
#include "cgt_move.h"
#include "strip.h"
#include <vector>

using std::string, std::cout, std::endl, std::pair;
using std::vector;

int player_dir(bw to_play)
{
    assert(is_black_white(to_play));
    return (to_play == BLACK) ? 1 : -1;
}

//////////////////////////////////////// elephants
elephants::elephants(const string& game_as_string) : strip(game_as_string)
{
}

elephants::elephants(const vector<int>& board) : strip(board)
{
}

void elephants::play(const move& m, bw to_play)
{
    assert_black_white(to_play);

    game::play(m, to_play);

    int from = cgt_move::from(m);
    int to = cgt_move::to(m);

    assert(checked_is_color(from, to_play));
    assert(checked_is_color(to, EMPTY));
    assert((to - from) == player_dir(to_play)); // correct direction

    play_stone(to, to_play);
    remove_stone(from);
}

void elephants::undo_move()
{
    move mc = game::last_move();
    game::undo_move();

    int to;
    bw to_play;
    int from = cgt_move::decode3(mc, &to, &to_play);

    assert(is_black_white(to_play));
    assert(checked_is_color(to, to_play));
    assert(checked_is_color(from, EMPTY));

    play_stone(from, to_play);
    remove_stone(to);
}

split_result elephants::_split_implementation() const
{
    vector<pair<int, int>> subgame_ranges;

    // <g1> O <0 or more empty tiles> X <g2>

    string board = board_as_string();
    const size_t N = board.size();

    size_t chunk_start = 0;

    bool seen_white = false;
    size_t last_white = 0;

    bool seen_black = false;
    size_t last_black = 0;

    // game splits when last_black > last_white and both are > -1

    for (size_t i = 0; i < N; i++)
    {
        const char c = board[i];
        int color = clobber_char_to_color(c);

        if (color == BLACK)
        {
            last_black = i;
            seen_black = true;
        }
        else if (color == WHITE)
        {
            last_white = i;
            seen_white = true;
        }

        // split game in two
        if ((seen_white && seen_black) && last_white < last_black)
        {
            subgame_ranges.push_back(
                {chunk_start, last_white - chunk_start + 1});

            chunk_start = i;
            seen_white = false;
            // keep last_black
        }
    }

    // always have one more subgame
    if (N > 0)
    {
        subgame_ranges.push_back(
            {chunk_start, (board.size() - 1) - chunk_start + 1});
    }

    if (subgame_ranges.size() == 1)
    {
        return split_result(); // no split
    }
    else
    {
        split_result result = split_result(vector<game*>());

        for (const pair<int, int>& range : subgame_ranges)
        {
            result->push_back(
                new elephants(board.substr(range.first, range.second)));
        }

        return result;
    }
}

move_generator* elephants::create_move_generator(bw to_play) const
{
    return new elephants_move_generator(*this, to_play);
}

game* elephants::inverse() const
{
    return new elephants(inverse_board());
}

//////////////////////////////////////// elephants_move_generator

elephants_move_generator::elephants_move_generator(const elephants& game,
                                                   bw to_play)
    : move_generator(to_play), _game(game)
{
    _idx = move_generator::to_play() == BLACK ? 0 : _game.size() - 1;
    _dir = player_dir(to_play);

    if (!is_move(_idx, _idx + _dir, to_play) && _game.size() > 0)
    {
        ++(*this);
    }
}

void elephants_move_generator::operator++()
{
    _idx += _dir;

    for (; in_range(_idx, 0, _game.size()); _idx += _dir)
    {
        if (is_move(_idx, _idx + _dir, to_play()))
        {
            break;
        }
    }
}

elephants_move_generator::operator bool() const
{
    return in_range(_idx, 0, _game.size());
}

move elephants_move_generator::gen_move() const
{
    assert(is_move(_idx, _idx + _dir, to_play()));
    return cgt_move::two_part_move(_idx, _idx + _dir);
}

bool elephants_move_generator::is_move(int from, int to, bw to_play) const
{
    if (!in_range(from, 0, _game.size()) || !in_range(to, 0, _game.size()))
    {
        return false;
    }

    int from_color = _game.at(from);
    int to_color = _game.at(to);

    if (                                        //
        (from_color == to_play)                 //
        && is_black_white(to_play)              //
        && (to_color == EMPTY)                  //
        && ((to - from) == player_dir(to_play)) //
        )                                       //
    {
        return true;
    }

    return false;
}
