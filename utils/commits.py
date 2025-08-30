"""
    Copy me to one directory above the MCGS directory, then add at least 2
    commit hashes to compare
"""

import os
import subprocess
import time
import shutil
import datetime
import select
import pathlib

############################################################ Input/config
commits = [
]

target_dir = "MCGS"
output_dir = "commits_data"


############################################################ Global variables
start_dir = pathlib.Path(os.curdir).absolute()
target_dir = pathlib.Path(target_dir).absolute()

assert pathlib.Path.exists(start_dir)
assert pathlib.Path.exists(target_dir)

############################################################ Functions
def get_lines(proc, stderr=False):
    assert type(proc) is subprocess.Popen

    lines = None
    if stderr:
        lines = proc.stderr.readlines()
    else:
        lines = proc.stdout.readlines()

    lines = [x.decode("utf-8") for x in lines]
    lines = "".join(lines).rstrip()
    return lines


def run_command(cmd, exp_code=None):
    assert type(cmd) is str
    assert exp_code is None or type(exp_code) is int

    proc = subprocess.Popen(cmd.split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    stdout_len = 0
    stdout_chunks = []

    stderr_len = 0
    stderr_chunks = []

    # TODO non-busy loop...
    while proc.poll() is None:
        s1, _, _ = select.select([proc.stdout], [], [])
        if len(s1) > 0:
            chunk = proc.stdout.read().decode("utf-8")
            b1 = len(chunk)
            if b1 > 0:
                stdout_len += b1
                stdout_chunks.append(chunk)

        s2, _, _ = select.select([proc.stderr], [], [])
        if len(s2) > 0:
            chunk = proc.stderr.read().decode("utf-8")
            b2 = len(chunk)
            if b2 > 0:
                stderr_len += b2
                stderr_chunks.append(chunk)

    rc = proc.wait()

    stdout_line = "".join(stdout_chunks).rstrip()
    stderr_line = "".join(stderr_chunks).rstrip()

    if exp_code is not None and rc != exp_code:
        print(f"run_command error (expected {exp_code}, got {rc}):\n{stderr_line}")
        assert False

    return [stdout_line, rc]

def get_current_path():
    p = pathlib.Path(os.curdir).absolute()
    return p

def set_current_path(target):
    p = pathlib.Path(target)
    assert p.exists()

    p = p.absolute()
    os.chdir(p)

    assert pathlib.Path(os.curdir).absolute() == p


def current_time():
    return datetime.datetime.now().strftime("%b %d %I:%M:%S %p")


def print_status_1(msg):
    assert type(msg) is str
    print(f"{msg} ({current_time()})... ", end="", flush=True)


def print_status_2(msg="OK!"):
    assert type(msg) is str
    print(msg)


class Commit:
    def __init__(self, hash):
        assert type(hash) is str

        assert get_current_path() == start_dir
        set_current_path(target_dir)
        message = run_command(f"git log --format=%B -n 1 {hash}")[0]
        assert type(message) is str and len(message) > 0
        set_current_path(start_dir)
        assert get_current_path() == start_dir

        self._hash = hash
        self._message = message

    def hash(self):
        return self._hash

    def message(self):
        return self._message


def assert_is_clean():
    files = run_command("git status --porcelain", 0)[0]
    assert len(files) == 0

    assert not pathlib.Path("MCGS").exists()
    assert not pathlib.Path("MCGS_test").exists()


def assert_target_dir():
    assert get_current_path() == target_dir


def assert_start_dir():
    assert get_current_path() == start_dir


def clean_dir(full):
    assert_target_dir()

    if full:
        run_command("git checkout -f", 0)
        run_command("git clean -f", 0)
        run_command("rm -f database.bin", 0)

    run_command("make clean", 0)
    assert_is_clean()


def build_debug():
    assert_target_dir()
    assert_is_clean()

    print_status_1("Making debug build")
    run_command("make -j 11", 0)
    print_status_2()


def build_test():
    assert_target_dir()
    assert_is_clean()

    print_status_1("Making test build")
    run_command("make -j 11 MCGS_test", 0)
    print_status_2()


def build_prod():
    assert_target_dir()
    assert_is_clean()

    print_status_1("Making production build")
    run_command("make -j 11 DEBUG=0", 0)
    print_status_2()


def gen_db():
    assert_target_dir()
    assert_is_clean()

    build_debug()

    print_status_1("Generating database")
    run_command("./MCGS []", 0)
    print_status_2()


def run_unit_tests():
    assert_target_dir()
    assert_is_clean()

    build_test()

    print_status_1("Running unit tests")
    lines = run_command("./MCGS_test", 0)[0]

    lines_split = lines.split()
    assert len(lines_split) > 0
    assert lines_split[-1].strip() == "SUCCESS"
    print_status_2()


def run_perf_tests():
    assert_target_dir()
    assert_is_clean()

    build_prod()

    command = "./MCGS --run-tests --test-dir input/main_tests --test-timeout 5000 "
    command += "--out-file commits_perf_test.csv"

    time.sleep(3)
    print_status_1("Running perf tests")
    run_command(command, 0)
    print_status_2()


def checkout(hash):
    assert type(hash) is str
    assert_target_dir()

    run_command(f"git checkout -f {hash}", 0)


def save_output(commit_hash):
    assert type(commit_hash) is str
    assert_start_dir()

    print_status_1("Saving output")

    output_file = pathlib.Path(target_dir) / "commits_perf_test.csv"
    assert output_file.exists()
    output_file = output_file.absolute()

    run_command(f"cp {output_file} {output_dir}/{commit_hash}.csv")

    print_status_2()


def init_output_dir():
    assert_start_dir()

    p = pathlib.Path(output_dir)

    if p.exists():
        choice = input(f"{p} exists, delete it? [Y/n]")

        if choice.rstrip() == "Y":
            shutil.rmtree(p)
        else:
            exit(0)

    os.mkdir(p)
    assert p.exists()

############################################################ Core script
set_current_path(target_dir)
assert_is_clean()

set_current_path(start_dir)
init_output_dir()

commits = [Commit(hash) for hash in commits]

n_commits = len(commits)
for i in range(n_commits):
    c = commits[i]

    print("=" * 80)
    print(f"{i + 1} of {n_commits}: {c.hash()} {c.message()}")
    print("=" * 80)

    # Go to target dir
    set_current_path(target_dir)

    # Checkout commit
    checkout(c.hash())

    # Build database
    clean_dir(True)
    gen_db()

    # Run unit tests
    clean_dir(False)
    run_unit_tests()

    # Run perf tests
    clean_dir(False)
    run_perf_tests()

    # Save output
    set_current_path(start_dir)
    save_output(c.hash())

    print("")

set_current_path(start_dir)

outfile = open(pathlib.Path(output_dir) / "compare.sh", "w")

for i in range(1, n_commits):
    c_new = commits[i]
    c_old = commits[i - 1]

    f_new = c_new.hash() + ".csv"
    f_old = c_old.hash() + ".csv"

    outfile.write(f"python3 create-table.py {f_new} --compare-to {f_old} -o {i-1}-to-{i}.html\n")

outfile.close()

