#pragma once

#include <string>
#include <vector>
#include <memory>

#include "cli_options.h"
#include "game.h"
#include "test_case.h"
#include "file_parser.h"

std::string get_games_string(const std::vector<game*>& games);

void run_test_from_main(std::shared_ptr<i_test_case> test_case,
                        const cli_options& opts);


