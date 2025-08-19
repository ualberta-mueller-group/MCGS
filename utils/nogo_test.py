import sys
import subprocess
import pathlib
import os
import re

################################################################################
SBHSOLVER_DIR = pathlib.Path("../SBHSolver")
CONFIG_PATH = SBHSOLVER_DIR / "configs.hpp"
SOLVER_PATH = SBHSOLVER_DIR / "solver_main"

solver_proc = None

HELP_MESSAGE = f"""
Usage: python3 {sys.argv[0]} <input file>

Input file format example:
(3 3) nogo:XO.|...|... B:1 W:0
"""

################################################################################

def print_help_message():
    print(HELP_MESSAGE)
    exit(0)

def file_exists(filename):
    return pathlib.Path(filename).exists()


def run_command(cmd, expected_rc=0):
    assert type(cmd) is str or (type(cmd) is list and len(cmd) > 0)
    assert expected_rc is None or type(expected_rc) is int

    if type(cmd) is list:
        proc = subprocess.Popen(cmd)
    else:
        proc = subprocess.Popen(cmd.split())
    rc = proc.wait()

    if expected_rc is not None:
        assert rc == expected_rc


def compile_solver(r, c):
    assert type(r) is int and r >= 1
    assert type(c) is int and c >= 1
    print(f"Recompiling solver for {r}x{c}")

    run_command(f"make -C {SBHSOLVER_DIR} clean")
    assert not file_exists(SOLVER_PATH)

    assert file_exists(CONFIG_PATH)
    sed_command_r = [
        "sed",
        "-i",
        f"s/^const int N_ROWS *= *[0-9]\\+;$/const int N_ROWS = {r};/g",
        str(CONFIG_PATH),
    ]

    sed_command_c = [
        "sed",
        "-i",
        f"s/^const int N_COLS *= *[0-9]\\+;$/const int N_COLS = {c};/g",
        str(CONFIG_PATH),
    ]

    print(sed_command_r)
    print(sed_command_c)

    run_command(sed_command_c)
    run_command(sed_command_r)

    run_command(f"make -C {SBHSOLVER_DIR} -j 6")
    assert file_exists(SOLVER_PATH)
    print("Done")


class Solver:
    def __init__(self, r, c):
        assert type(r) is int and r >= 1
        assert type(c) is int and c >= 1
        self._r = r
        self._c = c

        compile_solver(r, c)
        self._proc = subprocess.Popen(SOLVER_PATH, stdin=subprocess.PIPE,
                                      stdout=subprocess.PIPE)

    def close(self):
        assert self._proc is not None
        self._proc.stdin.close()
        self._proc.wait()

    def get_dims(self):
        return (self._r, self._c)

    def get_stdin(self):
        assert self._proc is not None
        return self._proc.stdin

    def get_stdout(self):
        assert self._proc is not None
        return self._proc.stdout

    def get_proc(self):
        assert self._proc is not None
        return self._proc


def is_mcgs_board(board):
    assert type(board) is str
    for c in board:
        if c not in ["X", "O", ".", "|"]:
            return False
    return True


def solve_board(r, c, board):
    global solver_proc
    assert type(r) is int and r >= 1
    assert type(c) is int and c >= 1
    assert type(board) is str

    if solver_proc is None or (r, c) != solver_proc.get_dims():
        if solver_proc is not None:
            solver_proc.close()
            solver_proc = None
        solver_proc = Solver(r, c)

    stdin = solver_proc.get_stdin()
    stdout = solver_proc.get_stdout()

    assert is_mcgs_board(board)
    stdin.write(bytes(board + "\n", "utf-8"))
    stdin.flush()

    # Buffered read OK, we're not using select.select
    line = stdout.readline().decode("utf-8").strip()
    assert solver_proc.get_proc().poll() is None

    results = line.split()
    assert len(results) == 2
    return tuple([int(res[2]) for res in results])


def parse_input_line(line):
    assert type(line) is str

    chunks = line.split()
    assert len(chunks) == 5

    fields = {
    }

    r = int(chunks[0][1 :])
    c = int(chunks[1][0 : -1])
    board = chunks[2][len("nogo:") : ]

    assert len(chunks[3]) == 3 and len(chunks[4]) == 3
    b_win = int(chunks[3][2])
    w_win = int(chunks[4][2])

    fields["r"] = r
    fields["c"] = c
    fields["board"] = board
    fields["b_win"] = b_win
    fields["w_win"] = w_win

    return fields


################################################################################
if len(sys.argv) != 2:
    print_help_message()

filename = sys.argv[1]
if not file_exists(filename):
    print_help_message()

infile = open(filename, "r")
for line in infile:
    fields = parse_input_line(line)
    print(fields)

    r = fields["r"]
    c = fields["c"]
    board = fields["board"]
    expected = (fields["b_win"], fields["w_win"])

    result = solve_board(r, c, board)

    if result != expected:
        print(f"Mismatch for {board}")
        print(f"Expected {expected}, got {result}")
        assert False


infile.close()
