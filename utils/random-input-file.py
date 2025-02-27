"""
up_star
nimber
integers and rationals
switch
"""

import sys
from random import randint
from fractions import Fraction

CASE_COUNT = 10
CASE_LEN = 5

assert len(sys.argv) == 2
outfile_name = sys.argv[1]

outfile = open(outfile_name, "w")



def bool_to_star(star):
    if star:
        return "*"
    return ""



#outfile.write("{version 1.0}\n[up_star]\n")
#outfile.write("{version 1.0}\n[nimber]\n")
outfile.write("{version 1.0}\n")

for i in range(CASE_COUNT):
    """
    # generate random up_star
    case_ups = 0
    case_star = False

    for i in range(CASE_LEN):
        local_ups = randint(-6, 6)
        local_star = randint(0, 1) == 0

        case_star ^= local_star
        case_ups += local_ups
        outfile.write(f"\t({local_ups} {bool_to_star(local_star)})\n")

    outfile.write(f"/{case_ups} {bool_to_star(case_star)}\\\n")
    """

    """
    # generate random nimbers
    sum = 0
    for i in range(CASE_LEN):
        x = randint(0, 6)

        sum ^= x
        outfile.write(f"\t{x}\n")
    outfile.write(f"/*{sum}\\\n")
    outfile.write("{B}\n")
    """

    # generate random integers and rationals
    sum = 0
    for i in range(CASE_LEN):
        if randint(0, 100) < 50:
            # int
            x = randint(-10, 10)
            sum += x
            outfile.write(f"\t[integer_game] {x}\n")
        else:
            # rational
            x = Fraction(randint(-100, 100), (1 << randint(1, 10)))
            sum += x
            outfile.write(f"\t[dyadic_rational] ({x.numerator} {x.denominator})\n")
    outfile.write(f"/{sum}\\\n")
    outfile.write("{B}\n")


outfile.close()
