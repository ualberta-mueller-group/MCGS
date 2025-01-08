#include "cli_options_test.h"


#include "all_game_headers.h"
#include "cli_options.h"
#include "clobber_1xn.h"
#include "file_parser.h"
#include "test/test_utilities.h"

using namespace std;

cli_options call_parse(const vector<string>& args)
{
    /*
        don't allocate memory here; exceptions will be thrown and 
            should be caught by the caller
    */
    int argc = args.size();
    const char* argv[argc];

    for (int i = 0; i < argc; i++)
    {
        argv[i] = args[i].c_str();
    }

    return parse_cli_args(argc, argv, true);
}

// empty args gives correct options
void cli_opts_test1()
{
    cli_options opts = call_parse({"./MCGS"});
    assert(opts.parser.get() == nullptr);
    assert(opts.dry_run == false);
    assert(opts.should_exit == false);
}

// -h and --help give correct options
void cli_opts_test2()
{
    {
        cli_options opts = call_parse({"./MCGS", "-h"});
        assert(opts.parser.get() == nullptr);
        assert(opts.dry_run == false);
        assert(opts.should_exit == true);
    }

    {
        cli_options opts = call_parse({"./MCGS", "--help"});
        assert(opts.parser.get() == nullptr);
        assert(opts.dry_run == false);
        assert(opts.should_exit == true);
    }
}

// --dry-run sets correct options
void cli_opts_test3()
{
    {
        cli_options opts = call_parse({"./MCGS", "--dry-run"});
        assert(opts.parser.get() == nullptr);
        assert(opts.dry_run == true);
        assert(opts.should_exit == false);
    }
}

// --file with wrong file throws IO exception
void cli_opts_test4()
{
    bool did_throw = false;

    try
    {
        cli_options opts = call_parse({"./MCGS", "--file", "not_a_real_file.test"});
    }
    catch (ios_base::failure& e)
    {
        did_throw = true;
    }

    assert(did_throw);
}

// --file without name throws opts exception
void cli_opts_test5()
{
    bool did_throw = false;

    try
    {
        cli_options opts = call_parse({"./MCGS", "--file"});
    }
    catch (cli_options_exception& e)
    {
        did_throw = true;
    }

    assert(did_throw);
}

// invalid flags throws opts exception
void cli_opts_test6()
{
    bool did_throw = false;

    try
    {
        cli_options opts = call_parse({"./MCGS", "--not-a-valid-flag"});
    }
    catch (cli_options_exception& e)
    {
        did_throw = true;
    }

    assert(did_throw);
}


// wrong number of args (unquoted game) throws opts exception
void cli_opts_test7()
{
    bool did_throw = false;

    try
    {
        cli_options opts = call_parse({"./MCGS", "[clobber_1xn]", "XOXO", "{B}"});
    }
    catch (cli_options_exception& e)
    {
        did_throw = true;
    }

    assert(did_throw);
}

// args give correct games/parser
void cli_opts_test8()
{
    cli_options opts = call_parse({"./MCGS", "[nogo_1xn] X..O X...O..X {B win}"});
    assert(opts.parser.get() != nullptr);
    assert(opts.dry_run == false);
    assert(opts.should_exit == false);

    vector<game_case *> cases;

    {
        game_case* gc = new game_case();
        cases.push_back(gc);

        gc->to_play = BLACK;
        gc->expected_outcome = TEST_OUTCOME_WIN;
        gc->games.push_back(new nogo_1xn("X..O"));
        gc->games.push_back(new nogo_1xn("X...O..X"));
    }

    assert_file_parser_output(opts.parser.get(), cases);

    for (game_case* gc : cases)
    {
        gc->cleanup_games();
        delete gc;
    }
}

// --file gives correct file/parser
void cli_opts_test9()
{
    cli_options opts = call_parse({"./MCGS", "--file", "test/input/file_parser/sumgames4.test"});
    assert(opts.parser.get() != nullptr);
    assert(opts.dry_run == false);
    assert(opts.should_exit == false);

    vector<game_case *> cases;

    {
        game_case* gc = new game_case();
        cases.push_back(gc);

        gc->to_play = BLACK;
        gc->expected_outcome = TEST_OUTCOME_LOSS;
        gc->games.push_back(new clobber_1xn("XOOX"));
    }

    assert_file_parser_output(opts.parser.get(), cases);

    for (game_case* gc : cases)
    {
        gc->cleanup_games();
        delete gc;
    }

}

void cli_options_test_all()
{
    cli_opts_test1();
    cli_opts_test2();
    cli_opts_test3();
    cli_opts_test4();
    cli_opts_test5();
    cli_opts_test6();
    cli_opts_test7();
    cli_opts_test8();
    cli_opts_test9();
}
