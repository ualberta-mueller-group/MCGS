#include "normalize_test.h"

#include "normalize_test_clobber_1xn.h"
#include "normalize_test_nogo_1xn.h"
#include "normalize_test_elephants.h"

void normalize_test_all()
{
    normalize_test_clobber_1xn();
    normalize_test_nogo_1xn();
    normalize_test_elephants();
}
