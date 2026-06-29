
#include "seg_replacer.h"

#include "bounds.h"
#include "db_link_t.h"
#include "grid_generator.h"
#include "gridlike_db_game_generator.h"
#include "safe_arithmetic.h"
#include "sumgame_change_record.h"
#include "database.h"
#include "type_table.h"
#include "clobber_1xn.h"
#include <algorithm>

using sumgame_impl::change_record;
using namespace std;

////////////////////////////////////////////////// Helpers
namespace {
typedef pair<const hash_t, db_entry_partisan> db_pair_t;

//////////////////////////////////////// struct temp_db_game_t
struct temp_db_game_t
{
    temp_db_game_t(game* real_game, db_pair_t* entry_ptr);
    temp_db_game_t(db_pair_t* entry_ptr);

    bool is_valid() const;
    void release();

    hash_t get_hash() const;

    static bool compare(const temp_db_game_t& tg1, const temp_db_game_t& tg2);

    game* real_game;
    db_pair_t* entry_ptr;
};

//////////////////////////////////////// temp_db_game_t methods
temp_db_game_t::temp_db_game_t(game* real_game, db_pair_t* entry_ptr)
    : real_game(real_game), entry_ptr(entry_ptr)
{
    assert(real_game != nullptr &&   //
           entry_ptr != nullptr      //
    );
}

temp_db_game_t::temp_db_game_t(db_pair_t* entry_ptr)
    : real_game(nullptr), entry_ptr(entry_ptr)
{
    assert(entry_ptr != nullptr);
}

bool temp_db_game_t::is_valid() const
{
    return entry_ptr != nullptr;
}

void temp_db_game_t::release()
{
    real_game = nullptr;
    entry_ptr = nullptr;
}

hash_t temp_db_game_t::get_hash() const
{
    return entry_ptr->first;
}

bool temp_db_game_t::compare(const temp_db_game_t& tg1, const temp_db_game_t& tg2)
{
    if (!tg1.is_valid())
        return false;

    if (!tg2.is_valid())
        return true;

    const uint64_t size_score1 = tg1.entry_ptr->second.size_score;
    const uint64_t size_score2 = tg2.entry_ptr->second.size_score;

    return size_score1 < size_score2;

}

} // namespace

////////////////////////////////////////////////// class seg_replacer
class seg_replacer
{
public:
    seg_replacer();

    /*
        Main driver code
    */
    void reset(sumgame* sum, change_record* cr, database* db);
    void replace_all();

    void replace1();
    void replace2();
    void replace3_plus();

    void inflate_games();

    /*
        Replacement functions
    */
    bool replace_with_bounds(db_pair_t* entry_ptr);
    bool replace_with_seg(db_pair_t* entry_ptr);
    bool replace_selection();

    /*
        Abstract game/container operations
    */
    void insert_temp_game(temp_db_game_t temp_game);
    vector<temp_db_game_t>& get_or_create_container(game_type_t disk_type);
    void bind_container(game_type_t new_disk_type);

    /*
        Selection operations
    */
    bool try_add_to_selection(size_t sel_idx);
    void pop_selection();
    void reverse_selection();

    hash_t get_selection_hash();
    db_pair_t* get_selection_entry();

    void clear_selection();
    void delete_selection_games();

private:
    sumgame* _sum;
    change_record* _cr;
    database* _db;

    global_hash _gh;

    bound_t _rational_sum;
    bound_t _up_sum;

    vector<vector<temp_db_game_t>> _game_containers;
    vector<bool> _active_container_mask;

    game_type_t _current_container_type;
    vector<temp_db_game_t>* _current_container;

    uint64_t _max_size_score;
    uint64_t _size_score_sum;

    vector<size_t> _selection_indices;
    vector<hash_t> _selection_hashes;
};

////////////////////////////////////////////////// seg_replacer methods
seg_replacer::seg_replacer()
{
}

void seg_replacer::reset(sumgame* sum, change_record* cr, database* db)
{
    assert(sum != nullptr && //
           cr != nullptr &&  //
           db != nullptr     //
    );

    _sum = sum;
    _cr = cr;
    _db = db;

    _rational_sum = 0;
    _up_sum = 0;

    for (vector<temp_db_game_t>& container : _game_containers)
        container.clear();

    std::fill(_active_container_mask.begin(), _active_container_mask.end(),
              false);

    _current_container_type = 0;
    _current_container = nullptr;

    _max_size_score = 0;
    _size_score_sum = 0;

    _selection_indices.clear();
    _selection_hashes.clear();
}

void seg_replacer::replace_all()
{
    // Populate initial candidates
    const int n_games = _sum->num_total_games();
    for (int i = 0; i < n_games; i++)
    {
        game* g = _sum->subgame(i);

        if (!g->is_active() || g->is_impartial())
            continue;

        db_pair_t* entry_ptr = _db->get_partisan_ptr_pair(*g);
        if (entry_ptr == nullptr)
            continue;

        insert_temp_game(temp_db_game_t(g, entry_ptr));
    }

    replace1();
    replace2();
    replace3_plus();
    inflate_games();
}

void seg_replacer::replace1()
{
    for (size_t container_type = 1;
         container_type < _active_container_mask.size(); container_type++)
    {
        if (!_active_container_mask[container_type])
            continue;

        bind_container(container_type);
        if (_max_size_score == 0)
            continue;

        const size_t container_size = _current_container->size();
        for (size_t sel_idx = 0; sel_idx < container_size; sel_idx++)
        {
            clear_selection();
            if (try_add_to_selection(sel_idx) && replace_selection())
                delete_selection_games();
        }
    }
}

void seg_replacer::replace2()
{

    for (size_t container_type = 1;
         container_type < _active_container_mask.size(); container_type++)
    {
        if (!_active_container_mask[container_type])
            continue;

        bind_container(container_type);
        if (_max_size_score == 0)
            continue;

        const size_t container_size = _current_container->size();
        for (size_t sel1 = 0; sel1 < container_size; sel1++)
        {
            clear_selection();

            if (!try_add_to_selection(sel1))
                continue;

            for (size_t sel2 = sel1 + 1; sel2 < container_size; sel2++)
            {
                if (!try_add_to_selection(sel2))
                    continue;

                if (replace_selection())
                {
                    delete_selection_games();
                    break;
                }

                pop_selection();
            }
        }
    }

}

void seg_replacer::replace3_plus()
{
    for (size_t container_type = 1;
         container_type < _active_container_mask.size(); container_type++)
    {
        if (!_active_container_mask[container_type])
            continue;

        bind_container(container_type);
        if (_max_size_score == 0)
            continue;

        std::sort(_current_container->begin(), _current_container->end(),
                  temp_db_game_t::compare);

        clear_selection();
        for (size_t sel1 = 0; sel1 < _current_container->size(); sel1++)
        {
            if (!(*_current_container)[sel1].is_valid())
                continue;

            // Window is maxed out
            if (!try_add_to_selection(sel1))
            {
                if (_selection_indices.size() < 3)
                {
                    clear_selection();
                    continue;
                }

                // Put smallest size score last
                reverse_selection();

                while (_selection_indices.size() >= 3)
                {
                    if (replace_selection())
                    {
                        delete_selection_games();
                        break;
                    }

                    pop_selection();
                }

                clear_selection();
            }
        }
    }
}

void seg_replacer::inflate_games()
{
    // Inflate bound sums
    if (_rational_sum != 0)
    {
        game* g_rational = get_scale_game(_rational_sum,
                BOUND_SCALE_DYADIC_RATIONAL);

        _sum->add(g_rational);
        _cr->added_games.push_back(g_rational);
    }

    if (_up_sum != 0)
    {
        game* g_up = get_scale_game(_up_sum, BOUND_SCALE_UP);

        _sum->add(g_up);
        _cr->added_games.push_back(g_up);
    }

    vector<game*> games;

    // Inflate added games
    for (vector<temp_db_game_t>& container : _game_containers)
    {
        for (temp_db_game_t& tg : container)
        {
            if (!tg.is_valid() || tg.real_game != nullptr)
                continue;

            db_entry_partisan& entry = tg.entry_ptr->second;
            games = entry.load_sum();

            _sum->add(games);
            for (game* g : games)
                _cr->added_games.push_back(g);
        }
    }
}

bool seg_replacer::replace_with_bounds(db_pair_t* entry_ptr)
{
    const shared_ptr<game_bounds>& bounds = entry_ptr->second.bounds_data;

    if (!bounds || !bounds->is_equal())
        return false;

    bool add_ok = false;

    switch (bounds->get_scale())
    {
        case BOUND_SCALE_DYADIC_RATIONAL:
        {
            add_ok = safe_add(_rational_sum, bounds->get_lower());
            break;
        }
        case BOUND_SCALE_UP:
        {
            add_ok = safe_add(_up_sum, bounds->get_lower());
            break;
        }
        default:
            assert(false);
    }

    assert(add_ok);
    return true;
}

bool seg_replacer::replace_with_seg(db_pair_t* entry_ptr)
{
    db_link_t seg_link = entry_ptr->second.simplest_equal_entry;
    db_pair_t* seg_entry_pair = seg_link.get_as_pointer();

    if (seg_entry_pair == nullptr || seg_entry_pair == entry_ptr)
        return false;

    db_entry_partisan& seg_entry = seg_entry_pair->second;

    for (db_link_t seg_subgame_link : seg_entry.subgame_links)
    {
        db_pair_t* seg_subgame_entry_pair = seg_subgame_link.get_as_pointer();

        if (replace_with_bounds(seg_subgame_entry_pair))
            continue;

        insert_temp_game(temp_db_game_t(seg_subgame_entry_pair));
    }

    return true;
}

bool seg_replacer::replace_selection()
{
    db_pair_t* selection_entry = get_selection_entry();

    if (selection_entry == nullptr)
        return false;

    if (replace_with_bounds(selection_entry))
        return true;

    if (replace_with_seg(selection_entry))
        return true;

    return false;
}

void seg_replacer::insert_temp_game(temp_db_game_t temp_game)
{
    const game_type_t disk_type = temp_game.entry_ptr->second.disk_game_type;
    vector<temp_db_game_t>& container = get_or_create_container(disk_type);

    container.push_back(temp_game);
    _active_container_mask[disk_type] = true;
}

vector<temp_db_game_t>& seg_replacer::get_or_create_container(game_type_t disk_type)
{
    if (disk_type >= _game_containers.size())
    {
        _game_containers.resize(disk_type + 1);
        _active_container_mask.resize(disk_type + 1, false);

        if (_current_container_type != 0)
            _current_container = &_game_containers[_current_container_type];
    }

    return _game_containers[disk_type];
}

void seg_replacer::bind_container(game_type_t new_disk_type)
{
    assert(new_disk_type > 0);
    clear_selection();

    _current_container_type = new_disk_type;
    _current_container = &_game_containers[new_disk_type];

    _max_size_score = _db->get_max_size_score(new_disk_type);
    _size_score_sum = 0;
}

bool seg_replacer::try_add_to_selection(size_t sel_idx)
{
    temp_db_game_t& tg = (*_current_container)[sel_idx];

    if (!tg.is_valid())
        return false;

    db_entry_partisan& entry = tg.entry_ptr->second;

    if (_size_score_sum + entry.size_score > _max_size_score)
        return false;

    _size_score_sum += entry.size_score;
    _selection_indices.push_back(sel_idx);
    return true;
}

void seg_replacer::pop_selection()
{
    assert(!_selection_indices.empty());

    const size_t sel_idx = _selection_indices.back();
    _selection_indices.pop_back();

    temp_db_game_t& tg = (*_current_container)[sel_idx];

    _size_score_sum -= tg.entry_ptr->second.size_score;
}

void seg_replacer::reverse_selection()
{
    std::reverse(_selection_indices.begin(), _selection_indices.end());
}

// TODO move some parts into global_hash
hash_t seg_replacer::get_selection_hash()
{
    // Find expected combined hash
    //static sumgame sum(BLACK);

    //assert(sum.num_total_games() == 0);
    //for (const size_t sel_idx : _selection_indices)
    //{
    //    temp_db_game_t& tg = (*_current_container)[sel_idx];
    //    assert(tg.is_valid());

    //    db_entry_partisan& entry = tg.entry_ptr->second;
    //    vector<game*> games = entry.load_sum();
    //    sum.add(games);
    //}

    //const hash_t expected_hash = database::get_db_hash(sum);

    //cout << "Finding hash: ";
    //sum.print_simple(cout);
    //cout << endl;

    //while (sum.num_total_games() != 0)
    //{
    //    game* g = sum.subgame(sum.num_total_games() - 1);
    //    sum.pop(g);
    //    delete g;
    //}

    // Compute combined hash
    _selection_hashes.resize(_selection_indices.size());

    const size_t selection_size = _selection_indices.size();
    for (size_t i = 0; i < selection_size; i++)
    {
        const size_t sel_idx = _selection_indices[i];
        temp_db_game_t& sel_tg = (*_current_container)[sel_idx];
        assert(sel_tg.is_valid());
        db_entry_partisan& entry = sel_tg.entry_ptr->second;
        assert(entry.subgame_hashes.size() == 1);
        _selection_hashes[i] = entry.subgame_hashes.back();
    }

    std::sort(_selection_hashes.begin(), _selection_hashes.end(),
              global_hash::hash_compare_fn);

    _gh.reset();
    _gh.set_to_play(EMPTY);

    for (size_t i = 0; i < selection_size; i++)
        _gh.add_hash(i, _selection_hashes[i]);

    const hash_t computed_hash = _gh.get_value();

    //assert(computed_hash == expected_hash);
    return computed_hash;
}

db_pair_t* seg_replacer::get_selection_entry()
{
    assert(!_selection_indices.empty());

    if (_selection_indices.size() == 1)
    {
        temp_db_game_t& tg = (*_current_container)[_selection_indices.back()];
        return tg.entry_ptr;
    }

    const hash_t selection_hash = get_selection_hash();
    return _db->get_partisan_ptr_pair(selection_hash);
}

void seg_replacer::clear_selection()
{
    _selection_indices.clear();
    _size_score_sum = 0;
}

void seg_replacer::delete_selection_games()
{
    vector<temp_db_game_t>& container = *_current_container;

    for (const size_t sel_idx : _selection_indices)
    {
        temp_db_game_t& tg = container[sel_idx];

        if (tg.real_game != nullptr)
        {
            game* g = tg.real_game;
            assert(g->is_active());
            g->set_active(false);
            _cr->deactivated_games.push_back(g);
        }

        tg.release();
    }
}

////////////////////////////////////////////////// Exported functions
seg_replacer* seg_replacer_new()
{
    return new seg_replacer();
}

void seg_replacer_delete(seg_replacer* replacer)
{
    assert(replacer != nullptr);
    delete replacer;
}

void seg_replacer_replace_all(seg_replacer* replacer, sumgame& sum,
                              change_record& cr, database& db)
{
    assert(replacer != nullptr);
    replacer->reset(&sum, &cr, &db);
    replacer->replace_all();
}

void test_seg_replacer_stuff()
{
    cout << "Testing SEG replacer" << endl;
    sumgame sum(BLACK);

    gridlike_db_game_generator<clobber_1xn, GRIDLIKE_TYPE_STRIP> gen(
        new grid_generator({1, 15}, {EMPTY, BLACK, WHITE}, true));

    seg_replacer replacer;

    while (gen)
    {
        game* g = gen.gen_game();
        ++gen;
        //cout << "TESTING: " << *g << endl;

        sum.add(g);
        sum.split_and_normalize();

        sum.seg_pass(&replacer);
        sum.undo_seg_pass();

        sum.undo_split_and_normalize();
        sum.pop(g);

        delete g;
    }
}

