#pragma once

#include <memory>
#include <string>

#include "file_parser.h"

void convert_tests_to_segclobber(std::shared_ptr<file_parser> fp,
                                 const std::string& output_dir_name);

void convert_tests_to_segclobber(const std::string& test_directory,
                                 const std::string& output_dir_name);
