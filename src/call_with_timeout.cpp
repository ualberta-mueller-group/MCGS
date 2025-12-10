#include "call_with_timeout.h"
#include "utilities.h"
using namespace std;


namespace {
optional<int> fib(const mcgs_stop_token& st, int n)
{
    if (st.stop_requested())
        return {};

    if (n <= 1)
        return n;

    optional<int> f1 = fib(st, n - 1);
    optional<int> f2 = fib(st, n - 2);

    if (f1.has_value() && f2.has_value())
        return *f1 + *f2;

    return {};
}
} // namespace

class fib_class1
{
public:
    optional<int> fib(const mcgs_stop_token& st, int n)
    {
        if (st.stop_requested())
            return {};

        if (n <= 1)
            return n;

        optional<int> f1 = fib(st, n - 1);
        optional<int> f2 = fib(st, n - 2);

        if (f1.has_value() && f2.has_value())
            return *f1 + *f2;

        return {};
    }

private:
};

class fib_class2
{
public:
    optional<int> fib(const mcgs_stop_token& st, int n)
    {
        if (st.stop_requested())
            return {};

        if (n <= 1)
            return n;

        optional<int> f1 = fib(st, n - 1);
        optional<int> f2 = fib(st, n - 2);

        if (f1.has_value() && f2.has_value())
            return *f1 + *f2;

        return {};
    }

private:
};

//////////////////////////////////////////////////
void test_call_with_timeout()
{
    fib_class1 fc1;
    fib_class2 fc2;

    optional<int> x1 = call_with_timeout(1000, fib, 20);
    optional<int> x2 = call_with_timeout(500, &fc1, &fib_class1::fib, 50);
    optional<int> x3 = call_with_timeout(500, &fc2, &fib_class2::fib, 50);
    //optional<int> x4 = call_with_timeout(500, &fc1, &fib_class2::fib, 50);
    //optional<int> x5 = call_with_timeout(500, &fc2, &fib_class1::fib, 50);

    cout << print_optional(x1) << " ";
    cout << print_optional(x2) << " ";
    cout << print_optional(x3) << endl;

}

