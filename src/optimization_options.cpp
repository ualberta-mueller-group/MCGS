#include "optimization_options.h"
#include <sstream>

using namespace std;

bool optimization_options::_simplify_basic_cgt_games = true;

////////////////////////////////////////
string optimization_options::get_summary()
{
    stringstream str;

    str << "simplify_basic_cgt_games " << _simplify_basic_cgt_games;

    return str.str();
}
