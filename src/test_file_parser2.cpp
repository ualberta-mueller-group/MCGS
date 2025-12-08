#include "test_file_parser2.h"
#include "file_parser2.h"
#include <iostream>

using namespace std;

//////////////////////////////////////////////////
void test_file_parser2_stuff()
{
    file_parser2* fp = file_parser2::from_file("test.test");

    bool first = true;
    while (fp->parse_chunk())
    {
        if (!first)
            cout << endl;
        first = false;

        fp->print_ast();
    }

    delete fp;
}

