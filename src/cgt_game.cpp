#include "cgt_game.h"

#include "cgt_basics.h"
#include "cgt_move.h"
#include "game.h"
#include "integral_conversion.h"
#include "throw_assert.h"


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
    vector<hash_t> elements;

    get_flat_hash_elements(elements);

    global_hash gh;
    gh.set_to_play(EMPTY);

    const size_t elements_size = elements.size();
    for (size_t i = 0; i < elements_size; i++)
        gh.add_hash(i, elements[i]);

    return gh.get_value();
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

void game_sum::_init_hash(local_hash& hash) const
{
    const hash_t lh_val = hash.get_value();
    const hash_t flat_hash_val = get_flat_hash_value();
    hash.__set_value(lh_val ^ flat_hash_val);
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

void cgt_game::_init_hash(local_hash& hash) const
{
    const hash_t lh_val = hash.get_value();
    const hash_t flat_hash_val = get_flat_hash_value();
    hash.__set_value(lh_val ^ flat_hash_val);
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
