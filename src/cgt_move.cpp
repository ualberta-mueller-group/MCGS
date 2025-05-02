//---------------------------------------------------------------------------
// Move utilities
//---------------------------------------------------------------------------
#include "cgt_move.h"

#include <bitset>

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
    result += color_char(player);
    result += ' ';
    result += std::to_string(first);
    result += ' ';
    result += std::to_string(second);
    return result;
}

} // namespace cgt_move
