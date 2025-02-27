#pragma once
#include "sumgame.h"
#include <vector>
#include <unordered_map>

//////////////////////////////////////// forward declarations

class sumgame;

////////////////////////////////////////

namespace sumgame_impl {

class change_record
{
public:
    ~change_record();

    void simplify_basic(sumgame& sum);
    void undo_simplify_basic(sumgame& sum);

    std::vector<game*> deactivated_games;
    std::vector<game*> added_games;
private:
    void _clear();
};



} // namespace sumgame_impl
