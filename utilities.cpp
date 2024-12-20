#include "utilities.h"
#include <sstream>


using std::vector, std::string, std::stringstream;

vector<string> split_string(const string& str)
{
    vector<string> strs;


    stringstream stream(str);

    string next_string;
    while (stream >> next_string)
    {
        strs.push_back(next_string);
    }

    return strs;
}

bool is_int(const string& str)
{
    const int N = str.size();

    if (N == 0)
    {
        return false;
    }

    for (int i = 0; i < N; i++)
    {
        const char& c = str[i];

        if (!isdigit(c) && i != 0 && c != '-')
        {
            return false;
        }
    }

    return true;
}

