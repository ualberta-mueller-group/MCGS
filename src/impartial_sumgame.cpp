//---------------------------------------------------------------------------
// Implementation of impartial sumgame search
//---------------------------------------------------------------------------
#include "impartial_sumgame.h"
#include "timeout_token.h"
#include "utilities.h"

#include <cstddef>
#include <cassert>
#include <iostream>

#include "alternating_move_game.h"
#include "cgt_nimber.h"
#include "game.h"
#include "global_options.h"
#include "impartial_game.h"
#include "impartial_lemoine_viennot.h"
#include "solver_stats.h"
#include "sumgame.h"

namespace {
// mcgs_init::init_impartial_sumgame() and 
// mcgs_init::init_lemoine_viennot_hashtable() 
// must be called first
std::optional<impartial_tt> tt_optional;
std::optional<lemoine_viennot::lv_bool_tt> lv_tt_optional;

int search_impartial(impartial_game* ig, const timeout_token& timeout_tok)
{
    if (global::impartial_algorithm_mex.get())
    {
        impartial_tt& tt = tt_optional.value();
        return ig->search_impartial_game_cancellable(tt, timeout_tok);
    }
    else
    {
        lemoine_viennot::lv_bool_tt& lv_tt = lv_tt_optional.value();
        const int result =
            lemoine_viennot::search_impartial_game(*ig, lv_tt, timeout_tok);
        // stats::print_global_stats(std::cout);
        return result;
    }
}

int search_impartial_sumgame_cancellable(const sumgame& s,
                                         const timeout_token& timeout_tok)
{
    assert_restore_sumgame ar(s);
    int sum_nim_value = 0;

    //stats::inc_node_count();
    stats::report_search_node(s, EMPTY, 0); // TODO depth?

    for (game* g : s.subgames())
    {
        if (timeout_tok.stop_requested())
            return -1;

        if (!g->is_active())
            continue;
        auto ig = static_cast<impartial_game*>(g);
        assert(ig == dynamic_cast<impartial_game*>(g));

        int result = 0;
        if (ig->is_solved())
            result = ig->nim_value();
        else
        {
            result = search_impartial(ig, timeout_tok);

            if (timeout_tok.stop_requested())
                return -1;
            assert(result >= 0);

            // TODO LV should also set is_solved?
            // Or get rid of is_solved ?
            // assert(ig->num_moves_played() > 0 || ig->is_solved());
        }

        assert(result >= 0);
        nimber::add_nimber(sum_nim_value, result);
    }
    return sum_nim_value;
}

} // namespace

int search_impartial_sumgame(const sumgame& s)
{
    timeout_source src;
    timeout_token timeout_tok = src.get_timeout_token();
    src.start_timeout(0);
    int result = search_impartial_sumgame_cancellable(s, timeout_tok);

    assert(!timeout_tok.stop_requested());
    assert(result >= 0);
    return result;
}

std::optional<int> search_impartial_sumgame_with_timeout_token(
    const sumgame& s, const timeout_token& timeout_tok)
{
    assert_restore_sumgame ars(s);

    for (game* g : s.subgames())
        g->normalize();

    const int result = search_impartial_sumgame_cancellable(s, timeout_tok);

    for (game* g : s.subgames())
        g->undo_normalize();

    if (timeout_tok.stop_requested())
        return {};

    assert(result >= 0);
    return result;
}

std::optional<int> search_impartial_sumgame_with_timeout(
    const sumgame& s, unsigned long long timeout)
{
    assert_restore_sumgame ars(s);

    timeout_source src;
    timeout_token timeout_tok = src.get_timeout_token();
    src.start_timeout(timeout);

    return search_impartial_sumgame_with_timeout_token(s, timeout_tok);
}

void init_impartial_sumgame_ttable(size_t idx_bits)
{
    THROW_ASSERT(idx_bits > 0);

    if (global::impartial_algorithm_mex.get())
    {
        assert(!tt_optional.has_value());
        if (global::print_ttable_size())
            std::cout << "Mex-TT ";
        tt_optional.emplace(idx_bits, 0);
    }
    else
    {
        assert(!lv_tt_optional.has_value());
        if (global::print_ttable_size())
            std::cout << "LV-TT ";
        lv_tt_optional.emplace(idx_bits, 0);
    }
}

void clear_impartial_sumgame_ttable()
{
    THROW_ASSERT(global::clear_tt());

    assert(                                             //
        logical_iff(global::impartial_algorithm_mex(),  //
                    tt_optional.has_value()) &&         //
        logical_iff(!global::impartial_algorithm_mex(), //
                    lv_tt_optional.has_value())         //
    );                                                  //

    if (tt_optional.has_value())
        tt_optional->clear();

    if (lv_tt_optional.has_value())
        lv_tt_optional->clear();
}

