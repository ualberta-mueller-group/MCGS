import sys

# <template cpp> <output cpp>
assert len(sys.argv) == 3


infile = open(sys.argv[1], "r")
outfile = open(sys.argv[2], "w")

replacements = ["YESERROR", "NOERROR"]

error_count = 0
var_number = 1

for line in infile:
    repl = None
    upper = False

    if line.find("CHECK") != -1:
        upper = True

    for r in replacements:
        if line.find(r) != -1:
            repl = r
            break

    if repl == "YESERROR":
        error_count += 1

    if repl is not None:
        old = "CHECK" if upper else "check"
        if not upper:
            repl = repl.lower()
        repl += "_" + str(var_number)
        var_number += 1
        line = line.replace(old, repl)

    outfile.write(line)

print(f"Expected errors: {error_count}")

outfile.close()
infile.close()
