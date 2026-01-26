/*
   Compare two database files

   Use with CLI option --db-file-compare

   TODO currently called from mcgs_init_2()
*/
#pragma once
#include <string>

void compare_databases(const std::string& db_file_name_1,
                       const std::string& db_file_name_2);
