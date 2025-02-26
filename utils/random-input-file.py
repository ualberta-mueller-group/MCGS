"""
up_star
nimber
integers and rationals
switch
"""

import sys
from random import randint

CASE_COUNT = 10
CASE_LEN = 5

assert len(sys.argv) == 2
outfile_name = sys.argv[1]

outfile = open(outfile_name, "w")

outfile.write("{version 1.0}\n[up_star]\n")


def bool_to_star(star):
    if star:
        return "*"
    return ""


for i in range(CASE_COUNT):
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
    outfile.write("{B, W}\n")

outfile.close()
