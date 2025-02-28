#pragma once
#include "sumgame.h"
#include <climits>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include "obj_id.h"

//////////////////////////////////////// forward declarations
class sumgame;


//////////////////////////////////////// sumgame_impl::change_record
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
