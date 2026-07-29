#pragma once

#include "sumgame.h"
#include "database.h"

extern uint64_t db_n_links_refined;

void db_make_simplest_equal_game(sumgame& sum, db_entry_partisan& entry,
                                 const db_gen_options_t& gen_opts,
                                 database& db);

void db_refine_simplest_equal_game(
    std::pair<const hash_t, db_entry_partisan>& entry_pair, database& db);

void delete_equivalence_classes();
void reinitialize_equivalence_classes(database& db);
void assert_equivalence_classes_have_zero_entry(database& db);
