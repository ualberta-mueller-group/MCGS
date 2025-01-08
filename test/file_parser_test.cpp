#include "file_parser_test.h"

#include "test_utilities.h"
#include "all_game_headers.h"
#include "file_parser.h"
#include <fstream>
#include <sstream>

using std::cout, std::endl, std::string, std::ifstream, std::stringstream;

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
        //cout << e.what() << endl;

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
    delete parser;

}

void assert_throw_status_string(const string& file_name, bool should_throw, parser_exception_code code)
{
    string file_content = "";
    ifstream input_file(input_root_dir + file_name);

    assert(input_file.is_open());

    string line;
    while (getline(input_file, line))
    {
        file_content += line + " ";
    }

    file_parser* parser = file_parser::from_string(file_content);
    _assert_throw_status(parser, should_throw, code);
    delete parser;
}


////////////////////////////// invalid input

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
    assert_throw_status_string("missing_version.test", false, PARSER_OK);
}

void e2e_test3()
{
    assert_throw_status_file("wrong_version.test", true, WRONG_VERSION_COMMAND);
    assert_throw_status_string("wrong_version.test", true, WRONG_VERSION_COMMAND);
}

///// games without sections
void e2e_test4()
{
    assert_throw_status_file("missing_section.test", true, MISSING_SECTION_TITLE);
    assert_throw_status_string("missing_section.test", true, MISSING_SECTION_TITLE);
}

///// invalid section titles (games without parsers)
void e2e_test5()
{
    assert_throw_status_file("missing_parser.test", true, MISSING_SECTION_PARSER);
    assert_throw_status_string("missing_parser.test", true, MISSING_SECTION_PARSER);
}

///// unmatched "brackets"
void e2e_test6()
{
    // [clobber_1xn]]
    assert_throw_status_file("fail_match1.test", true, FAILED_MATCH);
    assert_throw_status_string("fail_match1.test", true, FAILED_MATCH);
}

void e2e_test7()
{
    // ((XOXO.X))
    assert_throw_status_file("fail_match2.test", true, FAILED_MATCH);
    assert_throw_status_string("fail_match2.test", true, FAILED_MATCH);
}

void e2e_test8()
{
    // [[clobber_1xn]]
    assert_throw_status_file("fail_match3.test", true, FAILED_MATCH);
    assert_throw_status_string("fail_match3.test", true, FAILED_MATCH);
}

void e2e_test9()
{
    assert_throw_status_file("fail_match4.test", true, FAILED_MATCH);
    assert_throw_status_string("fail_match4.test", true, FAILED_MATCH);
}

///// invalid commands
void e2e_test10()
{
    assert_throw_status_file("invalid_command1.test", true, FAILED_CASE_COMMAND);
    assert_throw_status_string("invalid_command1.test", true, FAILED_CASE_COMMAND);
}

void e2e_test11()
{
    assert_throw_status_file("invalid_command2.test", true, FAILED_CASE_COMMAND);
    assert_throw_status_string("invalid_command2.test", true, FAILED_CASE_COMMAND);
}

void e2e_test12()
{
    assert_throw_status_file("invalid_command3.test", true, EMPTY_CASE_COMMAND);
    assert_throw_status_string("invalid_command3.test", true, EMPTY_CASE_COMMAND);
}

///// reserved characters outside of comments
void e2e_test13() {
    assert_throw_status_file("invalid_reserved_characters1.test", true, FAILED_MATCH);
    assert_throw_status_string("invalid_reserved_characters1.test", true, FAILED_MATCH);
}

void e2e_test14() {
    assert_throw_status_file("invalid_reserved_characters2.test", true, FAILED_MATCH);
    assert_throw_status_string("invalid_reserved_characters2.test", true, FAILED_MATCH);
}

///// invalid game tokens
void e2e_test15() {
    assert_throw_status_file("invalid_game1.test", true, FAILED_GAME_TOKEN_PARSE);
    assert_throw_status_string("invalid_game1.test", true, FAILED_GAME_TOKEN_PARSE);
}

void e2e_test16() {
    assert_throw_status_file("invalid_game2.test", true, FAILED_GAME_TOKEN_PARSE);
    assert_throw_status_string("invalid_game2.test", true, FAILED_GAME_TOKEN_PARSE);
}

///// missing whitespace
void e2e_test17() {
    assert_throw_status_file("missing_whitespace1.test", true, FAILED_MATCH);
    assert_throw_status_string("missing_whitespace1.test", true, FAILED_MATCH);
}

void e2e_test18() {
    assert_throw_status_file("missing_whitespace2.test", true, FAILED_MATCH);
    assert_throw_status_string("missing_whitespace2.test", true, FAILED_MATCH);
}


////////////////////////////// valid input

// reserved characters in comments
void e2e_test19() {
    assert_throw_status_file("comment_reserved_chars1.test", false, PARSER_OK);
    assert_throw_status_string("comment_reserved_chars1.test", false, PARSER_OK);
}

// not calling parse_chunk() over the whole file
void e2e_test20() {
    // First figure out how many cases are in the file
    int ncases = 0;

    string file_name = input_root_dir + "partial_read.test";

    {
        file_parser* parser = file_parser::from_file(file_name);
        game_case gc;

        while (parser->parse_chunk(gc))
        {
            ncases++;
            gc.cleanup_games();
        }

        delete parser;
    }

    int half = ncases / 2;

    assert(ncases >= 6);
    assert(half >= 2);

    // Read the file again, but only read half of it
    {
        file_parser* parser = file_parser::from_file(file_name);
        game_case gc;

        for (int i = 0; i < half; i++)
        {
            assert(parser->parse_chunk(gc));
            gc.cleanup_games();
        }

        delete parser;
    }

    // No exception should have been thrown
}


////////////////////////////// some sums of games

void e2e_test21()
{
    vector<game_case *> cases;

    {
        game_case* gc = new game_case();
        cases.push_back(gc);

        gc->to_play = BLACK;
        gc->expected_outcome = TEST_OUTCOME_WIN;
        gc->games.push_back(new nogo_1xn("X..O"));
    }

    {
        game_case* gc = new game_case();
        cases.push_back(gc);

        gc->to_play = WHITE;
        gc->expected_outcome = TEST_OUTCOME_WIN;
        gc->games.push_back(new nogo_1xn("X..O"));
    }

    {
        game_case* gc = new game_case();
        cases.push_back(gc);

        gc->to_play = BLACK;
        gc->expected_outcome = TEST_OUTCOME_LOSS;
        gc->games.push_back(new integer_game(4));
        gc->games.push_back(new integer_game(-5));
    }

    {
        game_case* gc = new game_case();
        cases.push_back(gc);

        gc->to_play = WHITE;
        gc->expected_outcome = TEST_OUTCOME_WIN;
        gc->games.push_back(new integer_game(4));
        gc->games.push_back(new integer_game(-5));
    }

    {
        game_case* gc = new game_case();
        cases.push_back(gc);

        gc->to_play = BLACK;
        gc->expected_outcome = TEST_OUTCOME_WIN;
        gc->games.push_back(new clobber_1xn("XOXOXOXO"));
    }

    {
        game_case* gc = new game_case();
        cases.push_back(gc);

        gc->to_play = WHITE;
        gc->expected_outcome = TEST_OUTCOME_UNKNOWN;
        gc->games.push_back(new clobber_1xn("XOXOXOXO"));
    }

    {
        game_case* gc = new game_case();
        cases.push_back(gc);

        gc->to_play = BLACK;
        gc->expected_outcome = TEST_OUTCOME_LOSS;
    }

    {
        game_case* gc = new game_case();
        cases.push_back(gc);

        gc->to_play = WHITE;
        gc->expected_outcome = TEST_OUTCOME_LOSS;
    }


    assert_file_parser_output_file(input_root_dir + "sumgames1.test", cases);

    for (game_case* gc : cases) 
    {
        gc->cleanup_games();
        delete gc;
    }
}

void e2e_test22()
{
    vector<game_case *> cases;

    assert_file_parser_output_file(input_root_dir + "sumgames2.test", cases);

    for (game_case* gc : cases) 
    {
        gc->cleanup_games();
        delete gc;
    }
}

void e2e_test23()
{
    vector<game_case *> cases;

    assert_file_parser_output_file(input_root_dir + "sumgames3.test", cases);

    for (game_case* gc : cases) 
    {
        gc->cleanup_games();
        delete gc;
    }
}

void e2e_test24()
{
    vector<game_case *> cases;

    {
        game_case* gc = new game_case();
        cases.push_back(gc);

        gc->to_play = BLACK;
        gc->expected_outcome = TEST_OUTCOME_LOSS;
        gc->games.push_back(new clobber_1xn("XOOX"));
    }

    assert_file_parser_output_file(input_root_dir + "sumgames4.test", cases);

    for (game_case* gc : cases) 
    {
        gc->cleanup_games();
        delete gc;
    }
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
    e2e_test13();
    e2e_test14();
    e2e_test15();
    e2e_test16();
    e2e_test17();
    e2e_test18();
    e2e_test19();
    e2e_test20();
    e2e_test21();
    e2e_test22();
    e2e_test23();
    e2e_test24();
}


////////////////////////////////////////////////// main function
void file_parser_test_all()
{
    end_to_end_tests();
}
