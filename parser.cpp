#include "parser.h"
#include <fstream>
#include <iostream>
#include <cassert>

using std::string, std::ifstream, std::cout, std::cin, std::endl;

bool token_is_format(const string& token, const char& open, const char& close)
{
    int N = token.size();

    if (N <= 0 || token[0] != open || token.back() != close)
    {
        return false;
    }

    for (int i = 1; i < N - 1; i++)
    {
        const char c = token[i];

        if (c == open || c == close)
        {
            return false;
        }
    }

    return true;
}

string get_format(string chunk, ifstream& stream, const char& open, const char& close)
{
    if (chunk.back() == close)
    {
        assert(token_is_format(chunk, open, close));
        return chunk;
    }

    string token;
    while (stream >> token)
    {
        chunk += " " + token;
        if (chunk.back() == close)
        {
            break;
        }
    }

    if (chunk.back() == close)
    {
        assert(token_is_format(chunk, open, close));
        return chunk;
    }

    assert(false);
    return "";
}

void strip_title(string& title)
{
    assert(title.size() >= 2);
    title.pop_back();
    title = title.substr(1);
}

void parse(const string& file_name)
{
    ifstream stream(file_name);

    if (!stream.is_open())
    {
        cout << "Failed to open" << endl;
        return;
    }


    string token;
    string chunk;

    string current_title;

    while (stream >> token)
    {
        // Match macro
        if (token[0] == '{')
        {
            chunk = get_format(token, stream, '{', '}');
            cout << "GOT MACRO: " << chunk << endl;
            continue;
        }

        // Match title
        if (token[0] == '(')
        {
            chunk = token;
            assert(token_is_format(token, '(', ')'));
            strip_title(chunk);
            cout << "GOT TITLE: " << chunk << endl;
            current_title = chunk;
            continue;
        }


        // Match quote
        if (token[0] == '\"')
        {
            chunk = get_format(token, stream, '\"', '\"');
            cout << "GOT QUOTE: " << chunk << endl;
            continue;
        }

        // Token must be a game_token
        cout << "GOT GAME TOKEN: " << token << endl;

    }

}

