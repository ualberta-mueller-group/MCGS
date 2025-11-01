#include "split_test.h"

#include "split_test_elephants.h"
#include "split_test_clobber_1xn.h"
#include "split_test_clobber.h"
#include "split_test_dyadic_rational.h"
#include "split_test_integer_game.h"
#include "split_test_switch_game.h"
#include "split_test_nogo_1xn.h"
#include "split_test_nogo.h"
#include "split_test_nimber.h"
#include "split_test_up_star.h"
#include "test/split_test_amazons.h"
#include "test/split_test_domineering.h"
#include "test/split_test_fission.h"
#include "test/split_test_gen_toads.h"
#include "test/split_test_toppling_dominoes.h"

void split_test_all()
{
    split_test_elephants_all();
    split_test_clobber_1xn_all();
    split_test_clobber_all();
    split_test_dyadic_rational_all();
    split_test_integer_game_all();
    split_test_switch_game_all();
    split_test_nogo_1xn_all();
    split_test_nogo_all();
    split_test_nimber_all();
    split_test_up_star_all();
    split_test_amazons_all();
    split_test_domineering_all();
    split_test_fission_all();
    split_test_toppling_dominoes_all();
    split_test_gen_toads_all();
}
