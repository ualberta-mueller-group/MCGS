#ifndef test_case_H
#define test_case_H

#include <istream>
#include <string>
#include <vector>

struct test_case
{
    std::string _game;
    bool _black_first;
    bool _is_win;
};

std::istream& operator>>(std::istream& is, test_case& one_case);

// Returns true on success, false on any read failure
bool read_test_cases(const std::string& file_name, 
                     std::string& game_name,
                     int& version,
                     std::vector<test_case>& cases);

#endif // test_case_H
