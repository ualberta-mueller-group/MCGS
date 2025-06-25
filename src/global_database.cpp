#include "global_database.h"
#include "throw_assert.h"
#include <optional>

//////////////////////////////////////////////////
namespace {

std::optional<database> global_db;

} // namespace

//////////////////////////////////////////////////
database& get_global_database()
{
    assert(global_db.has_value());
    return global_db.value();
}

void init_global_database()
{
    THROW_ASSERT(!global_db.has_value());
    global_db = database();
}
