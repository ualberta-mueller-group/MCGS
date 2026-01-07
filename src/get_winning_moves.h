/*
    Find all winning moves for a given player. Respects timeouts

    The order of the resulting moves corresponds to the order of the underlying
    move generators


    TODO: Should player only be BLACK/WHITE, or should we allow it to also be
          EMPTY?
*/
#pragma once

#include <algorithm>
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

// "get_winning_moves_XYZ" functions don't use this
void sort_winning_moves(std::vector<std::string>& winning_moves);

////////////////////////////////////////////////// class winning_moves_diff_t
class winning_moves_diff_t
{
public:
    winning_moves_diff_t(const std::vector<std::string>& computed_moves,
                         const std::vector<std::string>& expected_moves);

    // Moves are sorted according to sort_winning_moves()
    const std::vector<std::string>& get_extra_moves() const;
    const std::vector<std::string>& get_missing_moves() const;

    std::string get_diff_string(bool prepend_diff_symbols) const;

private:
    std::vector<std::string> _extra_moves;
    std::vector<std::string> _missing_moves;
};

////////////////////////////////////////////////// misc function implementations
inline void sort_winning_moves(std::vector<std::string>& winning_moves)
{
    std::sort(winning_moves.begin(), winning_moves.end());
}

////////////////////////////////////////////////// winning_moves_diff_t methods
inline const std::vector<std::string>& winning_moves_diff_t::get_extra_moves()
    const
{
    return _extra_moves;
}

inline const std::vector<std::string>& winning_moves_diff_t::get_missing_moves()
    const
{
    return _missing_moves;
}
