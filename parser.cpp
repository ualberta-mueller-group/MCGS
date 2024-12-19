#include "parser.h"
#include <fstream>
#include <iostream>
#include <cassert>


/*
    TODO expand on this and make it not awful
   */

using std::string, std::ifstream, std::cout, std::endl, std::cin;

void assert_macro_format(const string& s)
{
    int N = s.size();

    assert(N > 0);
    assert(s[0] == '{');
    assert(s.back() == '}');

    for (int i = 1; i < N - 1; i++)
    {
        const char c = s[i];
        assert(c != '{');
        assert(c != '}');
    }
}

void assert_section_format(const string& s)
{
    int N = s.size();

    assert(N > 0);
    assert(s[0] == '(');
    assert(s.back() == ')');

    for (int i = 1; i < N - 1; i++)
    {
        const char c = s[i];
        assert(c != '(');
        assert(c != ')');
    }
}


void parse(const string& file_name)
{
    bool macro_open = false;
    bool quote_open = false;
    bool in_section = false;

    ifstream stream(file_name);

    if (!stream.is_open())
    {
        cout << "Failed to open" << endl;
    }

    // Read the file...

    cout << "Parsing..." << endl;
    string chunk;

    string token;

    while (stream >> token)
    {
        if (token[0] == '(' && token.back() == ')')
        {
            assert_section_format(token);
            chunk.clear();
            cout << "FOUND SECTION: " << token << endl;
            continue;
        }


        chunk = (chunk.size() > 0) ? chunk + " " + token : chunk + token;

        if (!macro_open && token[0] == '{')
        {
            macro_open = true;
            chunk = token;
        }


        // Looking for macro
        if (macro_open)
        {
            if (token.back() == '}')
            {
                cout << "FOUND MACRO: " << chunk << endl;
                assert_macro_format(chunk);
                macro_open = false;
                chunk.clear();
            }
            continue;
        }



    }
}

