import subprocess
import sys
from pathlib import Path
import os

summary_path = Path("format_result.txt")
transform_suffix = "___transformed"

if summary_path.exists():
    os.remove(summary_path)

args = sys.argv[1 : ]

def print_flag(flag_text, flag_description):
    print("\t" + flag_text)
    print("\t\t" + flag_description)
    print("")

def print_help():

    print(f"Usage: python3 {sys.argv[0]} [flags] [input files]")
    print("\n\tCreates, deletes, or applies format transformations on source files.")

    print(f"""
[input files] is a possibly mixed list of source files and transform files.
Transform files have the transform suffix \"{transform_suffix}\" in their names, i.e.
\tSource file: some_file.cpp
\tTransform file: some_file{transform_suffix}.cpp

Modes:
    Create:
        When [flags] is empty, operates in \"create\" mode, using clang-format
        to generate transform files of given source files.
        Transform files in the input list are ignored.

    Delete:
        For each given source file, delete its corresponding transform file.
        For each given transform file, delete it.

    Replace:
        For each given source file, find its transform file. If the transform
        file exists, replace the source file with it.
        For each existing given transform file, replace its source file with it.
""")

    print("\nFlags:")
    print_flag("--delete", "Operate in \"delete\" mode")
    print_flag("--replace", "Operate in \"replace\" mode")

if "-h" in args or "--help" in args:
    print_help()
    exit(0)

if len(args) < 1:
    print(f"{sys.argv[0]} too few arguments")
    print_help()
    exit(-1)


def remove_if_exists(filename, print_message):
    p = Path(filename)
    if p.exists():
        if print_message:
            print(f"Deleting {filename}")
        os.remove(p)
        return True
    return False

def transform_filename(filename):
    assert type(filename) is str

    p = Path(filename)
    suffix = str(p.suffix)

    assert len(suffix) > 0

    without_suffix = filename.removesuffix(suffix)
    assert without_suffix + suffix == filename

    return without_suffix + transform_suffix + suffix


def replace_with_transform(src_filename, transformed_filename):
    p2 = Path(transformed_filename)

    if p2.exists():
        print(f"Applying {transformed_filename}")
        remove_if_exists(src_filename, False)
        os.rename(transformed_filename, src_filename)


if args[0] == "--delete":
    print("Deleting transformations...")
    for filename in args[1 : ]:
        if transform_suffix in filename:
            remove_if_exists(filename, True)
        else:
            transformed_filename = transform_filename(filename)
            remove_if_exists(transformed_filename, True)

    print("Done!")
    exit(0)

if args[0] == "--replace":
    print("Applying transformations...")
    for filename in args[1 : ]:
        if transform_suffix in filename:
            src_filename = filename.replace(transform_suffix, "")
            replace_with_transform(src_filename, filename)
        else:
            transformed_filename = transform_filename(filename)
            replace_with_transform(filename, transformed_filename)

    print("Done!")
    exit(0)

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


