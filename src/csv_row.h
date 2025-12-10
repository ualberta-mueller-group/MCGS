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

    void fill_visitor_fields(const std::vector<std::string>& comments,
                             command_type_enum command_type,
                             const std::string& input_hash);

    void fill_pre_test_fields(
        const std::vector<game*>& games, std::optional<ebw> player,
        const std::optional<std::string>& expected_result);

    void fill_post_test_fields(const std::optional<std::string>& result,
                               double time_ms);

    std::string get_status_string() const;
    std::string get_command_type_string() const;
    std::string get_time_ms_string() const;


    std::optional<std::string> file;               // autotests
    std::optional<int> case_number;                // autotests
    std::optional<std::string> games;              // visitor
    std::optional<std::string> player;             // visitor
    std::optional<std::string> expected_result;    // pre test
    std::optional<std::string> result;             // post test
    std::optional<double> time_ms;                 // post test
    std::optional<uint64_t> node_count;            // post test
    std::optional<uint64_t> unique_node_count;     // post test
    std::optional<test_case_status_enum> status;   // post test
    std::optional<std::string> comments;           // visitor
    std::optional<command_type_enum> command_type; // visitor
    std::optional<std::string> input_hash;         // visitor
};
