import subprocess
import os
import select
import shutil
import pathlib

############################################################
tt_idx_bits = 10
out_dir = "experiment_results"
n_solvers = 5
test_file = "tests.test2"

############################################################
solvers = []
outfiles = []

READY_TEXT = "READY FOR TEST CASE"


############################################################
def file_exists(file_path):
    return pathlib.Path(file_path).exists()


def remove_file(file_path):
    shutil.rmtree(file_path)


def assert_is_proc(proc):
    assert type(proc) is subprocess.Popen


class SolverThread:
    def __init__(self, proc, stdout_name):
        assert_is_proc(proc)
        assert type(stdout_name) is str

        self._proc = proc
        self._stdout_name = stdout_name

        assert not pathlib.Path(self._stdout_name).exists()
        self._stdout_file = open(self._stdout_name, "w")

    def finalize(self):
        proc = self._proc
        assert proc is not None

        proc.stdin.close()
        rc = proc.wait()
        assert rc == 0

        self._proc = None
        self._stdout_file.close()

    def solve(self, test_string):
        assert type(test_string) is str
        proc = self._proc
        assert proc is not None

        test_string = (test_string.rstrip() + "\n").encode("utf-8")
        proc.stdin.write(test_string)
        proc.stdin.flush()

    def get_stdout(self):
        assert self._proc is not None
        return [self._proc.stdout, self._stdout_file]


def spawn_solver(thread_number):
    assert type(thread_number) is int
    global outfiles

    csv_name = f"{out_dir}/{thread_number}.csv"
    stdout_name = f"{out_dir}/{thread_number}_stdout.txt"
    stderr_name = f"{out_dir}/{thread_number}_stderr.txt"

    args = [
        "--run-tests-stdin",
        "--clear-tt",
        "--test-timeout 10000",
        f"--out-file {csv_name}",
        f"--tt-sumgame-idx-bits {tt_idx_bits}",
    ]

    args = " ".join(args)

    cmd = "./MCGS " + args

    print(cmd)
    stderr_file = open(stderr_name, "w")

    outfiles += [stderr_file]

    proc = subprocess.Popen(cmd.split(), stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE, stderr=stderr_file)

    proc.stdout._mcgs_id = thread_number

    assert proc.poll() is None
    return SolverThread(proc, stdout_name)


def close_files():
    global outfiles

    for f in outfiles:
        f.close()


def close_solvers():
    global solvers

    for s in solvers:
        assert type(s) is SolverThread
        s.finalize()


def init_out_dir():
    global out_dir

    p = pathlib.Path(out_dir).absolute()

    if file_exists(p):
        print(f"{p} already exists. Deleting...")
        remove_file(p)

    assert not file_exists(p)
    os.mkdir(p)
    assert file_exists(p)


def run_all_tests():
    global solvers
    assert len(solvers) == n_solvers

    stdout_stream_list = [s.get_stdout()[0] for s in solvers]
    stdout_file_list = [s.get_stdout()[1] for s in solvers]
    buffers = [bytes() for s in solvers]

    ready_solver_ids = []

    tests = open(test_file, "r")

    for t in tests:
        while True:
            # Use a solver that's ready
            if len(ready_solver_ids) > 0:
                sid = ready_solver_ids.pop()
                s = solvers[sid]
                s.solve(t)
                print(f"Using solver {sid}")
                break

            # Wait for output from one of the solvers
            ready, _, _ = select.select(stdout_stream_list, [], [])

            for stream in ready:
                sid = stream._mcgs_id
                assert type(sid) is int

                # Don't use any other read functions -- they buffer data which
                # select doesn't see, causing a deadlock...
                char = os.read(stream.fileno(), 1)

                if char != b'\n':
                    buffers[sid] += char
                else:
                    buf = buffers[sid]
                    buffers[sid] = bytes()
                    line = buf.decode("utf-8").strip()

                    if (line == READY_TEXT):
                        assert sid not in ready_solver_ids
                        ready_solver_ids.append(sid)
                    else:
                        f = stdout_file_list[sid]
                        f.write(line)
                        f.write("\n")
                        f.flush()

    tests.close()


############################################################ Main script
# Sanity checks
assert file_exists(test_file)

# Initialize output directory
init_out_dir()

# Spawn solvers
assert len(solvers) == 0

for i in range(n_solvers):
    solvers.append(spawn_solver(i))

# Read input file, solve tests
run_all_tests()

# Close everything
close_solvers()
close_files()


