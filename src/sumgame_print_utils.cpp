#include "sumgame_print_utils.h"

using namespace std;

string sumgame_move_to_string(const sumgame& sum, const sumgame_move& sm, ebw player,
                       bool with_subgame_idx)
{
    const game* g = sum.subgame_const(sm.subgame_idx);

    stringstream stream;

    if (with_subgame_idx)
        stream << sm.subgame_idx << ':';

    g->print_move(stream, sm.m, player);

    const string str = stream.str();
    assert(!string_contains_whitespace(str));

    return str;
}


