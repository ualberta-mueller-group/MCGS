#pragma once
#include "sumgame.h"
#include <vector>
#include "game.h"

//////////////////////////////////////// forward declarations
class sumgame;

//////////////////////////////////////// sumgame_impl::change_record
namespace sumgame_impl {

class change_record
{
public:
    change_record() {};
    ~change_record();

    change_record(change_record&& other) noexcept;
    change_record& operator=(change_record&& other) noexcept;

    void simplify_basic(sumgame& sum);
    void undo_simplify_basic(sumgame& sum);

    bool no_change() const;

    std::vector<game*> deactivated_games;
    std::vector<game*> added_games;

private:
    void _clear();
    void _move_impl(change_record&& other) noexcept;
};

} // namespace sumgame_impl
