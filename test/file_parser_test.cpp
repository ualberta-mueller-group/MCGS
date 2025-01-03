#include "file_parser_test.h"
#include "file_parser.h"

using std::cout, std::endl, std::string;


const string input_root_dir = "test/input/file_parser/";



////////////////////////////////////////////////// end to end tests

void _assert_throw_status(file_parser* parser, bool should_throw, parser_exception_code code)
{
    game_case gc;

    bool did_throw = false;

    try {
        while (parser->parse_chunk(gc))
        {
            gc.cleanup_games();
        }

    } catch (parser_exception& e)
    {
        cout << e.what() << endl;

        assert(e.code() == code);
        did_throw = true;
    }

    gc.cleanup_games();

    assert(did_throw == should_throw);
}

void assert_throw_status_file(const string& file_name, bool should_throw, parser_exception_code code)
{
    file_parser* parser = file_parser::from_file(input_root_dir + file_name);
    _assert_throw_status(parser, should_throw, code);
}


////////////////////////////// valid input
////////////////////////////// invalid input
/*

- end to end tests (from file and string)
    - valid input
        - not calling parse_chunk() over the whole file
        - reserved characters in comments
    - invalid input
///// reserved characters outside of comments
///// invalid commands
///// missing/wrong version command
///// missing whitespace
///// invalid game tokens


///// wrong CLI flags
- unit tests
    - test helper functions (from a friend function?)
   */




////////// wrong file name
void e2e_test1()
{
    bool did_throw = false;
    file_parser* parser = nullptr;

    try
    {
        parser = file_parser::from_file(input_root_dir + "some_nonexistent_file.test");
    }
    catch (std::ios_base::failure& e)
    {
        did_throw = true;
    }

    assert(did_throw);
    assert(parser == nullptr);
}

///// missing/wrong version command
void e2e_test2()
{
    assert_throw_status_file("missing_version.test", true, MISSING_VERSION_COMMAND);
}

void e2e_test3()
{
    assert_throw_status_file("wrong_version.test", true, WRONG_VERSION_COMMAND);
}

///// games without sections
void e2e_test4()
{
    assert_throw_status_file("missing_section.test", true, MISSING_SECTION_TITLE);
}

///// invalid section titles (games without parsers)
void e2e_test5()
{
    assert_throw_status_file("missing_parser.test", true, MISSING_SECTION_PARSER);
}

///// unmatched "brackets"
void e2e_test6()
{
    assert_throw_status_file("fail_match1.test", true, FAILED_MATCH);
}

void e2e_test7()
{

}

void e2e_test8()
{

}

void e2e_test9()
{

}

void e2e_test10()
{

}

void e2e_test11()
{

}

void e2e_test12()
{

}



void end_to_end_tests()
{
    e2e_test1();
    e2e_test2();
    e2e_test3();
    e2e_test4();
    e2e_test5();
    e2e_test6();
    e2e_test7();
    e2e_test8();
    e2e_test9();
    e2e_test10();
    e2e_test11();
    e2e_test12();


}


////////////////////////////////////////////////// unit tests

void unit_tests()
{

}


////////////////////////////////////////////////// main function
void file_parser_test_all()
{
    end_to_end_tests();
    unit_tests();
}
