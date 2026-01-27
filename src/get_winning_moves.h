/*
    Find all winning moves for a given player. Respects timeouts

    The "get_winning_moves" functions return moves in the order they are
    generated. Use sort_winning_moves() to deduplicate and sort them
*/
#pragma once

#include <vector>
#include <string>
#include <optional>
#include <cstdint>

#include "cgt_basics.h"
#include "sumgame.h"
#include "timeout_token.h"


// Never times out
std::vector<std::string> get_winning_moves(sumgame& sum, ebw player);

// Timeout in ms, 0 means never timeout
std::optional<std::vector<std::string>> get_winning_moves_with_timeout(
    sumgame& sum, ebw player, unsigned long long timeout_ms);

// Uses timeout_token
std::optional<std::vector<std::string>> get_winning_moves_with_timeout_token(
    sumgame& sum, ebw player, const timeout_token& timeout_tok, uint64_t depth);

/*
    Sort and deduplicate moves
    NOTE: "get_winning_moves_XYZ" functions don't call this
*/
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
