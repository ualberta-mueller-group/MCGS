#include "init_serialization.h"

#include "dynamic_serializable.h"
#include "all_game_headers.h"

namespace mcgs_init {

void init_serialization()
{
    register_dyn_serializable<clobber_1xn>();
    register_dyn_serializable<nogo_1xn>();
    register_dyn_serializable<elephants>();
    register_dyn_serializable<toppling_dominoes>();

    register_dyn_serializable<amazons>();
    register_dyn_serializable<nogo>();
    register_dyn_serializable<clobber>();
    register_dyn_serializable<cannibal_clobber>();
    register_dyn_serializable<domineering>();
    register_dyn_serializable<fission>();
    register_dyn_serializable<sheep>();
}

} // namespace mcgs_init

