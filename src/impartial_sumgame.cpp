//---------------------------------------------------------------------------
// Implementation of impartial sumgame search
//---------------------------------------------------------------------------
#include "impartial_sumgame.h"

#include <chrono>
#include <cstddef>
#include <cassert>

#ifndef __EMSCRIPTEN__
#include <thread>
#include <future>
#endif

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

// Calling thread may assign "true" to over_time to stop search
int search_impartial_sumgame_cancellable(const sumgame& s,
                                         const bool& over_time)
{
    assert_restore_sumgame ar(s);
    int sum_nim_value = 0;

    impartial_tt& tt = tt_optional.value();
    lemoine_viennot::lv_bool_tt& lv_tt = lv_tt_optional.value();

    stats::inc_node_count();

    for (game* g : s.subgames())
    {
        if (over_time)
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
            if (global::alt_imp_search.get())
                result = search_impartial_game(*ig, lv_tt);
            else
                result = ig->search_impartial_game_cancellable(tt, over_time);

            if (over_time)
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
    int result = search_impartial_sumgame_cancellable(s, false);
    assert(result >= 0);
    return result;
}

#ifndef __EMSCRIPTEN__
std::optional<int> search_impartial_sumgame_with_timeout(
    const sumgame& s, unsigned long long timeout)
{
    bool over_time = false;

    assert_restore_sumgame ars(s);
    for (game* g : s.subgames())
        g->normalize();

    std::promise<int> promise;
    std::future<int> future = promise.get_future();

    std::thread thr([&]() -> void
    {
        int result = search_impartial_sumgame_cancellable(s, over_time);
        promise.set_value(result);
    });

    std::future_status status = std::future_status::ready;

    if (timeout == 0)
        future.wait();
    else
        status = future.wait_for(std::chrono::milliseconds(timeout));

    if (status == std::future_status::timeout)
        over_time = true;

    future.wait();
    thr.join();

    for (game* g : s.subgames())
        g->undo_normalize();

    assert(future.valid());

    if (over_time)
        return std::optional<int>();

    int value = future.get();
    assert(value >= 0);
    return std::optional<int>(value);
}
#else
// TODO emscripten pthreads
std::optional<int> search_impartial_sumgame_with_timeout(
    const sumgame& s, unsigned long long timeout)
{
    bool over_time = false;

    assert_restore_sumgame ars(s);

    for (game* g : s.subgames())
        g->normalize();

    int result = search_impartial_sumgame_cancellable(s, over_time);

    for (game* g : s.subgames())
        g->undo_normalize();

    return result;
}
#endif

void init_impartial_sumgame_ttable(size_t idx_bits)
{
    assert(idx_bits > 0);
    assert(!tt_optional.has_value());
    tt_optional.emplace(idx_bits, 0);

    assert(!lv_tt_optional.has_value());
    lv_tt_optional.emplace(idx_bits, 0);
}
