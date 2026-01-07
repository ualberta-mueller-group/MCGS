/*
    Find all winning moves for a given player. Respects timeouts

    The order of the resulting moves corresponds to the order of the underlying
    move generators


    TODO: Should player only be BLACK/WHITE, or should we allow it to also be
          EMPTY?
*/
#pragma once

#include <vector>
#include <string>
#include <optional>

#include "cgt_basics.h"
#include "sumgame.h"
#include "timeout_token.h"

// Never times out
std::vector<std::string> get_winning_moves(sumgame& sum, bw player);

// Timeout in ms, 0 means never timeout
std::optional<std::vector<std::string>> get_winning_moves_with_timeout(
    sumgame& sum, bw player, unsigned long long timeout_ms);

// Uses timeout_token
std::optional<std::vector<std::string>> get_winning_moves_with_timeout_token(
    sumgame& sum, bw player, const timeout_token& timeout_tok);
