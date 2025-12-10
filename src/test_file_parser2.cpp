#include "test_file_parser2.h"
#include "csv_row.h"
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

        //fp->print_ast();

        const int n_tests = fp->n_test_cases();
        for (int i = 0; i < n_tests; i++)
        {
            std::shared_ptr<i_test_case> test_case = fp->get_test_case(i);
            const csv_row& row = test_case->get_csv_row();

            cout << row.player.value() << endl;
            cout << row.games.value() << endl;
            cout << row.input_hash.value() << endl;

            test_case->run(0);

            cout << row.result.value() << endl;
        }
    }

    delete fp;
}

