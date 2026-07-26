//---------------------------------------------------------------------------
// Implementation of impartial sumgame search
//---------------------------------------------------------------------------
#include "impartial_sumgame.h"
#include "iobuffer.h"
#include "timeout_token.h"
#include "utilities.h"

#include <cstddef>
#include <cassert>
#include <iostream>
#include <cstdint>

#include "alternating_move_game.h"
#include "cgt_nimber.h"
#include "game.h"
#include "global_options.h"
#include "impartial_game.h"
#include "impartial_lemoine_viennot.h"
#include "solver_stats.h"
#include "throw_assert.h"
#include "sumgame.h"
#include "transposition_serializer.h"

using namespace std;

namespace {
// mcgs_init::init_impartial_sumgame() and 
// mcgs_init::init_lemoine_viennot_hashtable() 
// must be called first
optional<impartial_tt> tt_optional;
optional<lemoine_viennot::lv_bool_tt> lv_tt_optional;

int search_impartial(impartial_game* ig, const timeout_token& timeout_tok, uint64_t depth)
{
    if (global::impartial_algorithm_mex.get())
    {
        assert(tt_optional.has_value());
        impartial_tt& tt = tt_optional.value();

        return ig->search_impartial_game_cancellable(tt, timeout_tok, depth);
    }
    else
    {
        assert(lv_tt_optional.has_value());
        lemoine_viennot::lv_bool_tt& lv_tt = lv_tt_optional.value();

        const int result =
            lemoine_viennot::search_impartial_game(*ig, lv_tt, timeout_tok, depth);
        // stats::print_global_stats(cout);
        return result;
    }
}

int search_impartial_sumgame_cancellable(const sumgame& s,
                                         const timeout_token& timeout_tok, uint64_t depth)
{
    assert_restore_sumgame ar(s);
    int sum_nim_value = 0;

    stats::report_search_node(s, EMPTY, depth);
    // No "next_depth = depth + 1" -- no move is played here

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
            result = search_impartial(ig, timeout_tok, depth);

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
    // Not interruptible
    assert(!exit_signal::handlers_are_enabled());

    timeout_source src;
    timeout_token timeout_tok = src.get_timeout_token();
    src.start_timeout(0);
    const optional<int> result_opt =
        search_impartial_sumgame_with_timeout_token(s, timeout_tok,
                                                    INITIAL_SEARCH_DEPTH);

    assert(!timeout_tok.stop_requested() && result_opt.has_value());
    assert(*result_opt >= 0);
    return *result_opt;
}

optional<int> search_impartial_sumgame_with_timeout_token(
    const sumgame& s, const timeout_token& timeout_tok, uint64_t depth)
{
    assert_restore_sumgame ars(s);

    for (game* g : s.subgames())
        g->normalize();

    const int result = search_impartial_sumgame_cancellable(s, timeout_tok, depth);

    for (game* g : s.subgames())
        g->undo_normalize();

    if (timeout_tok.stop_requested())
        return {};

    assert(result >= 0);
    return result;
}

optional<int> search_impartial_sumgame_with_timeout(
    const sumgame& s, unsigned long long timeout)
{
    assert_restore_sumgame ars(s);

    timeout_source src;
    timeout_token timeout_tok = src.get_timeout_token();
    src.start_timeout(timeout);

    return search_impartial_sumgame_with_timeout_token(s, timeout_tok,
                                                       INITIAL_SEARCH_DEPTH);
}

void init_impartial_sumgame_ttable(size_t idx_bits,
                                   const string& ttable_load_file_name)
{
    THROW_ASSERT(idx_bits > 0);

    if (ttable_load_file_name.empty())
    {
        if (global::impartial_algorithm_mex.get())
        {
            assert(!tt_optional.has_value());
            if (global::print_ttable_size())
                cout << "Mex-TT ";
            tt_optional.emplace(idx_bits, 0);
        }
        else
        {
            assert(!lv_tt_optional.has_value());
            if (global::print_ttable_size())
                cout << "LV-TT ";
            lv_tt_optional.emplace(idx_bits, 0);
        }
    }
    else
    {
        cout << "Loading impartial ttable \"" << ttable_load_file_name
             << "\"..." << flush;

        file_ibuffer is(ttable_load_file_name);
        const bool ttable_is_mex = is.read_bool();

        if (ttable_is_mex != global::impartial_algorithm_mex()) [[unlikely]]
        {
            const string file_algorithm = ttable_is_mex ? "Mex" : "LV";
            const string runtime_algorithm =
                global::impartial_algorithm_mex() ? "Mex" : "LV";

            throw logic_error(
                "Attempted to load impartial ttable from file, and got "
                "algorithm mismatch. Runtime algorithm: " +
                runtime_algorithm + " File algorithm: " + file_algorithm);
        }

        size_t new_idx_bits;

        if (ttable_is_mex)
        {
            tt_optional.emplace(serializer<impartial_tt>::load(is, nullptr));
            new_idx_bits = tt_optional->n_index_bits();
        }
        else
        {
            lv_tt_optional.emplace(
                serializer<lemoine_viennot::lv_bool_tt>::load(is, nullptr));

            new_idx_bits = lv_tt_optional->n_index_bits();
        }

        global::tt_imp_sumgame_idx_bits.set(new_idx_bits);
        cout << " DONE (has " << new_idx_bits << " index bits)." << endl;
    }
}

void save_impartial_sumgame_ttable(const string& ttable_save_file_name)
{
    cout << "Saving impartial ttable \"" << ttable_save_file_name << "\"..."
         << flush;

    file_obuffer os(ttable_save_file_name);

    if (global::impartial_algorithm_mex())
    {
        assert(tt_optional.has_value());

        os.write_bool(true);
        serializer<impartial_tt>::save(os, *tt_optional, nullptr);
    }
    else
    {
        assert(lv_tt_optional.has_value());

        os.write_bool(false);
        serializer<lemoine_viennot::lv_bool_tt>::save(os, *lv_tt_optional,
                                                      nullptr);
    }

    os.close();

    cout << " OK " << endl;
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

