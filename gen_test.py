import random

outfile = open("tests.test2", "w")

n_tests = 40
min_pairs = 10
max_pairs = 12

for i in range(n_tests):
    test = "XO" * random.randint(min_pairs, max_pairs)
    line = f"[clobber_1xn] {test} " + "{B}\n"
    outfile.write(line)

outfile.close()
