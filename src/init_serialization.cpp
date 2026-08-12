#include "init_serialization.h"

#include "poly_serializable.h"
#include "all_game_headers.h"

namespace mcgs_init {

void init_serialization()
{
    register_poly_serializable<clobber_1xn>();
    register_poly_serializable<nogo_1xn>();
    register_poly_serializable<elephants>();
    register_poly_serializable<toppling_dominoes>();

    register_poly_serializable<amazons>();
    register_poly_serializable<nogo>();
    register_poly_serializable<clobber>();
    register_poly_serializable<cannibal_clobber>();
    register_poly_serializable<domineering>();
    register_poly_serializable<fission>();
    register_poly_serializable<sheep>();
}

} // namespace mcgs_init

