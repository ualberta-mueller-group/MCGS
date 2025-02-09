#include "simple_text_hash.h"

#include <string>
#include <cassert>
#include "simple_text_hash.h"

using namespace std;

namespace
{
    void assert_different_hash(const string& str1, const string& str2)
    {
        simple_text_hash hash1;
        hash1.update(str1);
        string hash_string1 = hash1.get_string();

        simple_text_hash hash2;
        hash2.update(str2);
        string hash_string2 = hash2.get_string();

        assert(hash_string1 != hash_string2);
    }

    void test1()
	{
        assert_different_hash("XOXO", "OXOX");
	}

    // Test input that "wraps around" the buffer
    void test2()
	{
        string input1 = "X";

        string input2 = input1;
        for (int i = 0; i < simple_text_hash::buffer_size; i++)
        {
            input2 += input1;
        }

        assert_different_hash(input1, input2);
	}

    void test3()
	{
        assert_different_hash("1", "2");
	}

    void test4()
	{
        assert_different_hash("1", "");
	}

    void test5()
	{
        // intentional misspelling:
        assert_different_hash("Different by one", "Diferent by one");
	}
}

void simple_text_hash_test_all()
{
    test1();
    test2();
    test3();
    test4();
    test5();
}
