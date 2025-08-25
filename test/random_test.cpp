#include "random_test.h"
#include "random.h"
#include <unordered_set>
#include <cstdint>
#include <cassert>
#include <cstddef>

using namespace std;

namespace {

void test_mixed()
{
    unordered_set<uint32_t> nums;
    random_generator& global_rng = get_global_rng();

    for (size_t i = 0; i < 100; i++)
    {
        uint64_t val64 = global_rng.get_u64();
        nums.insert(val64 & 0xFFFFFFFF);
        nums.insert((val64 >> 32) & 0xFFFFFFFF);

        uint64_t val32 = global_rng.get_u32();
        nums.insert(val32);
    }

    assert(nums.size() == 300);
}

void test_u64()
{
    unordered_set<uint64_t> nums;
    random_generator& global_rng = get_global_rng();

    for (size_t i = 0; i < 100; i++)
    {
        uint64_t val = global_rng.get_u64();
        nums.insert(val);
    }

    assert(nums.size() == 100);
}

void test_u32()
{
    unordered_set<uint32_t> nums;
    random_generator& global_rng = get_global_rng();

    for (size_t i = 0; i < 100; i++)
    {
        uint32_t val = global_rng.get_u32();
        nums.insert(val);
    }

    assert(nums.size() == 100);
}

} // namespace

void random_test_all()
{
    test_mixed();
    test_u64();
    test_u32();
}
