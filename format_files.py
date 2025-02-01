import subprocess
import sys
from pathlib import Path
import os

summary_filename = "format_result.txt"
transform_suffix = "___transformed"

assert len(sys.argv) > 1

args = sys.argv[1 : ]

if args[0] == "--delete":
    print("Deleting transformations...")
    for file_name in args[1 : ]:
        if transform_suffix in file_name:
            print(f"Deleting {file_name}")
            os.remove(file_name)
        else:
            print(f"Skipping {file_name}")
            continue
    print("Done!")
    exit(0)

if args[0] == "--apply":
    print("Applying transformations...")
    for file_name in args[1 : ]:
        if transform_suffix in file_name:
            print(f"Applying {file_name}")
            src_name = file_name.replace(transform_suffix, "")
            os.remove(src_name)
            os.rename(file_name, src_name)
        else:
            print(f"Skipping {file_name}")
            continue
    print("Done!")
    exit(0)



def transform_filename(filename):
    assert type(filename) is str

    p = Path(filename)
    suffix = str(p.suffix)

    assert len(suffix) > 0

    without_suffix = filename.removesuffix(suffix)
    assert without_suffix + suffix == filename

    return without_suffix + transform_suffix + suffix

class File_Thing:
    def __init__(self, file):
        self.file = file
        self.index = 0
        self.line = ""

    def get_next(self):
        if self.index < len(self.line):
            c = self.line[self.index]
            self.index += 1
            return c

        self.line = self.file.readline()
        self.index = 0

        if len(self.line) == 0:
            return None

        c = self.line[0]
        self.index += 1
        return c

    def get_next_non_space(self):
        while True:
            c = self.get_next()
            if c is None:
                return None
            if c.isspace():
                continue
            return c

def diff_ignore_whitespace(filename1, filename2):
    with (
        open(filename1, "r") as file1,
        open(filename2, "r") as file2
    ):

        ft1 = File_Thing(file1)
        ft2 = File_Thing(file2)

        while True:
            c1 = ft1.get_next_non_space()
            c2 = ft2.get_next_non_space()

            if c1 != c2:
                return True

            if c1 is None:
                return False

unsafe_changes = []

for src_filename in args:
    if transform_suffix in src_filename:
        continue

    new_filename = transform_filename(src_filename)

    with open(new_filename, "w") as new_file:
        command = f"clang-format --style=file:clangFormatConfig {src_filename}"
        proc = subprocess.run(command.split(), stdout = new_file)
        assert proc.returncode == 0

    if diff_ignore_whitespace(src_filename, new_filename):
        unsafe_changes.append(new_filename)


if len(unsafe_changes) > 0:
    print(f"{len(unsafe_changes)} files seem to have been meaningfully changed. Check if it's a comment at the end of a namespace")
    print("Relevant files:")
    for f in unsafe_changes:
        print(f)


