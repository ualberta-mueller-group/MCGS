#pragma once

#include <optional>
#include <string>
#include <vector>
#include <cstdint>

#include "test_case_enums.h"
#include "game_case.h"

/*
   Comments indicate where each field might be filled in
*/

class csv_row
{
public:
    bool has_visitor_fields() const;
    bool has_pre_test_fields() const;
    bool has_post_test_fields() const;
    bool has_autotest_fields() const;

    void fill_visitor_fields(const std::vector<std::string>& comments,
                             command_type_enum command_type,
                             const std::string& input_hash);

    void fill_pre_test_fields(
        const std::vector<game*>& games, std::optional<ebw> player,
        const std::optional<std::string>& expected_result);

    /*
        For reporting test results.

        The non-verbose version infers the test status by comparing the result
        to the expected result.

        The verbose version is used by tests which provide more meaningful
        output i.e. the "winning moves" test showing a diff of the expected
        moves and computed moves
    */
    void fill_post_test_fields(const std::optional<std::string>& result,
                               double time_ms);

    void fill_post_test_fields_verbose(
        const std::optional<std::string>& alt_result, double time_ms,
        test_case_status_enum test_case_status);

    void fill_autotest_fields(const std::string& file, int case_number);

    std::string get_status_string() const;
    std::string get_command_type_string() const;
    std::string get_time_ms_string() const;

    /*
        Returns non-sanitized field contents. Use the write_csv_field_strings()
        utility function to format the fields and write them to an output
        .csv file
    */
    std::vector<std::string> get_row_field_strings() const;
    static std::vector<std::string> get_header_field_strings();

    std::optional<std::string> file;               // autotests
    std::optional<int> case_number;                // autotests
    std::optional<std::string> games;              // visitor
    std::optional<std::string> player;             // visitor
    std::optional<std::string> expected_result;    // pre test
    std::optional<std::string> result;             // post test
    std::optional<double> time_ms;                 // post test
    std::optional<test_case_status_enum> status;   // post test
    std::optional<std::string> comments;           // visitor

    std::optional<command_type_enum> command_type; // visitor

    std::optional<uint64_t> node_count;            // post test
    std::optional<uint64_t> unique_node_count;     // post test
    std::optional<std::string> input_hash;         // visitor
};

/*
    Writes one line of a .csv file (including newline). Input is non-sanitized
    fields.

    Sanitizes and formats each field:
        1. Strip left and right whitespace
        2. Replace double quotes with 2 single quotes
        3. Wrap the result in double quotes

    Join fields with ','. Print line to stream (including newline).
*/
void write_csv_field_strings(std::ostream& os, const std::vector<std::string>& fields);
