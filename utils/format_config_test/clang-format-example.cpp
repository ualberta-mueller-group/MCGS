/*
    This file is useful for testing clang-format options
*/

static_assert(false, "This file isn't meant to be compiled");

#include <vector>
#include <string>

std::vector<int> find_asdasd(std::vector<int> asd, int asd2, std::string sfdoijsdf, std::vector<float>, std::string ashdasdiuhasdiuhasd, double asdasd);

void some_other_func(int x, std::string asd, int y, std::string z);

void some_func1(int one, int two, std::string three);

void some_func2(int one, int two, std::string three, std::vector<unsigned long long> four);

void some_func3_aoijdsasodijasdoijasdoijasasdd(int one, int two, std::string three, std::vector<unsigned long long> four, double five, int six, std::vector<int> seven, int eight, long nine, long long ten);

inline void some_func4()
{
    some_other_func(5, "abcd", 5, "efg");

    some_other_func(5, (std::string) "abcd" + "qwertyuiopasdfghjkl;zxcvbnm", 5, "efg");

    some_other_func(5, (std::string) "abcd" + "qwertyuiopasdfghjkl;zxcvbnm" + "oafjisgrhdibojpkfwaijesgrdofbkpj", 5, "efg");

}

struct some_struct_a
{

    some_struct_a() : x("asdoi"), y("aso"), z("asdoijsoij"), w("asp")
    {
    }

    std::string x;
    std::string y;
    std::string z;
    std::string w;
};

struct some_struct_b
{

    some_struct_b() : x("asdoiajsfoiasjfoi"), y("aso"), z("asdoijsoij"), w("asp")
    {
    }

    std::string x;
    std::string y;
    std::string z;
    std::string w;
};

struct some_struct_c
{

    some_struct_c() : x("asdoiajsfoiasjfoisdjfosidjf"), y("asodijawojasodfij"), z("asdoijsoij"), w("aspodjaspodjaspofj")
    {
    }

    std::string x;
    std::string y;
    std::string z;
    std::string w;
};
