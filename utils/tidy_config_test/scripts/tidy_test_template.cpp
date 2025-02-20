#include <iostream>


using namespace std;

int check = 0; // NOERROR
int _check = 0; // YESERROR
const int check = 0; // YESERROR
const int _check = 0; // YESERROR
int CHECK = 0; // YESERROR
int _CHECK = 0; // YESERROR
const int CHECK = 0; // NOERROR
const int _CHECK = 0; // YESERROR

static int check = 0; // NOERROR
static int _check = 0; // YESERROR
static const int check = 0; // YESERROR
static const int _check = 0; // YESERROR
static int CHECK = 0; // YESERROR
static int _CHECK = 0; // YESERROR
static const int CHECK = 0; // NOERROR
static const int _CHECK = 0; // YESERROR

enum CHECK_ENUM // YESERROR
{
    check, // YESERROR
};

enum check_enum // NOERROR
{
    CHECK, // NOERROR
};

struct CHECK_STRUCT // YESERROR
{
};

struct check_struct // NOERROR
{
};

void CHECK_FUNCTION() // YESERROR
{
}

void check_function() // NOERROR
{
}

constexpr int check = 0; // YESERROR
constexpr int CHECK = 0; // NOERROR

constexpr int CHECK_FUNCTION() // YESERROR
{
    return 5;
};


constexpr int check_function() // NOERROR
{
    return 5;
};

void some_func(int x, const int CHECK) // YESERROR
{
}

struct some_other_struct
{
public:
    static constexpr int CHECK = 5; // NOERROR
    static constexpr int check = 6; // YESERROR

    constexpr int check() // NOERROR
    {
        return 5;
    }

    constexpr int CHECK() // YESERROR
    {
        return 5;
    }
};


struct some_struct
{
public:
    int check; // NOERROR
    int _check; // YESERROR
    const int check; // NOERROR
    const int _check; // YESERROR
    int CHECK; // YESERROR
    int _CHECK; // YESERROR
    const int CHECK; // YESERROR
    const int _CHECK; // YESERROR

    static int check; // NOERROR
    static int _check; // NOERROR
    static const int check; // YESERROR
    static const int _check; // YESERROR
    static int CHECK; // YESERROR
    static int _CHECK; // YESERROR
    static const int CHECK; // NOERROR
    static const int _CHECK; // NOERROR

protected:
    int check; // YESERROR
    int _check; // NOERROR
    const int check; // YESERROR
    const int _check; // NOERROR
    int CHECK; // YESERROR
    int _CHECK; // YESERROR
    const int CHECK; // YESERROR
    const int _CHECK; // YESERROR

    static int check; // NOERROR
    static int _check; // NOERROR
    static const int check; // YESERROR
    static const int _check; // YESERROR
    static int CHECK; // YESERROR
    static int _CHECK; // YESERROR
    static const int CHECK; // NOERROR
    static const int _CHECK; // NOERROR

private:
    int check; // YESERROR
    int _check; // NOERROR
    const int check; // YESERROR
    const int _check; // NOERROR
    int CHECK; // YESERROR
    int _CHECK; // YESERROR
    const int CHECK; // YESERROR
    const int _CHECK; // YESERROR

    static int check; // NOERROR
    static int _check; // NOERROR
    static const int check; // YESERROR
    static const int _check; // YESERROR
    static int CHECK; // YESERROR
    static int _CHECK; // YESERROR
    static const int CHECK; // NOERROR
    static const int _CHECK; // NOERROR
};


int main() {
    int check = 0; // NOERROR
    int _check = 0; // YESERROR
    const int check = 0; // NOERROR
    const int _check = 0; // YESERROR
    int CHECK = 0; // YESERROR
    int _CHECK = 0; // YESERROR
    const int CHECK = 0; // NOERROR
    const int _CHECK = 0; // YESERROR

    static int check = 0; // NOERROR
    static int _check = 0; // YESERROR
    static const int check = 0; // YESERROR
    static const int _check = 0; // YESERROR
    static int CHECK = 0; // YESERROR
    static int _CHECK = 0; // YESERROR
    static const int CHECK = 0; // NOERROR
    static const int _CHECK = 0; // YESERROR


    return 0;
}
