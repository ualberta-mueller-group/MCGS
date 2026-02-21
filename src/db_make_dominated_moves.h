#pragma once


#include "cgt_basics.h"
#include "sumgame.h"
#include "hashing.h"
#include "serializer.h"
#include "dominated_moves.h"


//////////////////////////////////////////////////
std::shared_ptr<dominated_moves_t> db_make_dominated_moves(const sumgame& sum);
