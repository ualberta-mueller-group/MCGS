/*
    TODO: We really need a split_and_normalize() method
*/
#include "elephants.h"
#include "cgt_basics.h"
#include "cgt_move_new.h"
#include "print_move_helpers.h"
#include "strip.h"
#include "iobuffer.h"
#include "utilities.h"
#include <vector>
#include <cassert>
#include <utility>
#include <cstddef>

using std::string, std::pair;
using std::vector;

//////////////////////////////////////// helper functions
namespace {
int player_dir(bw to_play)
{
    assert(is_black_white(to_play));
    return (to_play == BLACK) ? 1 : -1;
}

/*
    1. Prune left whites and right blacks
    2. Shorten ...X to X, and O... to O
    3. Check if game empty
*/
bool refine_subgame_range(const vector<int>& board, pair<size_t, size_t>& range)
{
    const size_t board_size = board.size();

    if (board_size == 0 || range.second == 0)
        return false;

    size_t& start = range.first;
    size_t& length = range.second;
    size_t end = start + length;

    assert(start < board_size && length <= board_size);

    // Prune left whites/right blacks
    size_t prune_left_whites = 0;
    size_t prune_right_blacks = 0;

    for (size_t i = start; i < end; i++)
        if (board[i] == WHITE)
            prune_left_whites++;
        else
            break;

    for (size_t i = 0; i < length; i++)
        if (board[end - 1 - i] == BLACK)
            prune_right_blacks++;
        else
            break;

    // Do the pruning
    assert(prune_left_whites + prune_right_blacks <= length);
    // Prune left
    range.first += prune_left_whites;
    range.second -= prune_left_whites;
    // Prune right
    range.second -= prune_right_blacks;

    // Now simplify ".X" and "O."
    end = start + length;

    size_t prune_left_empty = 0;
    size_t prune_right_empty = 0;

    for (size_t i = start; i < end; i++)
    {
        const int& color = board[i];
        assert(is_empty_black_white(color));

        if (color == EMPTY)
            prune_left_empty++;
        else if (color == BLACK)
            break;
        else
        {
            prune_left_empty = 0;
            break;
        }
    }

    for (size_t i = 0; i < length; i++)
    {
        const int& color = board[end - 1 - i];
        assert(is_empty_black_white(color));

        if (color == EMPTY)
            prune_right_empty++;
        else if (color == WHITE)
            break;
        else
        {
            prune_right_empty = 0;
            break;
        }
    }

    // Board completely empty
    if (prune_left_empty == length || prune_right_empty == length)
    {
        assert(prune_left_empty == length && prune_right_empty == length);
        range.second = 0;
        return false;
    }

    assert(prune_left_empty < length && prune_right_empty < length);

    // Prune left
    range.first += prune_left_empty;
    range.second -= prune_left_empty;
    // Prune right
    range.second -= prune_right_empty;

    // Now check if board is dead
    end = start + length;
    assert(length > 0);

    bool has_color = false;
    bool has_empty = false;
    for (size_t i = start; i < end; i++)
    {
        const int& color = board[i];
        assert(is_empty_black_white(color));

        if (color == EMPTY)
            has_empty = true;
        else
            has_color = true;

        if (has_empty && has_color)
            return true;
    }

    range.second = 0;
    return false;
}

void get_subgame_ranges(const vector<int>& board,
                        vector<pair<size_t, size_t>>& ranges)
{
    assert(ranges.empty());

    const size_t N = board.size();

    if (N == 0)
        return;

    size_t chunk_start = 0;

    bool seen_black = false;
    size_t last_black = 0;

    bool seen_white = false;
    size_t last_white = 0;

    for (size_t i = 0; i < N; i++)
    {
        const int& color = board[i];
        assert(is_empty_black_white(color));

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

        if (!(seen_black && seen_white))
            continue;

        // XO split
        if (last_black + 1 == last_white)
        {
            assert(last_white == i);
            assert(chunk_start <= last_black && chunk_start <= last_white);

            // Skip X
            ranges.emplace_back(chunk_start, last_black - chunk_start);

            // Skip O
            chunk_start = i + 1;
            seen_black = false;
            seen_white = false;

            continue;
        }

        // O\.*X split
        if (last_white < last_black)
        {
            assert(last_black == i);
            assert(chunk_start <= last_black && chunk_start <= last_white);

            // keep O in "left" subgame
            ranges.emplace_back(chunk_start, last_white - chunk_start + 1);

            // keep X in remainder
            chunk_start = i;
            seen_white = false;
            // keep last_black and seen_black

            continue;
        }
    }

    // maybe have one more subgame
    if (N > 0 && chunk_start < N && (seen_black || seen_white))
        ranges.emplace_back(chunk_start, N - chunk_start);
}

inline void filter_ranges(const vector<int>& board,
                          vector<pair<size_t, size_t>>& ranges,
                          vector<pair<size_t, size_t>>& filtered_ranges)
{
    assert(filtered_ranges.empty());

    filtered_ranges.reserve(ranges.size());
    for (pair<size_t, size_t>& range : ranges)
        if (refine_subgame_range(board, range))
            filtered_ranges.emplace_back(range);
}

bool only_legal_colors(const std::vector<int>& board)
{
    for (const int& x : board)
        if (!is_empty_black_white(x))
            return false;
    return true;
}

} // namespace

//////////////////////////////////////// elephants
elephants::elephants(const string& game_as_string) : strip(game_as_string)
{
    THROW_ASSERT(only_legal_colors(board_const()));
}

elephants::elephants(const vector<int>& board) : strip(board)
{
    THROW_ASSERT(only_legal_colors(board_const()));
}

void elephants::play(const move& m, bw to_play)
{
    game::play(m, to_play);

    assert_black_white(to_play);

    const int from = cgt_move_new::move2_get_from(m);
    const int to = cgt_move_new::move2_get_to(m);

    assert(checked_is_color(from, to_play));
    assert(checked_is_color(to, EMPTY));
    assert((to - from) == player_dir(to_play)); // correct direction

    // incremental hash
    if (_hash_updatable())
    {
        local_hash& hash = _get_hash_ref();
        hash.toggle_value(from, to_play);
        hash.toggle_value(to, EMPTY);

        hash.toggle_value(from, EMPTY);
        hash.toggle_value(to, to_play);

        _mark_hash_updated();
    }

    play_stone(to, to_play);
    remove_stone(from);
}

void elephants::undo_move()
{
    move mc = game::last_move();
    game::undo_move();

    const int from = cgt_move_new::move2_get_from(mc);
    const int to = cgt_move_new::move2_get_to(mc);
    bw to_play = cgt_move_new::get_color(mc);

    assert(is_black_white(to_play));
    assert(checked_is_color(from, EMPTY));
    assert(checked_is_color(to, to_play));

    // incremental hash
    if (_hash_updatable())
    {
        local_hash& hash = _get_hash_ref();
        hash.toggle_value(from, EMPTY);
        hash.toggle_value(to, to_play);

        hash.toggle_value(from, to_play);
        hash.toggle_value(to, EMPTY);

        _mark_hash_updated();
    }

    play_stone(from, to_play);
    remove_stone(to);
}

void elephants::save_impl(obuffer& os) const
{
    _save_board(os, board_const());
}

dyn_serializable* elephants::load_impl(ibuffer& is)
{
    return new elephants(_load_board(is));
}

// Two types of splits: O\.*X, and XO
split_result elephants::_split_impl() const
{
    const vector<int>& board = board_const();
    const size_t board_size = board.size();

    if (board_size == 0)
        return {};

    vector<pair<size_t, size_t>> ranges;
    get_subgame_ranges(board, ranges);

    vector<pair<size_t, size_t>> ranges_filtered;
    filter_ranges(board, ranges, ranges_filtered);

    // Let normalize handle it...
    if (ranges_filtered.size() < 2)
        return {};

    split_result result = split_result(vector<game*>());

    for (const pair<size_t, size_t>& range : ranges_filtered)
        result->emplace_back(
            new elephants(vector_substr(board, range.first, range.second)));

    return result;
}

void elephants::_normalize_impl()
{
    const vector<int>& board = board_const();
    const size_t board_size = board.size();

    vector<pair<size_t, size_t>> ranges;
    get_subgame_ranges(board, ranges);

    vector<pair<size_t, size_t>> ranges_filtered;
    filter_ranges(board, ranges, ranges_filtered);

    const size_t n_subgames = ranges_filtered.size();
    if (n_subgames == 1 && ranges_filtered[0].first == 0 &&
        ranges_filtered[0].second == board_size)
    {
        if (_hash_updatable())
            _mark_hash_updated();

        _normalize_did_change.push_back(false);
        return;
    }

    _normalize_did_change.push_back(true);
    _normalize_boards.emplace_back(board);

    vector<int> new_board;
    new_board.reserve(board_size);

    // Stitch together filtered games...
    for (size_t i = 0; i < n_subgames; i++)
    {
        const pair<size_t, size_t>& range = ranges_filtered[i];
        assert(range.second > 0);

        const size_t start = range.first;
        const size_t end = start + range.second;

        for (size_t j = start; j < end; j++)
            new_board.push_back(board[j]);

        if (i + 1 < n_subgames)
        {
            const pair<size_t, size_t>& next_range = ranges_filtered[i + 1];
            assert(next_range.second > 0);

            const int& left_boundary = board[end - 1];
            const int& right_boundary = board[next_range.first];

            const bool have_black =
                (left_boundary == BLACK) | (right_boundary == BLACK);
            const bool have_white =
                (left_boundary == WHITE) | (right_boundary == WHITE);

            if (!(have_black && have_white))
            {
                new_board.push_back(BLACK);
                new_board.push_back(WHITE);
            }
        }
    }

    _set_board(new_board);
}

void elephants::_undo_normalize_impl()
{
    const bool did_change = _normalize_did_change.back();
    _normalize_did_change.pop_back();

    if (!did_change)
    {
        if (_hash_updatable())
            _mark_hash_updated();

        return;
    }

    _set_board(_normalize_boards.back());
    _normalize_boards.pop_back();
}

move_generator* elephants::create_move_generator(bw to_play) const
{
    return new elephants_move_generator(*this, to_play);
}

void elephants::print_move(std::ostream& str, const move& m) const
{
    // from, to
    print_move2_as_points(str, m);
}

game* elephants::inverse() const
{
    return new elephants(inverse_mirror_board());
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
    return cgt_move_new::move2_create(_idx, _idx + _dir);
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
