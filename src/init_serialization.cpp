#include "init_serialization.h"

#include "dynamic_serializable.h"

#include "clobber_1xn.h"
#include "nogo_1xn.h"

namespace mcgs_init {

void init_serialization()
{
    register_dyn_serializable<clobber_1xn>();
    register_dyn_serializable<nogo_1xn>();
}

} // namespace mcgs_init
