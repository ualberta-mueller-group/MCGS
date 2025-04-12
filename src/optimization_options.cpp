#include "optimization_options.h"
#include <sstream>

using namespace std;

bool optimization_options::_subgame_split = true;
bool optimization_options::_simplify_basic_cgt = true;
bool optimization_options::_sumgame_ttable = true;

////////////////////////////////////////
string optimization_options::get_summary()
{
    stringstream str;

    str << "subgame_split " << _subgame_split;

    str << endl;
    str << "simplify_basic_cgt " << _simplify_basic_cgt;

    str << endl;
    str << "sumgame_ttable " << _sumgame_ttable;

    return str.str();
}
