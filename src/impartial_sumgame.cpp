//---------------------------------------------------------------------------
// Implementation of impartial sumgame search
//---------------------------------------------------------------------------
#include "impartial_sumgame.h"

#include "cgt_nimber.h"
#include "game.h"
#include "impartial_game.h"
#include "sumgame.h"
#include "alternating_move_game.h"
#include <thread>
#include <future>
#include <chrono>

namespace {

// Calling thread may assign "true" to over_time to stop search
int search_impartial_sumgame_cancellable(const sumgame& s,
                                         const bool& over_time)
{
    assert_restore_alternating_game ar(s);
    int sum_nim_value = 0;

    impartial_tt tt(24, 0);

    for (game* g : s.subgames())
    {
        if (over_time)
            return -1;

        if (! g->is_active())
            continue;
        auto ig = static_cast<impartial_game*>(g);
        int result = 0;
        if (ig->is_solved())
            result = ig->nim_value();
        else
        {
            result = ig->search_impartial_game_cancellable(tt, over_time);

            if (over_time)
                return -1;
            assert(result >= 0);

            assert(ig->num_moves_played() > 0 || ig->is_solved());
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

std::optional<int> search_impartial_sumgame_with_timeout(
    const sumgame& s, unsigned long long timeout)
{
    bool over_time = false;

    std::promise<int> promise;
    std::future<int> future = promise.get_future();

    std::thread thr(
        [&]() -> void
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

    assert(future.valid());

    if (over_time)
        return std::optional<int>();

    int value = future.get();
    assert(value >= 0);
    return std::optional<int>(value);
}
