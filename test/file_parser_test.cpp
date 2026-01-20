#include "file_parser_test.h"
#include "csv_row.h"
#include "test_case.h"
#include "test_case_enums.h"
#include "test_utilities.h"
#include "all_game_headers.h"
#include "file_parser.h"
#include <fstream>
#include <memory>
#include <sstream>
#include <iostream>
#include <string>
#include <cassert>
#include <vector>
#include <ios>

using std::cout, std::endl, std::string, std::ifstream, std::stringstream;
using std::vector;

namespace {
const string INPUT_ROOT_DIR = "test/input/file_parser/";

////////////////////////////////////////////////// end to end tests

// Specifies whether file_parser is expected to warn for wrong version
enum version_warn
{
    MAYBE_WARN, // indifferent to whether or not it warns
    SHOULD_WARN,
    SHOULD_NOT_WARN,
};

void assert_throw_status(file_parser* parser, bool should_throw,
                         parser_exception_code code, version_warn vw)
{
    bool did_throw = false;

    try
    {
        while (parser->parse_chunk())
        {
            const int n_test_cases = parser->n_test_cases();
            for (int i = 0; i < n_test_cases; i++)
                std::shared_ptr<i_test_case> test_case = parser->get_test_case(i);
        }
    }
    catch (parser_exception& e)
    {
        // cout << e.what() << endl;

        assert(e.code() == code);
        did_throw = true;
    }

    if (vw != MAYBE_WARN)
    {
        bool expected_warn = (vw == SHOULD_WARN);
        assert(parser->warned_wrong_version() == expected_warn);
    }

    assert(did_throw == should_throw);
}

void assert_throw_status_file(const string& file_name, bool should_throw,
                              parser_exception_code code,
                              version_warn vw = MAYBE_WARN)
{
    file_parser* parser = file_parser::from_file(INPUT_ROOT_DIR + file_name);
    assert_throw_status(parser, should_throw, code, vw);
    delete parser;
}

void assert_throw_status_string(const string& file_name, bool should_throw,
                                parser_exception_code code,
                                version_warn vw = MAYBE_WARN)
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
        parser = file_parser::from_file(INPUT_ROOT_DIR +
                                        "some_nonexistent_file.test");
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
    assert_throw_status_file("missing_version.test", true,
                             MISSING_VERSION_COMMAND);
    assert_throw_status_string("missing_version.test", false, PARSER_OK,
                               SHOULD_NOT_WARN);
}

void e2e_test3()
{
    assert_throw_status_file("wrong_version.test", false, PARSER_OK,
                             SHOULD_WARN);
    assert_throw_status_string("wrong_version.test", false, PARSER_OK,
                               SHOULD_WARN);
}

///// games without sections
void e2e_test4()
{
    assert_throw_status_file("missing_section.test", true,
                             MISSING_SECTION_TITLE);
    assert_throw_status_string("missing_section.test", true,
                               MISSING_SECTION_TITLE);
}

///// invalid section titles (games without parsers)
void e2e_test5()
{
    assert_throw_status_file("missing_parser.test", true,
                             MISSING_SECTION_PARSER);
    assert_throw_status_string("missing_parser.test", true,
                               MISSING_SECTION_PARSER);
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
    assert_throw_status_file("invalid_command1.test", true,
                             FAILED_CASE_COMMAND);
    assert_throw_status_string("invalid_command1.test", true,
                               FAILED_CASE_COMMAND);
}

void e2e_test11()
{
    assert_throw_status_file("invalid_command2.test", true,
                             FAILED_CASE_COMMAND);
    assert_throw_status_string("invalid_command2.test", true,
                               FAILED_CASE_COMMAND);
}

void e2e_test12()
{
    assert_throw_status_file("invalid_command3.test", true, FAILED_CASE_COMMAND);
    assert_throw_status_string("invalid_command3.test", true,
                               FAILED_CASE_COMMAND);
}

void more_invalid_commands()
{
    assert_throw_status_file("invalid_command4.test", true,
                             FAILED_CASE_COMMAND);
    assert_throw_status_string("invalid_command4.test", true,
                               FAILED_CASE_COMMAND);

    assert_throw_status_file("invalid_command5.test", true,
                             FAILED_CASE_COMMAND);
    assert_throw_status_string("invalid_command5.test", true,
                               FAILED_CASE_COMMAND);

    assert_throw_status_file("invalid_command6.test", true,
                             FAILED_CASE_COMMAND);
    assert_throw_status_string("invalid_command6.test", true,
                               FAILED_CASE_COMMAND);

    assert_throw_status_file("invalid_command7.test", true,
                             FAILED_CASE_COMMAND);
    assert_throw_status_string("invalid_command7.test", true,
                               FAILED_CASE_COMMAND);

    assert_throw_status_file("invalid_command8.test", true,
                             FAILED_CASE_COMMAND);
    assert_throw_status_string("invalid_command8.test", true,
                               FAILED_CASE_COMMAND);

    assert_throw_status_file("invalid_command9.test", true,
                                 FAILED_CASE_COMMAND);
    assert_throw_status_string("invalid_command9.test", true,
                               FAILED_CASE_COMMAND);

}

///// reserved characters outside of comments
void e2e_test13()
{
    assert_throw_status_file("invalid_reserved_characters1.test", true,
                             FAILED_MATCH);
    assert_throw_status_string("invalid_reserved_characters1.test", true,
                               FAILED_MATCH);
}

void e2e_test14()
{
    assert_throw_status_file("invalid_reserved_characters2.test", true,
                             FAILED_MATCH);
    assert_throw_status_string("invalid_reserved_characters2.test", true,
                               FAILED_MATCH);
}

///// invalid game tokens
void e2e_test15()
{
    assert_throw_status_file("invalid_game1.test", true,
                             FAILED_GAME_TOKEN_PARSE);
    assert_throw_status_string("invalid_game1.test", true,
                               FAILED_GAME_TOKEN_PARSE);
}

void e2e_test16()
{
    assert_throw_status_file("invalid_game2.test", true,
                             FAILED_GAME_TOKEN_PARSE);
    assert_throw_status_string("invalid_game2.test", true,
                               FAILED_GAME_TOKEN_PARSE);
}

///// missing whitespace
void e2e_test17()
{
    assert_throw_status_file("missing_whitespace1.test", true, FAILED_MATCH);
    assert_throw_status_string("missing_whitespace1.test", true, FAILED_MATCH);
}

void e2e_test18()
{
    assert_throw_status_file("missing_whitespace2.test", true, FAILED_MATCH);
    assert_throw_status_string("missing_whitespace2.test", true, FAILED_MATCH);
}

////////////////////////////// valid input

// reserved characters in comments
void e2e_test19()
{
    assert_throw_status_file("comment_reserved_chars1.test", false, PARSER_OK);
    assert_throw_status_string("comment_reserved_chars1.test", false,
                               PARSER_OK);
}

// not calling parse_chunk() over the whole file
void e2e_test20()
{
    // First figure out how many cases are in the file
    int actual_cases = 0;

    string file_name = INPUT_ROOT_DIR + "partial_read.test";

    {
        file_parser* parser = file_parser::from_file(file_name);

        while (parser->parse_chunk())
        {
            const int n_test_cases = parser->n_test_cases();
            for (int i = 0; i < n_test_cases; i++)
            {
                std::shared_ptr<i_test_case> test_case = parser->get_test_case(i);
                actual_cases++;
            }
        }

        delete parser;
    }

    int half = actual_cases / 2;

    assert(actual_cases >= 6);
    assert(half >= 2);

    // Read the file again, but only read half of it
    {
        file_parser* parser = file_parser::from_file(file_name);

        int did_read = 0;

        while (parser->parse_chunk())
        {
            bool should_break = false;

            const int n_test_cases = parser->n_test_cases();

            for (int i = 0; i < n_test_cases; i++)
            {
                std::shared_ptr<i_test_case> test_cases = parser->get_test_case(i);
                did_read++;

                if (did_read == half)
                {
                    should_break = true;
                    break;
                }
            }

            if (should_break)
                break;
        }

        delete parser;
    }

    // No exception should have been thrown
}

////////////////////////////// some sums of games
#define WIN_TEXT "Win"
#define LOSS_TEXT "Loss"

void e2e_test21()
{
    using namespace file_parser_test;
    std::vector<csv_row*> rows;

    add_row(rows, BLACK, WIN_TEXT, {new nogo_1xn("X..O")},
                              COMMAND_TYPE_SOLVE_BW);
    add_row(rows, WHITE, WIN_TEXT, {new nogo_1xn("X..O")},
                              COMMAND_TYPE_SOLVE_BW);

    add_row(rows, BLACK, LOSS_TEXT,
                              {new integer_game(4), new integer_game(-5)},
                              COMMAND_TYPE_SOLVE_BW);
    add_row(rows, WHITE, WIN_TEXT,
                              {new integer_game(4), new integer_game(-5)},
                              COMMAND_TYPE_SOLVE_BW);

    add_row(rows, BLACK, WIN_TEXT,
                              {new clobber_1xn("XOXOXOXO")},
                              COMMAND_TYPE_SOLVE_BW);
    add_row(rows, WHITE, {}, {new clobber_1xn("XOXOXOXO")},
                              COMMAND_TYPE_SOLVE_BW);

    add_row(rows, BLACK, LOSS_TEXT, {},
                              COMMAND_TYPE_SOLVE_BW);
    add_row(rows, WHITE, LOSS_TEXT, {},
                              COMMAND_TYPE_SOLVE_BW);

    assert_file_parser_output_file(
        INPUT_ROOT_DIR + "sumgames1.test", rows);

    for (csv_row* row : rows)
        delete row;
}

void e2e_test22()
{
    using namespace file_parser_test;

    vector<csv_row*> rows;
    assert_file_parser_output_file(
        INPUT_ROOT_DIR + "sumgames2.test", rows);

    for (csv_row* row : rows)
        delete row;
}

void e2e_test23()
{
    using namespace file_parser_test;

    vector<csv_row*> rows;
    assert_file_parser_output_file(
        INPUT_ROOT_DIR + "sumgames3.test", rows);

    for (csv_row* row : rows)
        delete row;
}

void e2e_test24()
{
    using namespace file_parser_test;

    vector<csv_row*> rows;

    add_row(rows, BLACK, LOSS_TEXT, {new clobber_1xn("XOOX")},
            COMMAND_TYPE_SOLVE_BW);

    assert_file_parser_output_file(INPUT_ROOT_DIR + "sumgames4.test", rows);
    for (csv_row* row : rows)
        delete row;
}

// Comment stuff...

// Make sure _, #0, #1 all work
void e2e_test25()
{
    auto get_all_test_comments =
        [](const std::string& file_name) -> std::vector<std::string>
    {
        std::vector<std::string> comments;
        file_parser* p = file_parser::from_file(file_name);

        while (p->parse_chunk())
        {
            const int n_test_cases = p->n_test_cases();

            for (int i = 0; i < n_test_cases; i++)
            {
                std::shared_ptr<i_test_case> test_case = p->get_test_case(i);
                comments.emplace_back(test_case->get_csv_row().comments.value());
            }
        }

        delete p;
        return comments;
    };

    std::vector<std::string> comments =
        get_all_test_comments(INPUT_ROOT_DIR + "comments.test");

    assert(comments.size() == 2);

    const std::string& comments1 = comments[0];
    const std::string& comments2 = comments[1];


    assert(comments1.find("A") != string::npos);
    assert(comments1.find("B") == string::npos);
    assert(comments1.find("C") != string::npos);
    assert(comments1.find("D") == string::npos);

    assert(comments2.find("A") != string::npos);
    assert(comments2.find("B") == string::npos);
    assert(comments2.find("C") == string::npos);
    assert(comments2.find("D") != string::npos);
}

//"#3"
void e2e_test26()
{
    assert_throw_status_file("invalid_comment1.test", true, BAD_COMMENT_FORMAT);
    assert_throw_status_string("invalid_comment1.test", true,
                               BAD_COMMENT_FORMAT);
}

//"#"
void e2e_test27()
{
    assert_throw_status_file("invalid_comment2.test", true, BAD_COMMENT_FORMAT);
    assert_throw_status_string("invalid_comment2.test", true,
                               BAD_COMMENT_FORMAT);
}

// #D
void e2e_test28()
{
    assert_throw_status_file("invalid_comment3.test", true, BAD_COMMENT_FORMAT);
    assert_throw_status_string("invalid_comment3.test", true,
                               BAD_COMMENT_FORMAT);
}

// #0B
void e2e_test29()
{
    assert_throw_status_file("invalid_comment4.test", true, BAD_COMMENT_FORMAT);
    assert_throw_status_string("invalid_comment4.test", true,
                               BAD_COMMENT_FORMAT);
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

} // namespace

////////////////////////////////////////////////// main function
void file_parser_test_all()
{
    bool warning_state = file_parser::silence_warnings;
    file_parser::silence_warnings = true;

    end_to_end_tests();

    file_parser::silence_warnings = warning_state;
}
