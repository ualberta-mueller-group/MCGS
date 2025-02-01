got_file = open("got_errors.txt", "r")
exp_file = open("expected_errors.txt", "r")

got = [x for x in got_file]
exp = [x for x in exp_file]

exp_file.close()
got_file.close()

out_file = open("errors.txt", "w")


for x in got:
    if x not in exp:
        out_file.write(f"FALSE POSITIVE {x}\n")

for x in exp:
    if x not in got:
        out_file.write(f"MISSING ERROR {x}\n")

out_file.close()


