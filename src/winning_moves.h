#pragma once

#include <vector>
#include <memory>

#include "game.h"
#include "file_parser.h"

/*
std::vector<move> get_winning_moves(const game* g, bw to_play);

void print_winning_moves_impl(std::shared_ptr<file_parser> fp);
*/

//////////////////////////////////////////////////
void print_winning_moves_new(std::shared_ptr<file_parser> fp);
