#include "cli_options_test.h"

#include "all_game_headers.h"
#include "cli_options.h"
#include "clobber_1xn.h"
#include "file_parser.h"
#include "test/test_utilities.h"
#include <cassert>
#include <vector>
#include <string>
#include <ios>

constexpr const char* EXEC_NAME = "./MCGS_test";

using namespace std;

namespace {
cli_options call_parse(const vector<string>& args)
{
    /*
        don't allocate memory here; exceptions will be thrown and
            should be caught by the caller
    */
    int argc = args.size();
    vector<const char*> argv(argc);
    // const char* argv[argc];

    for (int i = 0; i < argc; i++)
    {
        argv[i] = args[i].c_str();
    }

    return parse_args(argc, argv.data(), true);
}

// empty args gives correct options
void cli_opts_test1()
{
    cli_options opts = call_parse({EXEC_NAME});
    assert(opts.parser.get() == nullptr);
    assert(opts.dry_run == false);
    assert(opts.should_exit == true);
}

// -h and --help give correct options
void cli_opts_test2()
{
    {
        cli_options opts = call_parse({EXEC_NAME, "-h"});
        assert(opts.parser.get() == nullptr);
        assert(opts.dry_run == false);
        assert(opts.should_exit == true);
    }

    {
        cli_options opts = call_parse({EXEC_NAME, "--help"});
        assert(opts.parser.get() == nullptr);
        assert(opts.dry_run == false);
        assert(opts.should_exit == true);
    }
}

// --dry-run sets correct options
void cli_opts_test3()
{
    {
        cli_options opts = call_parse({EXEC_NAME, "--dry-run"});
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
        cli_options opts =
            call_parse({EXEC_NAME, "--file", "not_a_real_file.test"});
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
        cli_options opts = call_parse({EXEC_NAME, "--file"});
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
        cli_options opts = call_parse({EXEC_NAME, "--not-a-valid-flag"});
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
        cli_options opts =
            call_parse({EXEC_NAME, "[clobber_1xn]", "XOXO", "{B}"});
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
    cli_options opts =
        call_parse({EXEC_NAME, "[nogo_1xn] X..O X...O..X {B win}"});
    assert(opts.parser.get() != nullptr);
    assert(opts.dry_run == false);
    assert(opts.should_exit == false);

    vector<game_case*> cases;

    {
        game_case* gc = new game_case();
        cases.push_back(gc);

        gc->run_command.player = BLACK;
        gc->run_command.expected_outcome = TEST_RESULT_WIN;
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
    cli_options opts = call_parse(
        {EXEC_NAME, "--file", "test/input/file_parser/sumgames4.test"});
    assert(opts.parser.get() != nullptr);
    assert(opts.dry_run == false);
    assert(opts.should_exit == false);

    vector<game_case*> cases;

    {
        game_case* gc = new game_case();
        cases.push_back(gc);

        gc->run_command.player = BLACK;
        gc->run_command.expected_outcome = TEST_RESULT_LOSS;
        gc->games.push_back(new clobber_1xn("XOOX"));
    }

    assert_file_parser_output(opts.parser.get(), cases);

    for (game_case* gc : cases)
    {
        gc->cleanup_games();
        delete gc;
    }
}

// --test-timeout
void cli_opts_test10()
{
    cli_options opts = call_parse({EXEC_NAME, "--test-timeout", "42734"});
    assert(opts.test_timeout == 42734);
}

// --out-file
void cli_opts_test11()
{
    cli_options opts = call_parse({EXEC_NAME, "--out-file", "65432.csv"});
    assert(opts.outfile_name == "65432.csv");
}

// --test-dir
void cli_opts_test12()
{
    cli_options opts = call_parse({EXEC_NAME, "--test-dir", "somedir462"});
    assert(opts.test_directory == "somedir462");
}
} // namespace

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

    cli_opts_test10();
    cli_opts_test11();
    cli_opts_test12();
}
