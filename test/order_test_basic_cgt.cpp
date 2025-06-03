#include "order_test_basic_cgt.h"
#include <vector>
#include <functional>

#include "order_test_utilities.h"

#include "game.h"
#include "cgt_dyadic_rational.h"
#include "cgt_integer_game.h"
#include "cgt_nimber.h"
#include "cgt_switch.h"
#include "cgt_up_star.h"

#include "kayles.h" // TODO should this get its own file?

using namespace std;

namespace {

////////////////////////////////////////////////// helpers
vector<fraction> make_fraction_range(int top_max, int bot_max_power)
{
    assert(top_max > 0 && bot_max_power > 0);
    vector<fraction> fracs;

    const int bot_max = (1 << bot_max_power);

    for (int bot = 1; bot <= bot_max; bot <<= 1)
        for (int top = -top_max; top < top_max; top++)
            fracs.push_back(fraction(top, bot));

    return fracs;
}

////////////////////////////////////////////////// generator functions
typedef function<void(vector<game*>&, bool)> generator_function_t;

void gen_dyadic_rational(vector<game*>& games, bool fewer)
{
    int top_max = fewer ? 6 : 10;
    int bot_max_power = fewer ? 3 : 5;

    vector<fraction> fracs = make_fraction_range(top_max, bot_max_power);

    for (const fraction& f : fracs)
        games.push_back(new dyadic_rational(f));
}

void gen_integer_game(vector<game*>& games, bool fewer)
{
    int range = fewer ? 6 : 10;

    for (int i = -range; i <= range; i++)
        games.push_back(new integer_game(i));
}

void gen_nimber(vector<game*>& games, bool fewer)
{
    int max = fewer ? 6 : 20;

    for (int i = 0; i <= max; i++)
        games.push_back(new nimber(i));
}

void gen_switch_game(vector<game*>& games, bool fewer)
{
    int top_max = 5;
    int bot_max_power = 3;

    vector<fraction> fracs = make_fraction_range(top_max, bot_max_power);

    for (const fraction& f1 : fracs)
        for (const fraction& f2 : fracs)
            games.push_back(new switch_game(f1, f2));
}

void gen_up_star(vector<game*>& games, bool fewer)
{
    int range = fewer ? 6 : 20;

    for (int i = 0; i <= range; i++)
        for (int j = 0; j < 2; j++)
            games.push_back(new up_star(i, j > 0));
}

void gen_kayles(vector<game*>& games, bool fewer)
{
    int max = fewer ? 6 : 20;
    for (int i = 0; i < max; i++)
        games.push_back(new kayles(i));
}

////////////////////////////////////////////////// generic test implementation
void test_generic(const vector<generator_function_t>& funcs)
{
    assert(!funcs.empty());
    bool fewer = funcs.size() > 1;

    vector<game*> games;

    for (const generator_function_t& func : funcs)
    {
        const size_t N = games.size();
        func(games, fewer);
        assert(games.size() > N);
    }

    order_test_impl(games);

    for (game* g : games)
        delete g;
}

} // namespace

//////////////////////////////////////////////////
void order_test_basic_cgt_all()
{
    test_generic({gen_dyadic_rational});
    test_generic({gen_integer_game});
    test_generic({gen_nimber});
    test_generic({gen_switch_game});
    test_generic({gen_up_star});
    test_generic({gen_kayles});

    test_generic({
        gen_dyadic_rational,
        gen_integer_game,
        gen_nimber,
        gen_switch_game,
        gen_up_star,
        gen_kayles,
    });
}
