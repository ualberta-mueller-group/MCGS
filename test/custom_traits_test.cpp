#include "custom_traits_test.h"
#include "custom_traits.h"
#include "all_game_headers.h"
#include <memory>

using namespace custom_traits;
using namespace std;

/*
    NOTE: this test runs at compile time and not run time
*/
void custom_traits_test_all()
{
    // is_smart_ptr
    static_assert(is_smart_ptr_v<shared_ptr<int>>);
    static_assert(is_smart_ptr_v<unique_ptr<float>>);

    static_assert(!is_smart_ptr_v<int*>);
    static_assert(!is_smart_ptr_v<int>);

    // is_some_ptr
    static_assert(is_some_ptr_v<int*>);
    static_assert(is_some_ptr_v<shared_ptr<int>>);
    static_assert(is_some_ptr_v<unique_ptr<float>>);

    static_assert(!is_some_ptr_v<int>);

    // is_some_game_ptr
    static_assert(is_some_game_ptr_v<game*>);
    static_assert(is_some_game_ptr_v<shared_ptr<game>>);
    static_assert(is_some_game_ptr_v<unique_ptr<game>>);

    static_assert(is_some_game_ptr_v<integer_game*>);
    static_assert(is_some_game_ptr_v<shared_ptr<clobber_1xn>>);
    static_assert(is_some_game_ptr_v<unique_ptr<nogo_1xn>>);

    static_assert(!is_some_game_ptr_v<int>);
    static_assert(!is_some_game_ptr_v<int*>);
}
