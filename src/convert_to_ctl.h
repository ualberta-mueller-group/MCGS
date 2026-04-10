#pragma once

#include <memory>
#include <string>

#include "file_parser.h"
#include "test_filter.h"

void convert_tests_to_ctl_format(std::shared_ptr<file_parser> fp,
                                 const std::string& output_dir_name,
                                 test_filter_enum filter_type);

void convert_tests_to_ctl_format(const std::string& test_directory,
                                 const std::string& output_dir_name,
                                 test_filter_enum filter_type);
