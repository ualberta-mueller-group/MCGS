//---------------------------------------------------------------------------
// A game board which is a 1-dimensional strip
//---------------------------------------------------------------------------
#include "strip.h"

#include "cgt_basics.h"
#include "throw_assert.h"
#include <cassert>
#include <cstdlib>
#include <limits>
#include <vector>
#include <utility>
#include "warn_default.h"
#include <cstddef>

//---------------------------------------------------------------------------

int clobber_char_to_color(char c)
{
    if (c == 'X')
        return BLACK;
    else if (c == 'O')
        return WHITE;
    else if (c == '.')
        return EMPTY;
    else
        assert(false);

    exit(-1);
    return -1;
}

char color_to_clobber_char(int color)
{
    static char clobber_char[] = {'X', 'O', '.'};

    assert_range(color, BLACK, EMPTY + 1);
    return clobber_char[color];
}

namespace {

void check_is_clobber_char(char c)
{
    THROW_ASSERT(c == 'X' || c == 'O' || c == '.');
}

std::vector<int> string_to_board(const std::string& game_as_string)
{
    std::vector<int> board;
    for (auto c : game_as_string)
    {
        check_is_clobber_char(c);
        board.push_back(clobber_char_to_color(c));
    }
    return board;
}

std::string board_to_string(const std::vector<int>& board)
{
    std::string result;
    for (int p : board)
        result += color_to_clobber_char(p);
    return result;
}

template <const bool mirror>
inline std::vector<int> inverse_board_impl(
    const std::vector<int>& original_board)
{
    std::vector<int> new_board(original_board.size());

    const size_t N = original_board.size();
    assert(new_board.size() == N);

    for (size_t i = 0; i < N; i++)
    {
        int x;
        if constexpr (mirror)
            x = original_board[N - 1 - i];
        else
            x = original_board[i];

        new_board[i] = ebw_opponent(x);
    }

    return new_board;
}

} // namespace

//---------------------------------------------------------------------------

strip::strip(const std::vector<int>& board) : game(), _board(board)
{
    _check_legal();
}

strip::strip(const std::string& game_as_string)
    : strip(string_to_board(game_as_string))
{
}

std::string strip::board_as_string() const
{
    return board_to_string(_board);
}

void strip::_init_hash(local_hash& hash) const
{
    WARN_DEFAULT_IMPL();

    const size_t N = this->size();

    for (size_t i = 0; i < N; i++)
        hash.toggle_value(i, this->at(i));
}

void strip::_normalize_impl()
{
    WARN_DEFAULT_IMPL();

    // Is mirrored board lexicographically less than the current board?
    relation rel = _compare_boards(_board, _board, true, false);
    bool do_mirror = (rel == REL_LESS);

    _default_normalize_did_mirror.push_back(do_mirror);

    if (do_mirror)
        _mirror_self();
    else
    {
        if (_hash_updatable())
            _mark_hash_updated();
    }
}

void strip::_undo_normalize_impl()
{
    WARN_DEFAULT_IMPL();

    assert(!_default_normalize_did_mirror.empty());
    bool do_mirror = _default_normalize_did_mirror.back();
    _default_normalize_did_mirror.pop_back();

    if (do_mirror)
        _mirror_self();
    else
    {
        if (_hash_updatable())
            _mark_hash_updated();
    }
}

relation strip::_order_impl(const game* rhs) const
{
    WARN_DEFAULT_IMPL();
    assert(game_type() == rhs->game_type());

    const strip* other = reinterpret_cast<const strip*>(rhs);
    assert(dynamic_cast<const strip*>(rhs) == other);

    return _compare_boards(_board, other->_board);
}

relation strip::_compare_boards(const std::vector<int>& board1,
                                const std::vector<int>& board2, bool mirror1,
                                bool mirror2)
{
    if (board1.size() != board2.size())
        return board1.size() < board2.size() ? REL_LESS : REL_GREATER;

    const size_t N = board1.size();
    assert(board2.size() == N);

    // Compare contents of boards

    size_t idx1 = 0; // initial index (assume forward iteration)
    int step1 = 1;   // index stride (assume forward iteration)
    if (mirror1)     // If comparing mirror board, iterate backwards
    {
        idx1 = N - 1;
        step1 = -1;
    }

    size_t idx2 = 0;
    int step2 = 1;
    if (mirror2)
    {
        idx2 = N - 1;
        step2 = -1;
    }

    for (size_t i = 0; i < N; i++)
    {
        const int& val1 = board1[idx1];
        const int& val2 = board2[idx2];

        idx1 += step1;
        idx2 += step2;

        if (val1 == val2)
            continue;

        return val1 < val2 ? REL_LESS : REL_GREATER;
    }

    return REL_EQUAL;
}

void strip::_mirror_self()
{
    const std::vector<int> old_board = std::move(_board);
    assert(_board.size() == 0);

    _board.resize(old_board.size());

    size_t idx = 0;
    for (auto it = old_board.rbegin(); it != old_board.rend(); it++)
    {
        _board[idx] = *it;
        idx++;
    }
}

void strip::_save_board(obuffer& os, const std::vector<int>& board)
{
    const size_t size = board.size();
    os.write_u64(size);

    for (size_t i = 0; i < size; i++)
    {
        const int& tile = board[i];

        THROW_ASSERT(                                             //
            (int) std::numeric_limits<uint8_t>::min() <= tile &&  //
            tile <= (int) std::numeric_limits<uint8_t>::max()     //
            );                                                    //

        os.write_u8(board[i]);
    }
}

std::vector<int> strip::_load_board(ibuffer& is)
{
    std::vector<int> board;

    const size_t size = is.read_u64();
    board.reserve(size);

    for (size_t i = 0; i < size; i++)
        board.push_back(is.read_u8());

    return board;
}

void strip::_check_legal() const
{
    for (const int& x : _board)
        THROW_ASSERT(x == BLACK || x == WHITE || x == EMPTY);
}

std::vector<int> strip::inverse_board() const
{
    return inverse_board_impl<false>(_board);
}

std::vector<int> strip::inverse_mirror_board() const
{
    return inverse_board_impl<true>(_board);
}
