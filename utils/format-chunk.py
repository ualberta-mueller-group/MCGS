import subprocess
import os
import sys


use_vim = False
chunk_no = -1

args = sys.argv
for arg in sys.argv[1 : ]:
    if arg == "vim":
        use_vim = True
        continue
    if arg[0] == "-":
        chunk_no = int(arg[1 : ])
        continue


def run_command(cmd):
    return subprocess.run(cmd, shell=True, capture_output=True, text=True)


def get_files():
    h_regex = "\\(.*\\.h$\\)"
    cpp_regex = "\\(.*\\.cpp$\\)"
    ext_regex = f"\\({h_regex}\\|{cpp_regex}\\)"
    dir_regex = "\\./\\(\\(src\\)\\|\\(test\\)\\)"
    proc = run_command(f"find -regex '{dir_regex}{ext_regex}'")
    assert proc.returncode == 0
    files = [x for x in proc.stdout.split() if "___transformed" not in x]
    return files


def exists(f):
    proc = run_command(f"test -e {f}")
    return proc.returncode == 0


def get_transformed(f):
    f1 = f.replace(".h", "___transformed.h")
    f2 = f1.replace(".cpp", "___transformed.cpp")
    return f2


files_list = get_files()

chunks = []

for i in range(len(files_list)):
    if (i % 10) == 0:
        chunks.append([])
    f = files_list[i]
    chunks[-1].append(f)


while not chunk_no in range(len(chunks)):
    chunk_no = int(input(f"Pick a chunk [0 - {len(chunks)}): "))

assert chunk_no >= 0 and chunk_no < len(chunks)

chunk = chunks[chunk_no]

lint_files_string = "LINT_FILES=\""
for f in chunk:
    lint_files_string += f + " "
lint_files_string += "\""


command = f"{lint_files_string} make format"
print(command)


proc = run_command(command)
print(proc.stdout)

summary_file = "format_stdout.txt"
f = open(summary_file, "w")
f.write(f"Chunk number: {chunk_no}\n")
f.write(proc.stdout)
f.close()

if use_vim:
    vim_files=""
    for f in chunk:
        ft = get_transformed(f)
        vim_files += f"tabnew {f} | "
        if exists(ft):
            vim_files += f"vsplit {ft} | "

    vim_command = ["nvim", "format_stdout.txt", f"+{vim_files}tabnext"]
    subprocess.call(vim_command)

    os.remove(summary_file)
