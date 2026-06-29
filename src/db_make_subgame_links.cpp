#include "db_make_subgame_links.h"

#include "sumgame.h"
#include "database.h"

using namespace std;

void db_make_subgame_links(const sumgame& sum, db_entry_partisan& entry,
                           database& db)
{
    assert(entry.subgame_links.empty());
    assert(entry.subgame_hashes.empty());

    const int n_games = sum.num_total_games();

    for (int i = 0; i < n_games; i++)
    {
        const game* g = sum.subgame_const(i);
        if (!g->is_active())
            continue;

        pair<const hash_t, db_entry_partisan>* entry_ptr =
            db.get_partisan_ptr_pair(*g);

        THROW_ASSERT(entry_ptr != nullptr);

        db_link_t link;
        link.set_as_pointer(entry_ptr);
        entry.subgame_links.push_back(link);

        entry.subgame_hashes.push_back(g->get_local_hash());
    }
}
