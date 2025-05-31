#include "order_test_strips_and_grids.h"
#include <vector>
#include <iostream>
#include <exception>
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

//////////////////////////////////////////////////
void test_clobber_1xn()
{
    vector<game*> games;

    for (grid_generator gen(4); gen; ++gen)
        games.push_back(new clobber_1xn(gen.gen_board()));

    order_test_impl(games);

    for (game* g : games)
        delete g;
}

void test_nogo_1xn()
{
    ASSERT_DID_THROW(nogo_1xn("XO"));

    vector<game*> games;

    for (grid_generator gen(5); gen; ++gen)
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
    order_test_impl(games);

    for (game* g : games)
        delete g;
}

void test_elephants()
{
    vector<game*> games;

    for (grid_generator gen(4); gen; ++gen)
        games.push_back(new elephants(gen.gen_board()));

    order_test_impl(games);

    for (game* g : games)
        delete g;
}

void test_clobber()
{
    vector<game*> games;

    for (grid_generator gen(2, 3); gen; ++gen)
        games.push_back(new clobber(gen.gen_board()));

    order_test_impl(games);

    for (game* g : games)
        delete g;
}

void test_nogo()
{
    ASSERT_DID_THROW(nogo(".O.|OXO|.O."));

    vector<game*> games;

    for (grid_generator gen(3, 3); gen; ++gen)
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

    for (game* g : games)
        delete g;
}

void test_dummy()
{
    vector<game*> games;

    games.push_back(new dummy_game());
    games.push_back(new dummy_game());
    games.push_back(new dummy_game());
    games.push_back(new dummy_game());
    games.push_back(new dummy_game());
    games.push_back(new dummy_game());

    order_test_impl(games);

    for (game* g : games)
        delete g;
}


void test_mixed()
{
    ASSERT_DID_THROW(nogo_1xn("XO"));
    ASSERT_DID_THROW(nogo(".O.|OXO|.O."));

    vector<game*> games;

    for (size_t i = 0; i < 6; i++)
        games.push_back(new dummy_game());

    for (grid_generator gen(3); gen; ++gen)
    {
        const string& board = gen.gen_board();

        games.push_back(new clobber_1xn(board));
        games.push_back(new elephants(board));

        try
        {
            games.push_back(new nogo_1xn(board));
        }
        catch (exception& exc)
        {
        }
    }

    for (grid_generator gen(2, 2); gen; ++gen)
    {
        const string& board = gen.gen_board();

        games.push_back(new clobber(board));

        try
        {
            games.push_back(new nogo(board));
        }
        catch (exception& exc)
        {
        }
    }

    order_test_impl(games);

    for (game* g : games)
        delete g;
}

} // namespace

void order_test_strips_and_grids_all()
{
    cout << __FILE__ << endl;

    test_clobber_1xn();
    test_nogo_1xn();
    test_elephants();
    test_clobber();
    test_nogo();
    test_dummy();
    test_mixed();
}
