#include "cgt_game.h"

#include "cgt_basics.h"
#include "cgt_move.h"
#include "game.h"
#include "hashing.h"
#include "integral_conversion.h"
#include "parsing_utilities.h"
#include "throw_assert.h"
#include "utilities.h"
#include "utils_for_main.h"
#include <algorithm>
#include <cstdlib>
#include <sstream>


borrowable<std::vector<hash_t>> i_cgt_game_base::_hash_elements_helper;
borrowable<global_hash> i_cgt_game_base::_global_hash_helper;

////////////////////////////////////////////////// Testing stuff
//namespace {
//std::unordered_map<hash_t, hash_t> cgt_hash_map;
//
//hash_t get_game_string_hash(const game* g)
//{
//    assert(g);
//    local_hash lh;
//
//    std::stringstream strstr;
//    strstr << *g;
//    const std::string& str = strstr.str();
//
//    const size_t str_size = str.size();
//    for (size_t i = 0; i < str_size; i++)
//        lh.toggle_value(i, str[i]);
//
//    return lh.get_value();
//}
//
//void test_hash_collision(const game* g, hash_t game_hash)
//{
//    assert(g);
//
//    const hash_t string_hash = get_game_string_hash(g);
//
//    const auto& inserted = cgt_hash_map.try_emplace(game_hash, string_hash);
//    const bool game_hash_collision = !inserted.second;
//    const bool string_hash_collision = (string_hash == inserted.first->second);
//
//    assert(LOGICAL_IMPLIES(game_hash_collision, string_hash_collision));
//}
//} // namespace


////////////////////////////////////////////////// class game_sum_move_generator
class game_sum_move_generator: public move_generator
{
public:
    game_sum_move_generator(bw to_play);

    void operator++() override;
    operator bool() const override;
    move gen_move() const override;
};

////////////////////////////////////////////////// class cgt_game_move_generator
class cgt_game_move_generator: public move_generator
{
public:
    cgt_game_move_generator(const cgt_game& g, bw to_play);

    void operator++() override;
    operator bool() const override;
    move gen_move() const override;

private:
    size_t _move_idx;
    size_t _n_options;

};

//////////////////////////////////////////////////
using namespace std;

////////////////////////////////////////////////// i_cgt_game_base methods

void i_cgt_game_base::get_flat_hash_elements_single(vector<hash_t>& elements,
                                                    const game* g)
{
    const i_cgt_game_base* base = dynamic_cast<const i_cgt_game_base*>(g);

    if (base)
        base->get_flat_hash_elements(elements);
    else
        elements.push_back(g->get_local_hash());
}

hash_t i_cgt_game_base::get_flat_hash_value() const
{
    vector<hash_t>* hash_elements = _hash_elements_helper.borrow();
    assert(hash_elements);

    hash_elements->clear();
    get_flat_hash_elements(*hash_elements);

    global_hash* gh = _global_hash_helper.borrow();
    assert(gh);

    gh->set_to_play(EMPTY);

    const size_t elements_size = hash_elements->size();
    for (size_t i = 0; i < elements_size; i++)
        gh->add_hash(i, (*hash_elements)[i]);

    const hash_t val = gh->get_value();

    hash_elements->clear();
    gh->reset();

    _hash_elements_helper.unborrow(hash_elements);
    _global_hash_helper.unborrow(gh);

    return val;
}

////////////////////////////////////////////////// game_sum methods
game_sum::game_sum(vector<shared_ptr<const game>> games)
    : _games(games)
{
    for (const shared_ptr<const game>& g : games)
        assert(g && g->is_active());
}

void game_sum::play(const ::move& m, bw to_play)
{
    THROW_ASSERT(false);
}

void game_sum::undo_move()
{
    THROW_ASSERT(false);
}

::move game_sum::encode_grid_move_to_db(const ::move& m) const
{
    THROW_ASSERT(false);
}

::move game_sum::decode_grid_move_from_db(const ::move& m) const
{
    THROW_ASSERT(false);
}

void game_sum::print(ostream& str) const
{
    str << "(";

    bool first = true;
    for (const shared_ptr<const game>& g : _games)
    {
        assert(g && g->is_active());

        if (!first)
            str << "+";
        first = false;

        g->print_simple(str);
    }

    str << ")";
}

void game_sum::print_move(ostream& str, const ::move& m, ebw to_play) const
{
    THROW_ASSERT(false);
}

game* game_sum::inverse() const
{
    vector<shared_ptr<const game>> inv_games;
    inv_games.reserve(_games.size());

    for (const shared_ptr<const game>& g : _games)
        inv_games.emplace_back(g->inverse());

    return new game_sum(inv_games);
}

game* game_sum::clone() const
{
    return new game_sum(_games);
}

void game_sum::get_flattened_operands(
    vector<shared_ptr<const game>>& operands) const
{
    const game_type_t game_sum_type_id = ::game_type<game_sum>();

    for (const shared_ptr<const game>& g : _games)
    {
        assert(g && g->is_active());

        if (g->game_type() != game_sum_type_id)
        {
            operands.push_back(g);
            continue;
        }

        const game_sum* g_casted = dynamic_cast<const game_sum*>(g.get());
        assert(g_casted != nullptr);

        g_casted->get_flattened_operands(operands);
    }
}

void game_sum::get_flat_hash_elements(vector<hash_t>& elements) const
{
    elements.push_back(integral_cast_checked<hash_t>(game_type()));
    elements.push_back(integral_cast_checked<hash_t>(_games.size()));

    for (const shared_ptr<const game>& g : _games)
        get_flat_hash_elements_single(elements, g.get());
}

move_generator* game_sum::_create_move_generator_impl(bw to_play) const
{
    return new game_sum_move_generator(to_play);
}

split_result game_sum::_split_impl() const
{
    split_result sr(vector<game*>{});
    assert(sr.has_value());

    vector<shared_ptr<const game>> flat_operands;
    get_flattened_operands(flat_operands);

    for (const shared_ptr<const game>& g : flat_operands)
        sr->push_back(g->clone());

    return sr;
}

//void game_sum::_init_hash(local_hash& hash) const
//{
//    const hash_t lh_val = hash.get_value();
//    const hash_t flat_hash_val = get_flat_hash_value();
//    hash.__set_value(lh_val ^ flat_hash_val);
//}

void game_sum::_init_hash(local_hash& hash) const
{
    // 1st pass: compute each operand's local hash and discard it (to avoid
    // unnecessary memory allocation i.e. using std::vector)
    for (const shared_ptr<const game>& g : _games)
    {
        assert(g && g->is_active());
        g->get_local_hash();
    }

    // 2nd pass: combine previously computed local hashes (these aren't
    // recomputed here because the `game` base class has them cached from the
    // 1st pass)
    const size_t n_operands = _games.size();

    // IMPORTANT: Only write to _gh after all operand hashes are computed (each
    // operand hash computation might also write to _gh)!
    global_hash* gh = _global_hash_helper.borrow();
    assert(gh);

    gh->reset();
    gh->set_to_play(EMPTY);
    for (size_t i = 0; i < n_operands; i++)
        gh->add_hash(i, _games[i]->get_local_hash());

    const hash_t gh_val = gh->get_value();
    _global_hash_helper.unborrow(gh);

    hash.toggle_value(0, gh_val);

    //test_hash_collision(this, hash.get_value());
}

////////////////////////////////////////////////// cgt_game methods
cgt_game::cgt_game(std::vector<std::shared_ptr<const game>> left_options,
                   std::vector<std::shared_ptr<const game>> right_options)
    : _left_options(left_options), _right_options(right_options)
{
    for (const shared_ptr<const game>& g : _left_options)
        assert(g && g->is_active());

    for (const shared_ptr<const game>& g : _right_options)
        assert(g && g->is_active());
}

void cgt_game::play(const ::move& m, bw to_play)
{
    assert(!_selected);
    game::play(m, to_play);

    const move_part idx_as_part = cgt_move::move1_get_part_1(m);
    const size_t idx = integral_cast_checked<size_t>(idx_as_part);

    const vector<shared_ptr<const game>>& options =
        get_player_option_set(to_play);

    assert(idx < options.size());
    _selected = options[idx];
}

void cgt_game::undo_move()
{
    assert(_selected);
    const ::move m_enc = last_move();
    game::undo_move();

    const bw to_play = cgt_move::get_color(m_enc);
    const move_part idx_as_part = cgt_move::move1_get_part_1(m_enc);

    assert(is_black_white(to_play));
    const size_t idx = integral_cast_checked<size_t>(idx_as_part);

    const vector<shared_ptr<const game>>& options = get_player_option_set(to_play);
    assert(idx < options.size());

    _selected = nullptr;
}

void cgt_game::print(std::ostream& str) const
{
    str << "{";

    if (_selected)
        _selected->print_simple(str);
    else
    {
        _print_option_set(str, _left_options);
        str << "|";
        _print_option_set(str, _right_options);
    }

    str << "}";
}

void cgt_game::print_move(std::ostream& str, const ::move& m, ebw to_play) const
{
    assert(is_black_white(to_play));
    const char prefix = (to_play == BLACK) ? 'L' : 'R';

    const move_part idx_as_part = cgt_move::move1_get_part_1(m);
    const size_t idx = integral_cast_checked<size_t>(idx_as_part);

    str << prefix << '_' << idx;
}

game* cgt_game::inverse() const
{
    if (_selected)
        return new cgt_game(shared_ptr<const game>(_selected->inverse()));

    vector<shared_ptr<const game>> left_inv;
    left_inv.reserve(_left_options.size());

    vector<shared_ptr<const game>> right_inv;
    right_inv.reserve(_right_options.size());

    for (const shared_ptr<const game>& g : _left_options)
        left_inv.emplace_back(g->inverse());

    for (const shared_ptr<const game>& g : _right_options)
        right_inv.emplace_back(g->inverse());

    return new cgt_game(right_inv, left_inv);
}

game* cgt_game::clone() const
{
    if (_selected)
        return new cgt_game(_selected);
    else
        return new cgt_game(_left_options, _right_options);
}

void cgt_game::get_flat_hash_elements(std::vector<hash_t>& elements) const
{
    elements.push_back(integral_cast_checked<hash_t>(game_type()));

    size_t size1 = _left_options.size();
    size_t size2 = _right_options.size();
    
    if (_selected)
    {
        size1 = numeric_limits<size_t>::max();
        size2 = numeric_limits<size_t>::max();
    }

    elements.push_back(integral_cast_checked<hash_t>(size1));
    elements.push_back(integral_cast_checked<hash_t>(size2));

    if (_selected)
        get_flat_hash_elements_single(elements, _selected.get());
    else
    {
        for (const shared_ptr<const game>& g : _left_options)
            get_flat_hash_elements_single(elements, g.get());
        for (const shared_ptr<const game>& g : _right_options)
            get_flat_hash_elements_single(elements, g.get());
    }
}

const shared_ptr<const game>& cgt_game::get_selected_option() const
{
    return _selected;
}

const vector<shared_ptr<const game>>& cgt_game::get_player_option_set(
    bw player) const
{
    assert(is_black_white(player));
    return (player == BLACK) ? _left_options : _right_options;
}

void cgt_game::_print_option_set(ostream& str,
                                 const vector<shared_ptr<const game>>& options)
{
    bool first = true;

    for (const shared_ptr<const game>& g : options)
    {
        assert(g && g->is_active());

        if (!first)
            str << ",";
        first = false;

        g->print_simple(str);
    }
}

move_generator* cgt_game::_create_move_generator_impl(bw to_play) const
{
    return new cgt_game_move_generator(*this, to_play);
}

split_result cgt_game::_split_impl() const
{
    if (!_selected)
        return {};

    if (_selected->game_type() == ::game_type<game_sum>())
    {
        const game_sum* g_casted = dynamic_cast<const game_sum*>(_selected.get());
        assert(g_casted != nullptr);

        return g_casted->split();
    }

    split_result sr(vector<game*>{});
    assert(sr.has_value());

    sr->push_back(_selected->clone());
    return sr;
}

//void cgt_game::_init_hash(local_hash& hash) const
//{
//    const hash_t lh_val = hash.get_value();
//    const hash_t flat_hash_val = get_flat_hash_value();
//    hash.__set_value(lh_val ^ flat_hash_val);
//}

void cgt_game::_init_hash(local_hash& hash) const
{
    global_hash* gh = nullptr;
    hash_t gh_val;

    if (_selected)
    {
        assert(_selected->is_active());
        const hash_t selected_hash = _selected->get_local_hash();

        gh = _global_hash_helper.borrow();
        assert(gh);

        // IMPORTANT: Must compute `_selected`'s local hash before writing to
        // _gh!
        gh->reset();
        gh->set_to_play(EMPTY);
        gh->add_hash(0, 0); // location 0 is 0 IFF an option is selected
        gh->add_hash(1, selected_hash);

        gh_val = gh->get_value();
    }
    else
    {
        // 1st pass: compute each operand's local hash and discard it (to avoid
        // unnecessary memory allocation i.e. using std::vector)
        for (const shared_ptr<const game>& g : _left_options)
        {
            assert(g && g->is_active());
            g->get_local_hash();
        }

        for (const shared_ptr<const game>& g : _right_options)
        {
            assert(g && g->is_active());
            g->get_local_hash();
        }

        // 2nd pass: combine previously computed local hashes (these aren't
        // recomputed here because the `game` base class has them cached from
        // the 1st pass)
        const size_t n_left = _left_options.size();
        const size_t n_right = _right_options.size();

        // IMPORTANT: Only write to _gh after all operand hashes are computed
        // (each operand hash computation might also write to _gh)!
        gh = _global_hash_helper.borrow();
        assert(gh);

        gh->reset();
        gh->set_to_play(EMPTY);
        gh->add_hash(0, 1); // location 0 is 1 IFF no option is selected
        gh->add_hash(1, n_left);
        gh->add_hash(2, n_right);

        size_t idx = 3;
        for (const shared_ptr<const game>& g : _left_options)
            gh->add_hash(idx++, g->get_local_hash());

        for (const shared_ptr<const game>& g : _right_options)
            gh->add_hash(idx++, g->get_local_hash());

        gh_val = gh->get_value();
    }

    _global_hash_helper.unborrow(gh);

    hash.toggle_value(0, gh_val);

    //test_hash_collision(this, hash.get_value());
}


cgt_game::cgt_game(std::shared_ptr<const game> selected)
    : _selected(selected)
{
    assert(_selected && _selected->is_active());
}

//////////////////////////////////////////////////
// game_sum_move_generator methods

game_sum_move_generator::game_sum_move_generator(bw to_play)
    : move_generator(to_play)
{
    assert(is_black_white(to_play));
}

void game_sum_move_generator::operator++()
{
    THROW_ASSERT(false);
}

game_sum_move_generator::operator bool() const
{
    return true;
}

::move game_sum_move_generator::gen_move() const
{
    THROW_ASSERT(false);
}

//////////////////////////////////////////////////
// cgt_game_move_generator methods
cgt_game_move_generator::cgt_game_move_generator(const cgt_game& g, bw to_play)
    : move_generator(to_play)
{
    assert(is_black_white(to_play));

    _move_idx = 0;
    _n_options = 0;

    if (!g.get_selected_option())
    {
        const vector<shared_ptr<const game>>& options =
            g.get_player_option_set(to_play);

        _n_options = options.size();
    }
}

void cgt_game_move_generator::operator++()
{
    assert(*this);
    ++_move_idx;
}

cgt_game_move_generator::operator bool() const
{
    return _move_idx < _n_options;
}

::move cgt_game_move_generator::gen_move() const
{
    assert(*this);

    const move_part idx_as_part = integral_cast_checked<move_part>(_move_idx);
    return cgt_move::move1_create(idx_as_part);
}
