#include "init_grid_hash_mask.h"

#include "grid_hash.h"

#include "nogo.h"
#include "clobber.h"
#include "domineering.h"
#include "amazons.h"
#include "fission.h"


namespace mcgs_init {

void init_grid_hash_mask()
{
    set_grid_hash_mask<nogo>(GRID_HASH_ACTIVE_MASK_ALL);
    set_grid_hash_mask<clobber>(GRID_HASH_ACTIVE_MASK_ALL);
    set_grid_hash_mask<domineering>(GRID_HASH_ACTIVE_MASK_MIRRORS);
    set_grid_hash_mask<amazons>(GRID_HASH_ACTIVE_MASK_ALL);
    set_grid_hash_mask<fission>(GRID_HASH_ACTIVE_MASK_MIRRORS);
}

} // namespace mcgs_init
