#include "db_make_simplest_equal_game.h"

#include <algorithm>
#include <filesystem>
#include <memory>
#include <type_traits>
#include <utility>

#include "bounds.h"
#include "cgt_basics.h"
#include "clobber_1xn.h"
#include "db_link_t.h"
#include "all_game_headers.h"
#include "game.h"
#include "global_options.h"
#include "grid.h"
#include "hashing.h"
#include "safe_arithmetic.h"
#include "size_score_enum.h"
#include "strip.h"
#include "sumgame.h"
#include "database.h"
#include "sumgame_helpers.h"
#include "thermograph_helpers.h"
#include "throw_assert.h"
#include "type_table.h"

using namespace std;

uint64_t db_n_links_refined = 0;

namespace {
////////////////////////////////////////////////// Helper types
typedef hash_t seg_map_idx_t;

class link_compare
{
public:
    link_compare(database& db);

    relation compare(const db_link_t& link1,
                     const db_link_t& link2) const noexcept;

    bool operator()(const db_link_t& link1,
                    const db_link_t& link2) const noexcept;

private:
    database& _db;
};

class equivalence_class
{
public:
    bool is_empty() const;
    void assert_correctly_sorted(database& db) const;

    db_link_t get_representative() const;
    void insert_link(db_link_t new_link, database& db);

    db_link_t get_best_link_for_game(db_link_t game_link, database& db) const;

private:
    db_link_t _representative;
    std::vector<db_link_t> _all_links;
};

////////////////////////////////////////////////// Helper type methods
//////////////////////////////////////// link_compare methods
link_compare::link_compare(database& db)
    : _db(db)
{
}

relation link_compare::compare(const db_link_t& link1,
                               const db_link_t& link2) const noexcept
{
    const db_entry_partisan* entry1 = _db.get_partisan_ptr(link1);
    const db_entry_partisan* entry2 = _db.get_partisan_ptr(link2);
    assert(entry1 != nullptr && entry2 != nullptr);

    if (entry1->complexity != entry2->complexity)
        return entry1->complexity < entry2->complexity ? REL_LESS : REL_GREATER;

    return entry1->compare_size_score(*entry2);
}

bool link_compare::operator()(const db_link_t& link1,
                              const db_link_t& link2) const noexcept
{
    return compare(link1, link2) == REL_LESS;
}

//////////////////////////////////////// equivalence_class methods
bool equivalence_class::is_empty() const
{
    return _all_links.empty();
}

void equivalence_class::assert_correctly_sorted(database& db) const
{
    link_compare cmp(db);
    assert(is_sorted(_all_links.begin(), _all_links.end(), cmp));
}

db_link_t equivalence_class::get_representative() const
{
    assert(!is_empty());
    return _representative;
}

void equivalence_class::insert_link(db_link_t new_link, database& db)
{
    if (is_empty())
    {
        _representative = new_link;
        _all_links.push_back(new_link);
        return;
    }

    link_compare cmp(db);

    if (cmp(new_link, _representative))
        _representative = new_link;

    const size_t N_LINKS = _all_links.size();
    assert(N_LINKS > 0);

    size_t low = 0;
    size_t high = N_LINKS - 1;

    size_t mid;
    relation link_relation = REL_UNKNOWN;

    assert(low <= high);
    while (low <= high)
    {
        THROW_ASSERT(add_is_safe(low, high));
        mid = (low + high) / 2;

        db_link_t link_mid = _all_links[mid];
        link_relation = cmp.compare(new_link, link_mid);

        if (link_relation == REL_EQUAL)
            break;
        else if (link_relation == REL_LESS)
        {
            // Too high
            if (mid == 0)
                break;
            high = mid - 1;
        }
        else if (link_relation == REL_GREATER)
        {
            // Too low
            low = mid + 1;
        }
        else
            assert(false);
    }

    if (link_relation == REL_LESS || link_relation == REL_GREATER)
    {
        const size_t new_idx = mid + (link_relation == REL_GREATER);
        _all_links.insert(_all_links.begin() + new_idx, new_link);
    }

    assert_correctly_sorted(db);
}

db_link_t equivalence_class::get_best_link_for_game(db_link_t game_link,
                                                    database& db) const
{
    const db_entry_partisan* game_entry = db.get_partisan_ptr(game_link);
    assert(game_entry != nullptr);

    for (const db_link_t& link : _all_links)
    {
        const db_entry_partisan* linked_entry = db.get_partisan_ptr(link);
        assert(linked_entry != nullptr);

        if (game_entry->complexity < linked_entry->complexity)
            return game_link;

        //if (game_entry->size_score > linked_entry->size_score)
        //if (game_entry->compare_size_score(*linked_entry) == REL_GREATER)
        if (game_entry->size_score_allows_replacement_by(*linked_entry))
            return link;
    }

    return game_link;
}

////////////////////////////////////////////////// Helper data
unordered_map<seg_map_idx_t, vector<equivalence_class>> seg_map;
unordered_map<hash_t, size_t> global_hash_to_seg_vec_idx;

////////////////////////////////////////////////// Helper functions
seg_map_idx_t make_seg_map_index(const db_entry_partisan& entry)
{
#warning TODO do equal games always have the same thermographs?

    THROW_ASSERT(                       //
        entry.thermograph &&            //
        entry.bounds_data &&            //
        entry.bounds_data->both_valid() //
    );

    const ThGraph& graph = *entry.thermograph;
    const game_bounds& bounds = *entry.bounds_data;

    local_hash h;

    using bound_scale_int_t = std::underlying_type_t<bound_scale>;
    using relation_int_t = std::underlying_type_t<relation>;

    h.toggle_value(0, get_thermograph_hash(graph));

    h.toggle_value(1, static_cast<bound_scale_int_t>(bounds.get_scale()));

    h.toggle_value(2, static_cast<relation_int_t>(bounds.get_lower_relation()));
    h.toggle_value(3, bounds.get_lower());

    h.toggle_value(4, static_cast<relation_int_t>(bounds.get_upper_relation()));
    h.toggle_value(5, bounds.get_upper());

    return h.get_value();
}

uint64_t ss_tree_height(sumgame& sum, db_entry_partisan& entry, database& db)
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

            max_child_score = max(max_child_score, child_entry->ss_tree_height);

            sum.undo_move();
        }
    }

    return max_child_score + has_moves;
}

uint64_t ss_max_local_options(sumgame& sum, db_entry_partisan& entry, database& db)
{
    assert_restore_sumgame ars(sum);
    restore_sumgame_player restore(sum);

    uint64_t largest = 0;
    uint64_t n_local = 0;

    constexpr array<bw, 2> COLORS = {BLACK, WHITE};
    for (const bw color : COLORS)
    {
        sum.set_to_play(color);
        unique_ptr<sumgame_move_generator>
            gen(sum.create_sum_move_generator(color));

        while (*gen)
        {
            n_local++;

            const sumgame_move sm = gen->gen_sum_move();
            ++(*gen);

            sum.play_sum(sm, color);

            db_entry_partisan* child_entry = db.get_partisan_ptr(sum);
            largest = max(largest, child_entry->ss_max_local_options);

            sum.undo_move();
        }
    }

    return max(largest, n_local);
}

uint64_t count_color(const game* g, int color, bool invert)
{
    uint64_t total = 0;

    const vector<int>* board = nullptr;

    const strip* g_strip = dynamic_cast<const strip*>(g);
    if (g_strip != nullptr)
        board = &g_strip->board_const();

    const grid* g_grid = dynamic_cast<const grid*>(g);
    if (g_grid != nullptr)
        board = &g_grid->board_const();

    assert(board != nullptr);

    for (const int tile : *board)
    {
        if (invert)
            total += (tile != color);
        else
            total += (tile == color);
    }

    return total;
}

uint64_t ss_stone_count(sumgame& sum, db_entry_partisan& entry, database& db)
{
    uint64_t total = 0;

    const int n_games = sum.num_total_games();
    for (int i = 0; i < n_games; i++)
    {
        game* g = sum.subgame(i);
        if (!g->is_active())
            continue;

        const game_type_t gt = g->game_type();

        if (gt == game_type<clobber_1xn>() || gt == game_type<clobber>())
        {
            total += count_color(g, EMPTY, true);
            continue;
        }

        if (gt == game_type<domineering>() || //
            gt == game_type<elephants>() ||   //
            gt == game_type<nogo_1xn>() ||    //
            gt == game_type<amazons>()        //
        )

        {
            total += count_color(g, EMPTY, false);
            continue;
        }

        assert(false);
    }

    return total;
}

uint64_t ss_board_size(sumgame& sum, db_entry_partisan& entry, database& db)
{
    uint64_t total = 0;

    const int n_games = sum.num_total_games();
    for (int i = 0; i < n_games; i++)
    {
        game* g = sum.subgame(i);
        if (!g->is_active())
            continue;

        strip* g_strip = dynamic_cast<strip*>(g);
        if (g_strip != nullptr)
        {
            total += g_strip->size();
            continue;
        }

        grid* g_grid = dynamic_cast<grid*>(g);
        if (g_grid != nullptr)
        {
            const int_pair shape = g_grid->shape();
            total += shape.first * shape.second;
            continue;
        }

        THROW_ASSERT(false);
    }

    return total;
}

uint64_t ss_node_count_naive(sumgame& sum, db_entry_partisan& entry, database& db)
{
    assert_restore_sumgame ars(sum);
    restore_sumgame_player restore(sum);

    uint64_t total = 1;
    bool add_ok = true;

    constexpr array<bw, 2> COLORS = {BLACK, WHITE};
    for (const bw color : COLORS)
    {
        sum.set_to_play(color);
        unique_ptr<sumgame_move_generator> gen(
            sum.create_sum_move_generator(color));

        for (; *gen; ++(*gen))
        {
            const sumgame_move sm = gen->gen_sum_move();
            sum.play_sum(sm, color);

            db_entry_partisan* child_entry = db.get_partisan_ptr(sum);
            THROW_ASSERT(child_entry != nullptr);

            add_ok &= safe_add(total, child_entry->ss_node_count);

            sum.undo_move();
        }
    }

    THROW_ASSERT(add_ok);
    return total;
}

optional<size_t> find_sum_equivalence_class(sumgame& sum, const vector<equivalence_class>& classes, database& db)
{
    assert_restore_sumgame ars(sum);
    restore_sumgame_player restore(sum);

    optional<size_t> class_idx;

    const size_t N_CLASSES = classes.size();
    for (size_t i = 0; i < N_CLASSES; i++)
    {
        const equivalence_class& eq_class = classes[i];
        assert(!eq_class.is_empty());

        const db_link_t& link = eq_class.get_representative();
        const db_entry_partisan* linked_entry = db.get_partisan_ptr(link);
        assert(linked_entry != nullptr);

        vector<game*> linked_games = linked_entry->load_sum();

        vector<game*> inverse_linked_games;
        inverse_linked_games.reserve(linked_games.size());
        for (game* g : linked_games)
            inverse_linked_games.push_back(g->inverse());

        sum.add(inverse_linked_games);

        if (sum_rel_zero(sum, REL_EQUAL))
            class_idx = i;

        sum.pop(inverse_linked_games);

        for (game* g : linked_games)
            delete g;
        for (game* g : inverse_linked_games)
            delete g;

        if (class_idx.has_value())
            return class_idx;
    }

    assert(!class_idx.has_value());
    return class_idx;
}

const equivalence_class* get_previous_equivalence_class(
    hash_t sum_hash, const db_entry_partisan& entry)
{
    const seg_map_idx_t seg_idx = make_seg_map_index(entry);

    const auto vec_iterator = seg_map.find(seg_idx);
    if (vec_iterator == seg_map.end())
        return nullptr;
    const vector<equivalence_class>& vec = vec_iterator->second;

    const auto idx_iterator = global_hash_to_seg_vec_idx.find(sum_hash);
    if (idx_iterator == global_hash_to_seg_vec_idx.end())
        return nullptr;
    const size_t idx = idx_iterator->second;

    THROW_ASSERT(idx < vec.size());
    return &vec[idx];
}

} // namespace

////////////////////////////////////////////////// Exported functions
void db_make_simplest_equal_game(sumgame& sum, db_entry_partisan& entry,
                                 database& db)
{
    entry.ss_board_size = ss_board_size(sum, entry, db);
    //entry.ss_node_count = ss_node_count_naive(sum, entry, db);
    entry.ss_stone_count = ss_stone_count(sum, entry, db);
    entry.ss_tree_height = ss_tree_height(sum, entry, db);
    entry.ss_max_local_options = ss_max_local_options(sum, entry, db);

    //db.report_size_score(entry.disk_game_type, entry.size_score);

    // Find equivalence class
    const seg_map_idx_t seg_map_idx = make_seg_map_index(entry);
    vector<equivalence_class>& eq_classes = seg_map[seg_map_idx];

    optional<size_t> class_idx = find_sum_equivalence_class(sum, eq_classes, db);
    equivalence_class* eq_class = nullptr;

    if (!class_idx.has_value())
    {
        eq_class = &eq_classes.emplace_back();
        class_idx = eq_classes.size() - 1;
    }
    else
        eq_class = &eq_classes[*class_idx];

    assert(class_idx.has_value() && eq_class != nullptr);

    // Save class index so we can find it again later
    const hash_t sum_hash = database::get_db_hash(sum);
    global_hash_to_seg_vec_idx[sum_hash] = *class_idx;

    // Try to add the sum to the equivalence class
    const db_link_t sum_link = db.get_partisan_link(sum);
    eq_class->insert_link(sum_link, db);

    // Find the best link
    const db_link_t best_link = eq_class->get_best_link_for_game(sum_link, db);
    entry.simplest_equal_entry = best_link;
}

void db_refine_simplest_equal_game(
    pair<const hash_t, db_entry_partisan>& entry_pair, database& db)
{
    const equivalence_class* eq_class =
        get_previous_equivalence_class(entry_pair.first, entry_pair.second);

    if (eq_class == nullptr)
        return;

    const db_link_t sum_link = db.get_partisan_link(&entry_pair);
    const db_link_t stored_link = entry_pair.second.simplest_equal_entry;
    const db_link_t best_link = eq_class->get_best_link_for_game(sum_link, db);

    assert(sum_link.get_as_pointer() != nullptr);
    assert(stored_link.get_as_pointer() != nullptr);
    assert(best_link.get_as_pointer() != nullptr);

    if (!stored_link.equal_as_pointers(best_link))
        db_n_links_refined++;

    if (sum_link.equal_as_pointers(best_link))
    {
        // Link shouldn't get worse
        THROW_ASSERT(sum_link.equal_as_pointers(stored_link));
    }
    else
    {
        link_compare cmp(db);
#warning TODO re-enable this?
        //THROW_ASSERT(cmp.compare(best_link, stored_link) != REL_GREATER);
        entry_pair.second.simplest_equal_entry = best_link;
    }
}

void delete_equivalence_classes()
{
    seg_map.clear();
    global_hash_to_seg_vec_idx.clear();
}

