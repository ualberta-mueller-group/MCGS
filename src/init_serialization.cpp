#include "init_serialization.h"

#include "dynamic_serializable.h"

#include "clobber_1xn.h"
#include "nogo_1xn.h"
#include "elephants.h"

namespace mcgs_init {

void init_serialization()
{
    register_dyn_serializable<clobber_1xn>();
    register_dyn_serializable<nogo_1xn>();
    register_dyn_serializable<elephants>();
}

} // namespace mcgs_init 
