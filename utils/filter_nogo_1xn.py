import sys
import re

assert len(sys.argv) == 2
input_filename = sys.argv[1]
input_file = open(input_filename, "r")

reject_re = re.compile(r"(XO+X)|(OX+O)|(^X+O)|(^O+X)|(OX+$)|(XO+$)")

for line in input_file:
    line = line.strip()
    if reject_re.search(line) is None:
        print(line)

input_file.close()

