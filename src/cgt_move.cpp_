//---------------------------------------------------------------------------
// Move utilities
//---------------------------------------------------------------------------
#include "cgt_move.h"
#include "cgt_basics.h"

#include <bitset>
#include <string>

namespace cgt_move {

std::string print(const move& m)
{
    return std::bitset<32>(m).to_string();
}

std::string print_two_part_move(const move& m)
{
    int second;
    bw player;
    const int first = decode3(m, &second, &player);
    std::string result;
    result += color_to_player_char(player);
    result += ' ';
    result += std::to_string(first);
    result += ' ';
    result += std::to_string(second);
    return result;
}

} // namespace cgt_move
