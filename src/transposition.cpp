#include "transposition.h"

class ttable_entry_empty
{
};

class ttable_entry_non_empty
{
public:
    bool win;
};

using namespace std;

typedef ttable<ttable_entry_empty> ttable_empty;
typedef ttable<ttable_entry_non_empty> ttable_non_empty;

void test_transposition()
{
    {
        ttable_non_empty t1(30, 0);
    }

    {
        ttable_empty t2(30, 1);
    }

}
