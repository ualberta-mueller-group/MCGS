#include "random.h"
#include "global_options.h"
#include "utilities.h"
#include <optional>
#include <cassert>

using namespace std;

namespace {
optional<random_generator> rng_int_seeded;
} // namespace

// TODO inline?
random_generator& get_global_rng()
{
    assert(rng_int_seeded.has_value());
    return *rng_int_seeded;
}

namespace mcgs_init {

void init_random_impl()
{
    assert(!rng_int_seeded.has_value());

    if (global::random_seed() == 0)
    {
        global::random_seed.set(ms_since_epoch());
        cout << "Chosen global random seed: " << global::random_seed() << endl;
    }

    rng_int_seeded = random_generator(global::random_seed());
}

} // namespace mcgs_init
