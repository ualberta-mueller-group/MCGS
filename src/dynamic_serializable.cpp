#include <vector>
#include <cassert>

#include "dynamic_serializable.h"
#include "type_table.h"
#include "throw_assert.h"

namespace {

std::vector<load_fn_ptr_t> load_functions;
dyn_serializable_id_t next_sid = 1;

} // namespace

namespace __dyn_serializable_impl {

void register_impl(type_table_t* type_table, load_fn_ptr_t load_function)
{
    dyn_serializable_id_t& sid = type_table->dyn_serializable_id_ref();
    THROW_ASSERT(sid == 0);
    sid = next_sid++;
    load_functions.push_back(load_function);

    assert(get_load_function(type_table->dyn_serializable_id()) ==
           load_function);
}

load_fn_ptr_t get_load_function(dyn_serializable_id_t sid)
{
    THROW_ASSERT(0 < sid && sid <= load_functions.size());
    return load_functions[sid - 1];
}

} // namespace __dyn_serializable_impl
