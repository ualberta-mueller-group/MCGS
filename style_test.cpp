#include <iostream>


using namespace std;


void some_function(const int some_var)
{
}


struct some_struct
{
public:
    int noerror_1; // NOERROR
    int _yeserror_2; // YESERROR
    const int noerror_3; // NOERROR
    const int _yeserror_4; // YESERROR
    int YESERROR_5; // YESERROR
    int _YESERROR_6; // YESERROR
    const int YESERROR_7; // YESERROR
    const int _YESERROR_8; // YESERROR

    static int noerror_9; // NOERROR
    static int _noerror_10; // NOERROR
    static const int yeserror_11; // YESERROR
    static const int _yeserror_12; // YESERROR
    static int YESERROR_13; // YESERROR
    static int _YESERROR_14; // YESERROR
    static const int NOERROR_15; // NOERROR
    static const int _NOERROR_16; // NOERROR

protected:
    int yeserror_17; // YESERROR
    int _noerror_18; // NOERROR
    const int yeserror_19; // YESERROR
    const int _noerror_20; // NOERROR
    int YESERROR_21; // YESERROR
    int _YESERROR_22; // YESERROR
    const int YESERROR_23; // YESERROR
    const int _YESERROR_24; // YESERROR

    static int noerror_25; // NOERROR
    static int _noerror_26; // NOERROR
    static const int yeserror_27; // YESERROR
    static const int _yeserror_28; // YESERROR
    static int YESERROR_29; // YESERROR
    static int _YESERROR_30; // YESERROR
    static const int NOERROR_31; // NOERROR
    static const int _NOERROR_32; // NOERROR



private:
    int yeserror_33; // YESERROR
    int _noerror_34; // NOERROR
    const int yeserror_35; // YESERROR
    const int _noerror_36; // NOERROR
    int YESERROR_37; // YESERROR
    int _YESERROR_38; // YESERROR
    const int YESERROR_39; // YESERROR
    const int _YESERROR_40; // YESERROR

    static int noerror_41; // NOERROR
    static int _noerror_42; // NOERROR
    static const int yeserror_43; // YESERROR
    static const int _yeserror_44; // YESERROR
    static int YESERROR_45; // YESERROR
    static int _YESERROR_46; // YESERROR
    static const int NOERROR_47; // NOERROR
    static const int _NOERROR_48; // NOERROR




};


int main() {



    return 0;
}
