#include <iostream>


using namespace std;


void some_function(const int some_var)
{
}


struct some_struct
{
public:
    int var_1; // NOERROR
    int _var_2; // YESERROR
    const int var_3; // NOERROR
    const int _var_4; // YESERROR
    int VAR_5; // YESERROR
    int _VAR_6; // YESERROR
    const int VAR_7; // YESERROR
    const int _VAR_8; // YESERROR

    static int var_9; // NOERROR
    static int _var_10; // NOERROR
    static const int var_11; // YESERROR
    static const int _var_12; // YESERROR
    static int VAR_13; // YESERROR
    static int _VAR_14; // YESERROR
    static const int VAR_15; // NOERROR
    static const int _VAR_16; // NOERROR

protected:
    int var_17; // YESERROR
    int _var_18; // NOERROR
    const int var_19; // YESERROR
    const int _var_20; // NOERROR
    int VAR_21; // YESERROR
    int _VAR_22; // YESERROR
    const int VAR_23; // YESERROR
    const int _VAR_24; // YESERROR

    static int var_25; // NOERROR
    static int _var_26; // NOERROR
    static const int var_27; // YESERROR
    static const int _var_28; // YESERROR
    static int VAR_29; // YESERROR
    static int _VAR_30; // YESERROR
    static const int VAR_31; // NOERROR
    static const int _VAR_32; // NOERROR



private:
    int var_33; // YESERROR
    int _var_34; // NOERROR
    const int var_35; // YESERROR
    const int _var_36; // NOERROR
    int VAR_37; // YESERROR
    int _VAR_38; // YESERROR
    const int VAR_39; // YESERROR
    const int _VAR_40; // YESERROR

    static int var_41; // NOERROR
    static int _var_42; // NOERROR
    static const int var_43; // YESERROR
    static const int _var_44; // YESERROR
    static int VAR_45; // YESERROR
    static int _VAR_46; // YESERROR
    static const int VAR_47; // NOERROR
    static const int _VAR_48; // NOERROR




};


int main() {



    return 0;
}
