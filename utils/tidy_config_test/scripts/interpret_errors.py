import sys

# <expected> <got> <out>
assert len(sys.argv) == 4

exp_file = open(sys.argv[1], "r")
got_file = open(sys.argv[2], "r")

exp = [x for x in exp_file]
got = [x for x in got_file]

exp_file.close()
got_file.close()

out_file = open(sys.argv[3], "w")


for x in got:
    if x not in exp:
        out_file.write(f"FALSE POSITIVE {x}\n")

for x in exp:
    if x not in got:
        out_file.write(f"MISSING ERROR {x}\n")

out_file.close()


