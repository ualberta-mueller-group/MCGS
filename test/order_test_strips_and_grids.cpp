#include "order_test_strips_and_grids.h"
#include <vector>
#include <iostream>
#include <exception>
#include <functional>
#include "game.h"
#include "grid_utils.h"
#include "test_utilities.h"

#include "clobber_1xn.h"
#include "nogo_1xn.h"
#include "elephants.h"

#include "clobber.h"
#include "nogo.h"
#include "test/order_test_utilities.h"

using namespace std;

namespace {

////////////////////////////////////////////////// class dummy_game
class dummy_game: public game
{
public:
    dummy_game()
    {
    }

    void play(const ::move& m, bw to_play) override
    {
        assert(false);
    }

    void undo_move() override
    {
        assert(false);
    }

protected:
    void _init_hash(local_hash& hash) const override
    {
        assert(false);
    }

public:
    move_generator* create_move_generator(bw to_play) const override
    {
        assert(false);
    }

    void print(std::ostream& str) const override
    {
        str << "dummy_game";
    }

    game* inverse() const override
    {
        assert(false);
    }
};

////////////////////////////////////////////////// generator functions
typedef function<void(vector<game*>&, bool)> generator_function_t;

void gen_clobber_1xn(vector<game*>& games, bool fewer)
{
    int range = fewer ? 3 : 4;

    for (grid_generator gen(range); gen; ++gen)
        games.push_back(new clobber_1xn(gen.gen_board()));
}

void gen_nogo_1xn(vector<game*>& games, bool fewer)
{
    ASSERT_DID_THROW(nogo_1xn("XO"));

    int range = fewer ? 3 : 5;

    for (grid_generator gen(range); gen; ++gen)
    {
        try
        {
            games.push_back(new nogo_1xn(gen.gen_board()));
        }
        catch (exception &exc)
        {
        }
    }

    assert(!games.empty());
}

void gen_elephants(vector<game*>& games, bool fewer)
{
    int range = fewer ? 3 : 4;

    for (grid_generator gen(range); gen; ++gen)
        games.push_back(new elephants(gen.gen_board()));
}

void gen_clobber(vector<game*>& games, bool fewer)
{
    int range_r = 2;
    int range_c = fewer ? 2 : 3;

    for (grid_generator gen(range_r, range_c); gen; ++gen)
        games.push_back(new clobber(gen.gen_board()));
}

void gen_nogo(vector<game*>& games, bool fewer)
{
    ASSERT_DID_THROW(nogo(".O.|OXO|.O."));

    int range_r = 2;
    int range_c = fewer ? 2 : 3;

    for (grid_generator gen(range_r, range_c); gen; ++gen)
    {
        try
        {
            games.push_back(new nogo(gen.gen_board()));
        }
        catch (exception& exc)
        {
        }
    }

    assert(!games.empty());
}

void gen_dummy_game(vector<game*>& games, bool fewer)
{
    int count = fewer ? 5 : 10;
    for (int i = 0; i < count; i++)
        games.push_back(new dummy_game());
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
void order_test_strips_and_grids_all()
{
    test_generic({gen_clobber_1xn});
    test_generic({gen_nogo_1xn});
    test_generic({gen_elephants});
    test_generic({gen_clobber});
    test_generic({gen_nogo});
    test_generic({gen_dummy_game});

    test_generic({
        gen_clobber_1xn,
        gen_nogo_1xn,
        gen_elephants,
        gen_clobber,
        gen_nogo,
        gen_dummy_game,
    });
}
