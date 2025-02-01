infile = open("style_test_template.cpp", "r")
outfile = open("style_test.cpp", "w")

replacements = ["YESERROR", "NOERROR"]

error_count = 0

for line in infile:
    repl = None
    upper = False

    if line.find("VAR") != -1:
        upper = True

    for r in replacements:
        if line.find(r) != -1:
            repl = r
            break

    if repl == "YESERROR":
        error_count += 1

    if repl is not None:
        old = "VAR" if upper else "var"
        if not upper:
            repl = repl.lower()
        line = line.replace(old, repl)

    outfile.write(line)

print(f"Expected errors: {error_count}")

outfile.close()
infile.close()
