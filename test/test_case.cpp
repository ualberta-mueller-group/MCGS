//---------------------------------------------------------------------------
// Read test cases for combinatorial game defined in a text file
//---------------------------------------------------------------------------
#include "test_case.h"

#include <cassert>
#include <cctype>
#include <iomanip>
#include <fstream>
#include <iostream>
#include <vector>


std::istream& operator>>(std::istream& is, test_case& c)
{
    std::string color_string, win_string;
    is >> std::quoted(c._game) >> color_string >> win_string;
    c._black_first = (toupper(color_string[0]) == 'B');
    c._is_win = (toupper(win_string[0]) == 'W');
//     if (is.good())
//         std::cout << "Read test_case " << c._game
//         << ' ' << c._black_first << ' ' << c._is_win << std::endl;
    return is;
}

bool read_test_cases(const std::string& file_name, 
                     std::string& game_name,
                     int& version,
                     std::vector<test_case>& cases)
{
    std::ifstream is(file_name);
    if (! is.is_open())
    {
        std::cout << "Error: could not open file "
                  << file_name << std::endl;
        return false;
    }
    is >> game_name >> version;
    if (! is.good())
    {
        std::cout << "Error: could not read game name or version" 
                  << std::endl;
        return false;
    }
    assert(version == 0);
    while (true)
    {
        test_case next_case;
        is >> next_case;
        if (is.good())
            cases.push_back(next_case);
        else
        {
            std::cout << "Read " << cases.size() << ' '
                      << game_name << " test cases"
                      << std::endl;
            return true;
        }
    }
}
