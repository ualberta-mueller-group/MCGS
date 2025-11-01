#pragma once
/*
    Game solving function exported to WebAssembly binary
*/

#include <iostream>
#include <string>

struct js_struct
{
    js_struct(const std::string& result, int n_cases)
        : result(result),
          n_cases(n_cases)
    {
        std::cout << "JS STRUCT CONS" << std::endl;
    }

    ~js_struct()
    {
        std::cout << "JS STRUCT DEST" << std::endl;
    }

    std::string result;
    int n_cases;
};



js_struct js_solve(const std::string& game_string);

