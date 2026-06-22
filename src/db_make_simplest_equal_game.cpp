#include "db_make_simplest_equal_game.h"

#include "sumgame.h"
#include "database.h"
#include "sumgame_helpers.h"
#include <algorithm>
#include <memory>

using namespace std;

////////////////////////////////////////////////// Helpers
namespace {
uint64_t make_size_score(sumgame& sum, database& db)
{
    assert_restore_sumgame ars(sum);
    restore_sumgame_player restore(sum);

    uint64_t max_child_score = 0;
    bool has_moves = false;

    constexpr array<bw, 2> COLORS = {BLACK, WHITE};
    for (const bw color : COLORS)
    {
        sum.set_to_play(color);
        unique_ptr<sumgame_move_generator> gen(
            sum.create_sum_move_generator(color));

        has_moves |= *gen;

        for (; *gen; ++(*gen))
        {
            const sumgame_move sm = gen->gen_sum_move();

            assert(sum.to_play() == color);
            sum.play_sum(sm, color);

            const db_entry_partisan* child_entry = db.get_partisan_ptr(sum);
            THROW_ASSERT(child_entry != nullptr && child_entry->dominated_moves);

            max_child_score = max(max_child_score, child_entry->size_score);

            sum.undo_move();
        }
    }

    return max_child_score + has_moves;
}
} // namespace

////////////////////////////////////////////////// Exported functions
void db_make_simplest_equal_game(sumgame& sum, db_entry_partisan& entry,
                                 database& db)
{
    entry.size_score = make_size_score(sum, db);
}
