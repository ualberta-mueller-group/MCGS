#pragma once

#include <cstdint>
#include <random>

class new_random
{
public:
    new_random(uint64_t seed);

    int8_t get_i8();
    int8_t get_i8_biased(int8_t min_val = INT8_MIN, int8_t max_val = INT8_MAX);
    int8_t get_i8_unbiased_reject(int8_t min_val = INT8_MIN, int8_t max_val = INT8_MAX);
    int8_t get_i8_unbiased_openbsd(int8_t min_val = INT8_MIN, int8_t max_val = INT8_MAX);
    int8_t get_i8_unbiased_paper(int8_t min_val = INT8_MIN, int8_t max_val = INT8_MAX);


private:
    std::mt19937_64 _rng;

};

//////////////////////////////////////////////////
void test_new_random();

/*

   Gen space [0, 4]
   5

   Output space [0, 1]
   2

   Outer space
   3

   Excluded space
   1


   0, 1, 2, 3, 4
   o  o  o  o  x



*/
