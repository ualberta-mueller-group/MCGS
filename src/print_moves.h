#pragma once

#include <memory>
#include <string>

#include "file_parser.h"
#include "sumgame.h"
#include "cgt_basics.h"

void print_winning_moves_by_chunk(std::shared_ptr<file_parser> parser);
void print_sum_moves_by_chunk(std::shared_ptr<file_parser> parser);
void print_subgame_moves_by_chunk(std::shared_ptr<file_parser> parser);
