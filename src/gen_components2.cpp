#include "gen_components2.h"
#include "gen_components.h"
#include "grid_generator.h"

using namespace std;

// TODO lots of duplicates and messy code
// TODO Clean up grid_generators general...

////////////////////////////////////////////////// helpers
namespace {
hash_t gen_hash(const grid_generator_impl::grid_mask& vec, int_pair shape)
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

// 'X' --true--> 'O' --false--> 'X'
bool increment_char_clobber_bw(char& c)
{
    if (c == 'X')
    {
        c = 'O';
        return true;
    }

    assert(c == 'O');
    c = 'X';
    return false;
}

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

void print_bool_grid(const grid_generator_impl::grid_mask& board, int_pair shape)
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

} // namespace 

//////////////////////////////////////////////////
// grid_generator_masked_cc methods

grid_generator_masked_cc::grid_generator_masked_cc(const int_pair& max_shape, bool only_max)
    : grid_generator_masked(max_shape),
      _only_max(only_max)
{
    _init();
}

grid_generator_masked_cc::grid_generator_masked_cc(int max_rows, int max_cols, bool only_max)
    : grid_generator_masked(max_rows, max_cols),
      _only_max(only_max)
{
    _init();
}

grid_generator_masked_cc::grid_generator_masked_cc(int max_cols, bool only_max)
    : grid_generator_masked(1, max_cols),
      _only_max(only_max)
{
    _init();
}

bool grid_generator_masked_cc::_is_one_cc(
    const grid_generator_impl::grid_mask& board, int_pair shape)
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

void grid_generator_masked_cc::_init_mask()
{
    _mask.set_shape(_shape);
    assert(_is_one_cc(_mask, _shape));

    const hash_t hash = gen_hash(_mask, _shape);
    auto it = _mask_hashes.insert(hash);
    assert(it.second);
}

bool grid_generator_masked_cc::_increment_mask()
{
    _scroll_mask_to_cc();
    return _mask;
}

void grid_generator_masked_cc::_scroll_mask_to_cc()
{
    assert(*this);

    // TODO I think this is okay???
    if (!_mask)
        return;

    ++_mask;

    while (true)
    {
        if (!_mask)
            return;

        if (!_is_one_cc(_mask, _shape))
        {
            ++_mask;
            continue;
        }

        const hash_t hash = gen_hash(_mask, _shape);
        auto it = _mask_hashes.insert(hash);

        if (!it.second)
        {
            ++_mask;
            continue;
        }

        return;
    }
}

void grid_generator_masked_cc::_init()
{
    if (_only_max)
    {
        _shape = {_max_shape.first, _max_shape.second - 1};
        _mask.set_shape_fill(_shape);
    }
}

//////////////////////////////////////////////////
// grid_generator_clobber_cc methods

grid_generator_clobber_cc::grid_generator_clobber_cc(const int_pair& max_shape)
    : grid_generator_masked_cc(max_shape, true)
{
    assert(*this);
    ++(*this);
}

grid_generator_clobber_cc::grid_generator_clobber_cc(int max_rows, int max_cols)
    : grid_generator_masked_cc(max_rows, max_cols, true)
{
    assert(*this);
    ++(*this);
}

grid_generator_clobber_cc::grid_generator_clobber_cc(int max_cols)
    : grid_generator_masked_cc(1, max_cols, true)
{
    assert(*this);
    ++(*this);
}

void grid_generator_clobber_cc::_init_board()
{
    const char black_char = color_to_clobber_char(BLACK);
    const char empty_char = color_to_clobber_char(EMPTY);
    init_board_helper_masked(_board, _shape, _mask, black_char, empty_char);
}

bool grid_generator_clobber_cc::_increment_board()
{
    bool carry = true;

    size_t mask_idx = 0;
    const size_t board_size = _board.size();

    for (size_t i = 0; i < board_size; i++)
    {
        if (_board[i] == '|')
            continue;

        if (!_mask[mask_idx++])
            continue;

        carry = !increment_char_clobber_bw(_board[i]);

        if (!carry)
            break;
    }

    return !carry;
}

/*
void grid_generator_clobber_cc::_init_board()
{
    const char black_char = color_to_clobber_char(BLACK);
    const char empty_char = color_to_clobber_char(EMPTY);
    init_board_helper_masked(_board, _shape, _mask, black_char, empty_char);
}

bool grid_generator_clobber_cc::_increment_board()
{
    bool carry = true;

    size_t mask_idx = 0;
    const size_t board_size = _board.size();

    for (size_t i = 0; i < board_size; i++)
    {
        if (_board[i] == '|')
            continue;

        if (!_mask[mask_idx++])
            continue;

        carry = !increment_char_clobber_bw(_board[i]);

        if (!carry)
            break;
    }

    return !carry;
}
*/

//////////////////////////////////////////////////
void test_gen_components2()
{
    //unordered_set<hash_t> expected_hashes = get_hash_set_trivial(5, 5);

    grid_generator_clobber_cc gen(3, 3);

    while (gen)
    {
        cout << gen.get_shape() << endl;
        cout << gen.gen_board() << endl;

        ++gen;
    }

    const unordered_set<hash_t>& got_hashes = gen.get_mask_hashes();

    cout << got_hashes.size() << endl;
    //assert(expected_hashes == got_hashes);
}
