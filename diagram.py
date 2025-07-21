import csv
import matplotlib.pyplot as plt
import numpy as np
import pathlib
import math
import sys

############################################################
args = sys.argv
argc = len(sys.argv)
assert argc == 2

input_dir = pathlib.Path(args[1])
assert input_dir.exists() and input_dir.is_dir()

timed_out = set()

############################################################
class WrappedReader:
    def __init__(self, file_name):
        assert type(file_name) is str or type(file_name) is pathlib.PosixPath

        self._file = open(file_name, "r")
        self._reader = csv.DictReader(self._file)

    def close(self):
        assert self._file is not None
        self._file.close()
        self._file = None
        self._reader = None

    def reader(self):
        assert self._file is not None
        return self._reader


def csv_files(pattern):
    assert type(pattern) is str
    return input_dir.glob(pattern)


"""
i.e.

diagram: 4,
game: "clobber_1xn",
x: 20,

"""
def parse_comment(comment):
    assert type(comment) is str

    expected_fields = ["diagram", "game", "x"]
    field_dict = {ef: None for ef in expected_fields}

    fields_read = [field.strip() for field in comment.split()]

    for f in fields_read:
        pair = f.split("::")
        assert len(pair) == 2

        name, value = pair
        assert name in expected_fields
        field_dict[name] = value

    assert None not in field_dict
    field_dict["x"] = int(field_dict["x"])
    field_dict["diagram"] = int(field_dict["diagram"])

    return field_dict


def row_did_complete(row):
    assert type(row) is dict
    status = row["Status"]

    options = ["TIMEOUT", "COMPLETED"]
    assert status in options
    return options.index(status) == 1


def prune_timeouts(file_list):
    assert type(file_list) is list

    for f in file_list:
        wrapped = WrappedReader(f)
        reader = wrapped.reader()

        for line in reader:
            if not row_did_complete(line):
                input_hash = line["Input hash"]
                assert type(input_hash) is str

                timed_out.add(input_hash)

        wrapped.close()


def row_in_timeout_set(row):
    assert type(row) is dict

    input_hash = row["Input hash"]
    assert type(input_hash) is str

    return input_hash in timed_out


def process_files(file_list, label):
    assert type(file_list) is list
    assert type(label) is str

    data = []
    n_timeouts = 0
    n_cases = 0

    for file_name in file_list:
        wrapped = WrappedReader(file_name)
        reader = wrapped.reader()

        for line in reader:
            n_cases += 1

            #if not row_did_complete(line):
            if row_in_timeout_set(line):
                n_timeouts += 1
                continue

            metadata = parse_comment(line["Comments"])

            x = metadata["x"]
            y = math.log(int(line["Node Count"]))
            #y = int(line["Node Count"])

            data.append((x, y))

        wrapped.close()

    print(f"{n_timeouts} timeouts of {n_cases} cases")

    data_x = [val[0] for val in data]
    data_y = [val[1] for val in data]

    data_x = np.array(data_x)
    data_y = np.array(data_y)

    x_scale = np.unique(data_x)
    means = [np.mean(data_y[data_x == xi]) for xi in x_scale]
    errors = [np.std(data_y[data_x == xi]) for xi in x_scale]

    plt.errorbar(x_scale, means, yerr=errors, fmt="o", label=label)
    plt.title("clobber_1xn")
    plt.legend()
    plt.xlabel("# Moves For Player")
    plt.ylabel("# Nodes (log)")

    plt.show(block=False)


def pattern_to_file_list(pattern):
    assert type(pattern) is str
    return [f for f in input_dir.glob(pattern)]


############################################################
patterns = [
    "*_00.csv",
    "*_10.csv",
    "*_11.csv",
]

for p in patterns:
    file_list = pattern_to_file_list(p)
    prune_timeouts(file_list)

for p in patterns:
    file_list = pattern_to_file_list(p)
    process_files(file_list, p)

x = input("")
