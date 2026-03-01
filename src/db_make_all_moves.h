#pragma once

#include <map>
#include "hashing.h"
#include "game.h"
#include "sumgame.h"


#ifdef MCGS_USE_DB_MOVES_TEST

enum db_move_action_enum
{
    DB_MOVE_ACTION_ENCODE = 0,
    DB_MOVE_ACTION_DECODE,
};

std::map<hash_t, std::set<move>> db_make_all_moves(const sumgame& sum, bw player);

void db_enc_or_dec_moves(const sumgame& sum,
                         std::map<hash_t, std::set<::move>>& move_map,
                         db_move_action_enum action);


void test_db_all_moves();
#endif
