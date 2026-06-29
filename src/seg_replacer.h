#pragma once

#include "sumgame_change_record.h"
#include "database.h"

class seg_replacer;

seg_replacer* seg_replacer_new();
void seg_replacer_delete(seg_replacer* replacer);
void seg_replacer_replace_all(seg_replacer* replacer, sumgame& sum,
                              sumgame_impl::change_record& cr, database& db);

void test_seg_replacer_stuff();
