//---------------------------------------------------------------------------
// Move utilities
//---------------------------------------------------------------------------
#include "cgt_move.h"

#include <bitset>
#include <string>

namespace cgt_move {

std::string print(const move& m)
{
    return std::bitset<32>(m).to_string();
}

} // namespace cgt_move
