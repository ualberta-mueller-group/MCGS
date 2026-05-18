#include "cli_options_test.h"

#include <cassert>
#include <vector>
#include <string>
#include <ios>

#include "all_game_headers.h"
#include "cli_options.h"
#include "clobber_1xn.h"
#include "paths.h"
#include "test_utilities.h"
#include "csv_row.h"
#include "test_case_enums.h"

using namespace std;

namespace {

class cli_options_test_class
{
private:
    const std::string _exec_name;

public:
    cli_options_test_class(const std::string exec_name) : _exec_name(exec_name)
    {
    }

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
            argv[i] = args[i].c_str();

        return parse_args(argc, argv.data(), true);
    }

    // empty args gives correct options
    void cli_opts_test1()
    {
        cli_options opts = call_parse({_exec_name});
        assert(opts.parser.get() == nullptr);
        assert(opts.dry_run == false);
        assert(opts.should_exit == true);
    }

    // -h and --help give correct options
    void cli_opts_test2()
    {
        {
            cli_options opts = call_parse({_exec_name, "-h"});
            assert(opts.parser.get() == nullptr);
            assert(opts.dry_run == false);
            assert(opts.should_exit == true);
        }

        {
            cli_options opts = call_parse({_exec_name, "--help"});
            assert(opts.parser.get() == nullptr);
            assert(opts.dry_run == false);
            assert(opts.should_exit == true);
        }
    }

    // --dry-run sets correct options
    void cli_opts_test3()
    {
        {
            cli_options opts = call_parse({_exec_name, "--dry-run"});
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
                call_parse({_exec_name, "--file", "not_a_real_file.test"});
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
            cli_options opts = call_parse({_exec_name, "--file"});
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
            cli_options opts = call_parse({_exec_name, "--not-a-valid-flag"});
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
                call_parse({_exec_name, "[clobber_1xn]", "XOXO", "{B}"});
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
        using namespace file_parser_test;

        cli_options opts =
            call_parse({_exec_name, "[nogo_1xn] X..O X...O..X {B win}"});
        assert(opts.parser.get() != nullptr);
        assert(opts.dry_run == false);
        assert(opts.should_exit == false);

        vector<csv_row*> rows;

        add_row(rows, BLACK, WIN_TEXT,
                {new nogo_1xn("X..O"), new nogo_1xn("X...O..X")},
                COMMAND_TYPE_SOLVE_BW);

        assert_file_parser_output(opts.parser.get(), rows);
        for (csv_row* row : rows)
            delete row;
    }

    // --file gives correct file/parser
    void cli_opts_test9()
    {
        using namespace file_parser_test;

        cli_options opts = call_parse(
            {_exec_name, "--file",
             get_test_exec_input_path() / "file_parser/sumgames4.test"});
        assert(opts.parser.get() != nullptr);
        assert(opts.dry_run == false);
        assert(opts.should_exit == false);

        vector<csv_row*> rows;

        add_row(rows, BLACK, LOSS_TEXT, {new clobber_1xn("XOOX")},
                COMMAND_TYPE_SOLVE_BW);

        assert_file_parser_output(opts.parser.get(), rows);

        for (csv_row* row : rows)
            delete row;
    }

    // --test-timeout
    void cli_opts_test10()
    {
        cli_options opts = call_parse({_exec_name, "--test-timeout", "42734"});
        assert(opts.test_timeout == 42734);
    }

    // --out-file
    void cli_opts_test11()
    {
        cli_options opts = call_parse({_exec_name, "--out-file", "65432.csv"});
        assert(opts.outfile_name == "65432.csv");
    }

    // --test-dir
    void cli_opts_test12()
    {
        cli_options opts = call_parse({_exec_name, "--test-dir", "somedir462"});
        assert(opts.test_directory == "somedir462");
    }

}; // class cli_options_test_class
} // namespace

void cli_options_test_all(const char* exec_path)
{
    cli_options_test_class test_class(exec_path);

    test_class.cli_opts_test1();
    test_class.cli_opts_test2();
    test_class.cli_opts_test3();
    test_class.cli_opts_test4();
    test_class.cli_opts_test5();
    test_class.cli_opts_test6();
    test_class.cli_opts_test7();
    test_class.cli_opts_test8();
    test_class.cli_opts_test9();

    test_class.cli_opts_test10();
    test_class.cli_opts_test11();
    test_class.cli_opts_test12();
}
