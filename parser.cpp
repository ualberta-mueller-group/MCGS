#include "parser.h"
#include <fstream>
#include <iostream>
#include <cassert>

using std::string, std::ifstream, std::cout, std::cin, std::endl;

/*
    (clobber_1xn) <-- game title
    {B win} <-- command

*/

bool is_enclosed_format(const string& token, const char& open, const char& close)
{
    if (token.size() < 2)
    {
        return false;
    }

    if (token[0] != open || token.back() != close)
    {
        return false;
    }

    int N = token.size();
    for (int i = 1; i < N - 1; i++)
    {
        const char& c = token[i];

        // TODO check for reserved characters instead
        if (c == open || c == close)
        {
            return false;
        }
    }

    return true;
}

bool get_enclosed(ifstream& stream, string& token, const char& open, const char& close)
{
    if (token[0] != open)
    {
        return false;
    }

    if (is_enclosed_format(token, open, close))
    {
        return true;
    }

    string new_token;
    while (stream >> new_token)
    {
        token += " " + new_token;

        if (is_enclosed_format(token, open, close))
        {
            return true;
        }

    }

    return false;
}


void parse(const string& file_name)
{
    ifstream stream(file_name);

    if (!stream.is_open())
    {
        cout << "Failed to open file: " << file_name << endl;
        return;
    }

    string token;
    while (stream >> token)
    {
        // Match command
        if (token[0] == '{')
        {
            bool success =  get_enclosed(stream, token, '{', '}');

            if (!success)
            {
                cout << "Parser error on line ?: Failed to match command" << endl;
                return;
            }

            cout << "Got command: " << token << endl;
            continue;
        }

        // Match title
        if (token[0] == '(')
        {
            bool success = get_enclosed(stream, token, '(', ')');

            if (!success)
            {
                cout << "Parser error on line ?: Failed to match game title" << endl;
                return;
            }

            cout << "Got game title: " << token << endl;
            continue;
        }

        // Match quotes
        if (token[0] == '"')
        {
            bool success = get_enclosed(stream, token, '"', '"');

            if (!success)
            {
                cout << "Parser error on line ?: Failed to match quotes" << endl;
                return;
            }

            cout << "Got quotes: " << token << endl;
            continue;
        }

        // Match comment
        if (token[0] == '/')
        {
            bool success = get_enclosed(stream, token, '/', '/');

            if (!success)
            {
                cout << "Parser error on line ?: Failed to match comment" << endl;
            }

            cout << "Got comment: " << token << endl;
            continue;
        }

        // Must be game token
        cout << "Got simple token: " << token << endl;

    }

}
