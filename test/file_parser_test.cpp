#include "file_parser_test.h"

#include "test_utilities.h"
#include "all_game_headers.h"
#include "file_parser.h"
#include <fstream>
#include <sstream>

using std::cout, std::endl, std::string, std::ifstream, std::stringstream;
using std::vector;

const string INPUT_ROOT_DIR = "test/input/file_parser/";

////////////////////////////////////////////////// end to end tests

// Should file_parser warn for wrong version?
enum version_warn {
    MAYBE_WARN, // indifferent to whether or not it warns
    SHOULD_WARN,
    SHOULD_NOT_WARN,
};


void assert_throw_status(file_parser* parser, bool should_throw, parser_exception_code code, version_warn vw)
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

    if (vw != MAYBE_WARN)
    {
        bool expected_warn = (vw == SHOULD_WARN);
        assert(parser->warned_wrong_version() == expected_warn);
    }

    assert(did_throw == should_throw);
}

void assert_throw_status_file(const string& file_name, bool should_throw, parser_exception_code code, version_warn vw = MAYBE_WARN)
{
    file_parser* parser = file_parser::from_file(INPUT_ROOT_DIR + file_name);
    assert_throw_status(parser, should_throw, code, vw);
    delete parser;

}

void assert_throw_status_string(const string& file_name, bool should_throw, parser_exception_code code, version_warn vw = MAYBE_WARN)
{
    string file_content = "";
    ifstream input_file(INPUT_ROOT_DIR + file_name);

    assert(input_file.is_open());

    string line;
    while (getline(input_file, line))
    {
        file_content += line + " ";
    }

    file_parser* parser = file_parser::from_string(file_content);
    assert_throw_status(parser, should_throw, code, vw);
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
        parser = file_parser::from_file(INPUT_ROOT_DIR + "some_nonexistent_file.test");
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
    assert_throw_status_string("missing_version.test", false, PARSER_OK, SHOULD_NOT_WARN);
}

void e2e_test3()
{
    assert_throw_status_file("wrong_version.test", false, PARSER_OK, SHOULD_WARN);
    assert_throw_status_string("wrong_version.test", false, PARSER_OK, SHOULD_WARN);
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

void more_invalid_commands()
{
    assert_throw_status_file("invalid_command4.test", true, FAILED_CASE_COMMAND);
    assert_throw_status_string("invalid_command4.test", true, FAILED_CASE_COMMAND);


    assert_throw_status_file("invalid_command5.test", true, FAILED_CASE_COMMAND);
    assert_throw_status_string("invalid_command5.test", true, FAILED_CASE_COMMAND);


    assert_throw_status_file("invalid_command6.test", true, FAILED_CASE_COMMAND);
    assert_throw_status_string("invalid_command6.test", true, FAILED_CASE_COMMAND);


    assert_throw_status_file("invalid_command7.test", true, FAILED_CASE_COMMAND);
    assert_throw_status_string("invalid_command7.test", true, FAILED_CASE_COMMAND);


    assert_throw_status_file("invalid_command8.test", true, FAILED_CASE_COMMAND);
    assert_throw_status_string("invalid_command8.test", true, FAILED_CASE_COMMAND);
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

    string file_name = INPUT_ROOT_DIR + "partial_read.test";

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
        gc->expected_outcome = TEST_RESULT_WIN;
        gc->games.push_back(new nogo_1xn("X..O"));
    }

    {
        game_case* gc = new game_case();
        cases.push_back(gc);

        gc->to_play = WHITE;
        gc->expected_outcome = TEST_RESULT_WIN;
        gc->games.push_back(new nogo_1xn("X..O"));
    }

    {
        game_case* gc = new game_case();
        cases.push_back(gc);

        gc->to_play = BLACK;
        gc->expected_outcome = TEST_RESULT_LOSS;
        gc->games.push_back(new integer_game(4));
        gc->games.push_back(new integer_game(-5));
    }

    {
        game_case* gc = new game_case();
        cases.push_back(gc);

        gc->to_play = WHITE;
        gc->expected_outcome = TEST_RESULT_WIN;
        gc->games.push_back(new integer_game(4));
        gc->games.push_back(new integer_game(-5));
    }

    {
        game_case* gc = new game_case();
        cases.push_back(gc);

        gc->to_play = BLACK;
        gc->expected_outcome = TEST_RESULT_WIN;
        gc->games.push_back(new clobber_1xn("XOXOXOXO"));
    }

    {
        game_case* gc = new game_case();
        cases.push_back(gc);

        gc->to_play = WHITE;
        gc->expected_outcome = TEST_RESULT_UNSPECIFIED;
        gc->games.push_back(new clobber_1xn("XOXOXOXO"));
    }

    {
        game_case* gc = new game_case();
        cases.push_back(gc);

        gc->to_play = BLACK;
        gc->expected_outcome = TEST_RESULT_LOSS;
    }

    {
        game_case* gc = new game_case();
        cases.push_back(gc);

        gc->to_play = WHITE;
        gc->expected_outcome = TEST_RESULT_LOSS;
    }


    assert_file_parser_output_file(INPUT_ROOT_DIR + "sumgames1.test", cases);

    for (game_case* gc : cases) 
    {
        gc->cleanup_games();
        delete gc;
    }
}

void e2e_test22()
{
    vector<game_case *> cases;

    assert_file_parser_output_file(INPUT_ROOT_DIR + "sumgames2.test", cases);

    for (game_case* gc : cases) 
    {
        gc->cleanup_games();
        delete gc;
    }
}

void e2e_test23()
{
    vector<game_case *> cases;

    assert_file_parser_output_file(INPUT_ROOT_DIR + "sumgames3.test", cases);

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
        gc->expected_outcome = TEST_RESULT_LOSS;
        gc->games.push_back(new clobber_1xn("XOOX"));
    }

    assert_file_parser_output_file(INPUT_ROOT_DIR + "sumgames4.test", cases);

    for (game_case* gc : cases) 
    {
        gc->cleanup_games();
        delete gc;
    }

}

// Comment stuff...

// Make sure _, #0, #1 all work
void e2e_test25()
{
    file_parser* p = file_parser::from_file(INPUT_ROOT_DIR + "comments.test");

    game_case case1;
    game_case case2;
    assert(p->parse_chunk(case1));
    assert(p->parse_chunk(case2));

    assert(case1.comments.find("A") != string::npos);
    assert(case1.comments.find("B") == string::npos);
    assert(case1.comments.find("C") != string::npos);
    assert(case1.comments.find("D") == string::npos);

    assert(case2.comments.find("A") != string::npos);
    assert(case2.comments.find("B") == string::npos);
    assert(case2.comments.find("C") == string::npos);
    assert(case2.comments.find("D") != string::npos);

    case1.cleanup_games();
    case2.cleanup_games();

    delete p;
}

//"#3"
void e2e_test26()
{
    assert_throw_status_file("invalid_comment1.test", true, BAD_COMMENT_FORMAT);
    assert_throw_status_string("invalid_comment1.test", true, BAD_COMMENT_FORMAT);
}

//"#"
void e2e_test27()
{
    assert_throw_status_file("invalid_comment2.test", true, BAD_COMMENT_FORMAT);
    assert_throw_status_string("invalid_comment2.test", true, BAD_COMMENT_FORMAT);
}


//#D
void e2e_test28()
{
    assert_throw_status_file("invalid_comment3.test", true, BAD_COMMENT_FORMAT);
    assert_throw_status_string("invalid_comment3.test", true, BAD_COMMENT_FORMAT);
}


//#0B
void e2e_test29()
{
    assert_throw_status_file("invalid_comment4.test", true, BAD_COMMENT_FORMAT);
    assert_throw_status_string("invalid_comment4.test", true, BAD_COMMENT_FORMAT);
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
    more_invalid_commands();

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
    e2e_test25();
    e2e_test26();
    e2e_test27();
    e2e_test28();
    e2e_test29();

}


////////////////////////////////////////////////// main function
void file_parser_test_all()
{
    bool warning_state = file_parser::silence_warnings;
    file_parser::silence_warnings = true;

    end_to_end_tests();

    file_parser::silence_warnings = warning_state;
}
