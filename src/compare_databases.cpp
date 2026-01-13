#include "compare_databases.h"

#include <filesystem>
#include <iostream>
#include <string>
#include "database.h"
#include "throw_assert.h"

using namespace std;

void compare_databases(const string& db_file_name_1,
                       const string& db_file_name_2)
{
    THROW_ASSERT(std::filesystem::exists(db_file_name_1));
    database db1;
    db1.load(db_file_name_1);

    THROW_ASSERT(std::filesystem::exists(db_file_name_2));
    database db2;
    db2.load(db_file_name_2);

    const bool is_equal = db1.is_equal(db2);

    cout << "Database files ";
    cout << "\"" << db_file_name_1 << "\"";
    cout << " and ";
    cout << "\"" << db_file_name_2 << "\"";

    cout << ' ';

    if (is_equal)
        cout << "are equal!";
    else
        cout << "differ!";

    cout << endl;
}
