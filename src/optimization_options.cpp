#include "optimization_options.h"
#include <sstream>

using namespace std;


bool optimization_options::_subgame_split = true;
bool optimization_options::_simplify_basic_cgt_games = true;

////////////////////////////////////////
string optimization_options::get_summary()
{
    stringstream str;

    str << "subgame_split " << _subgame_split;
    str << endl;
    str << "simplify_basic_cgt_games " << _simplify_basic_cgt_games;

    return str.str();
}
