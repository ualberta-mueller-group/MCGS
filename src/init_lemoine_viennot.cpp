#include "init_lemoine_viennot.h"

#include "impartial_lemoine_viennot.h"

namespace mcgs_init {

/* Size of nimber_hashcode table */ 
const int MAX_NIMBER = 500;

void init_lemoine_viennot_hashtable()
{
    lemoine_viennot::nimber_hashcode::init_codes(MAX_NIMBER);
}
} // namespace mcgs_init
