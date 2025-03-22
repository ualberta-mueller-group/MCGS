# Used by "format" targets in makefile
import subprocess
import sys
from pathlib import Path
import os

summary_path = Path("format_result.txt")
transform_suffix = "___transformed"
config_name = ".clang-format"



args = sys.argv[1 : ]


def print_flag(flag_text, flag_description):
    print("\t" + flag_text)
    print("\t\t" + flag_description)
    print("")


def print_help():
    print(f"Usage: python3 {sys.argv[0]} [flags] [input files]")
    print("\n\tCreates, deletes, or applies format transformations on source files.")
    print("\tThis file is meant to be used by makefile targets instead of being used directly.")

    print(f"""
[input files] is a possibly mixed list of source files and transform files.
Transform files have the transform suffix \"{transform_suffix}\" in their names, i.e.
\tSource file: some_file.cpp
\tTransform file: some_file{transform_suffix}.cpp

Source and transform files can be used interchangeably in the input list, producing
the same effect. This tool will convert between the two notations and deduplicate
the input list. i.e.:
\tpython3 {sys.argv[0]} some_file.cpp
and
\tpython3 {sys.argv[0]} some_file{transform_suffix}.cpp
are equivalent.


Modes:
    Create:
        When [flags] is empty, use clang-format to create transform files.
        Prints a diff of all files to {summary_path}. Also prints to stdout, a
        list of created transform files who differ from their source file by
        more than just whitespace.

    Delete:
        For each file in the input list, delete the corresponding transform file.

    Replace:
        For each file in the input list, replace the corresponding source file
        with its existing transform file.
""")

    print("Flags:")
    print_flag("--delete", "Operate in \"delete\" mode")
    print_flag("--replace", "Operate in \"replace\" mode")
    print_flag("--help, -h", "Print this message")


if "-h" in args or "--help" in args:
    print_help()
    exit(0)

if len(args) < 1:
    print("Too few arguments. Try: \'LINT_FILES=\"some_file.cpp\" make format\'")
    exit(-1)

if summary_path.exists():
    os.remove(summary_path)


assert Path(config_name).exists()


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
    assert transform_suffix not in filename

    p = Path(filename)
    suffix = str(p.suffix)

    assert len(suffix) > 0

    without_suffix = filename.removesuffix(suffix)
    assert without_suffix + suffix == filename

    return without_suffix + transform_suffix + suffix


def replace_with_transform_if_exists(src_filename, trn_filename):
    p2 = Path(trn_filename)

    if p2.exists():
        print(f"Replacing {src_filename} with {trn_filename}")
        remove_if_exists(src_filename, False)
        os.rename(trn_filename, src_filename)


def get_src_trn_filename_pair(filename):
    if transform_suffix in filename:
        return [filename.replace(transform_suffix, ""), filename]
    else:
        return [filename, transform_filename(filename)]


if args[0] == "--delete":
    print("Deleting transformations:")

    seen_files = set()

    for filename in args[1 : ]:
        src_name, trn_name = get_src_trn_filename_pair(filename)

        if trn_name not in seen_files:
            seen_files.add(trn_name)
            remove_if_exists(trn_name, True)

    print("Done!")
    exit(0)


if args[0] == "--replace":
    print("Applying transformations:")

    seen_files = set()

    for filename in args[1 : ]:
        src_name, trn_name = get_src_trn_filename_pair(filename)

        if trn_name not in seen_files:
            seen_files.add(trn_name)
            replace_with_transform_if_exists(src_name, trn_name)

    print("Done!")
    exit(0)


# For reading a file one character at a time
class File_Thing:

    # Takes a file created with open()
    def __init__(self, file):
        self.file = file
        self.index = 0
        self.line = ""

    # Return next character, or None if EOF
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

    # Return next non-whitespace character, or None if EOF
    def get_next_non_space(self):
        while True:
            c = self.get_next()
            if c is None:
                return None
            if c.isspace():
                continue
            return c


# True iff files differ after deleting all whitespace
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

# True iff files differ
def diff(filename1, filename2):
    with (
        open(filename1, "r") as file1,
        open(filename2, "r") as file2
    ):

        ft1 = File_Thing(file1)
        ft2 = File_Thing(file2)

        while True:
            c1 = ft1.get_next()
            c2 = ft2.get_next()

            if c1 != c2:
                return True

            if c1 is None:
                return False




# List of files differing by more than just whitespace
unsafe_changes = []

seen_files = set()


for filename in args:
    src_name, trn_name = get_src_trn_filename_pair(filename)

    if trn_name in seen_files:
        continue

    seen_files.add(trn_name)

    if not Path(src_name).exists():
        print(f"SKIPPING {src_name}, file doesn't exist")
        continue

    print(f"Creating transform: {trn_name}")

    with open(trn_name, "w") as trn_file:
        command = f"clang-format --style=file:{config_name} {src_name}"
        proc = subprocess.run(command.split(), stdout = trn_file)
        assert proc.returncode == 0

    if not diff(src_name, trn_name):
        print("\tNo changes, deleting transform")
        remove_if_exists(trn_name, False)
        continue

    with open(summary_path, "a") as diff_file:
        diff_file.write(f"{src_name} --> {trn_name}\n")
        diff_file.flush()

        command = f"diff {src_name} {trn_name}"
        proc = subprocess.run(command.split(), stdout = diff_file)
        assert proc.returncode in [0, 1]


    if diff_ignore_whitespace(src_name, trn_name):
        unsafe_changes.append(trn_name)



print("")
print(f"{len(unsafe_changes)} file(s) differ by more than just whitespace")

if len(unsafe_changes) > 0:
    print("Check if the differences are from comments at the ends of namespace braces, \
or are a result of exceeding the line column limit")
    print("Relevant files:")
    for f in unsafe_changes:
        print(f)
