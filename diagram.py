import csv
import random
import matplotlib.pyplot as plt
import numpy as np
import pathlib
import math
from collections import Counter
import sys


############################################################
args = sys.argv
argc = len(sys.argv)
assert argc >= 2

input_dir = pathlib.Path(args[1])
assert input_dir.exists() and input_dir.is_dir()

show_figures = True

timed_out_set = set()

output_dir = None

node_rate = True


def find(l, val):
    assert type(l) is list

    for i in range(len(l)):
        if l[i] == val:
            return i
    return -1

save_arg_idx = find(args, "--save")
if save_arg_idx != -1:
    output_path_idx = save_arg_idx + 1
    assert output_path_idx < argc

    output_dir = pathlib.Path(args[output_path_idx])
    assert not output_dir.exists()

    output_dir.mkdir()
    assert output_dir.exists()

    show_figures = False


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


def assert_group_format(group):
    assert type(group) is dict

    expected_fields = set(["pattern", "color", "label"])
    actual_fields = set([key for key in group])
    assert expected_fields == actual_fields

    for v in group.values():
        assert type(v) is str


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


def process_timeouts(groups, diagram_id, label_set):
    # Asserts
    assert type(groups) is list
    for g in groups:
        assert_group_format(g)

    assert type(diagram_id) is int

    assert type(label_set) is list and len(label_set) == 2

    # Find timeouts
    global timed_out_set

    title = label_set[0] + " timeouts"

    timed_out_set.clear()

    data_sets = []

    for g in groups:
        data = []

        file_list = pattern_to_file_list(g["pattern"])
        for f in file_list:
            wrapped = WrappedReader(f)
            reader = wrapped.reader()

            for line in reader:
                meta = parse_comment(line["Comments"])
                if meta["diagram"] != diagram_id:
                    continue

                x = meta["x"]
                timed_out = not row_did_complete(line)

                if timed_out:
                    data.append(x)
                    timed_out_set.add(line["Input hash"])

            wrapped.close()

        min_x = min(data) if len(data) != 0 else 0
        max_x = max(data) if len(data) != 0 else 1
        data_sets.append([data, min_x, max_x])

        #label = g["label"]
        #color = g["color"]

        #plt.hist(data, label=label, color=color, bins=(max_x - min_x + 1))
        #plt.title(title)
        #plt.legend()
    min_x_global = min([x[1] for x in data_sets])
    max_x_global = max([x[2] for x in data_sets])
    bins = [x for x in range(min_x_global - 1, max_x_global + 2)]
    for i in range(len(groups)):
        g = groups[i]
        ds = data_sets[i]

        label = g["label"]
        color = g["color"]

        counts = Counter(ds[0])
        counts = [counts[x] for x in bins]
        #plt.hist(ds[0], label=label, color=color, bins=bins)
        plt.bar(bins, counts)
        plt.xticks(bins)
        plt.title(title)
        plt.legend()


def row_in_timeout_set(row):
    assert type(row) is dict

    input_hash = row["Input hash"]
    assert type(input_hash) is str

    return input_hash in timed_out_set


def pattern_to_file_list(pattern):
    assert type(pattern) is str
    return [f for f in input_dir.glob(pattern)]


def process_files(group, diagram_id, label_set):
    assert_group_format(group)
    assert type(diagram_id) is int
    assert type(label_set) is list and len(label_set) == 2

    title, x_axis_name = label_set
    file_list = pattern_to_file_list(group["pattern"])

    data = []
    n_timeouts = 0
    n_cases = 0

    total_time = 0
    total_nodes = 0

    for file_name in file_list:
        wrapped = WrappedReader(file_name)
        reader = wrapped.reader()

        for line in reader:
            metadata = parse_comment(line["Comments"])
            if metadata["diagram"] != diagram_id:
                continue

            n_cases += 1

            #if not row_did_complete(line):
            if row_in_timeout_set(line):
                n_timeouts += 1
                continue

            y_raw = int(line["Node Count"])
            time_ms = float(line["Time (ms)"])

            total_time += time_ms
            total_nodes += y_raw

            y = math.log(y_raw)
            x = metadata["x"]
            #y = int(line["Node Count"])

            data.append((x, y))

        wrapped.close()

    print(f"{n_timeouts} timeouts of {n_cases} cases")

    if node_rate:
        total_time /= 1000
        print(f"{title} {group["label"]}: {total_nodes / total_time} nodes per "
              "second")

    data_x = [val[0] for val in data]
    data_y = [val[1] for val in data]

    data_x = np.array(data_x)
    data_y = np.array(data_y)

    x_scale = np.unique(data_x)
    means = [np.mean(data_y[data_x == xi]) for xi in x_scale]
    errors = [np.std(data_y[data_x == xi]) for xi in x_scale]

    plt.errorbar(x_scale, means, yerr=errors, fmt="o", label=group["label"],
                 color=group["color"], capsize=3)

    plt.title(title)
    plt.legend()
    plt.xlabel(x_axis_name)
    plt.ylabel("# Nodes (log)")
    plt.xticks(x_scale)

    if show_figures:
        plt.show(block=False)




############################################################
groups = [
    {
        "pattern": "*_0.csv",
        "color": "tab:red",
        "label": "No TT, no SN, no DB",
    },

    {
        "pattern": "*_1.csv",
        "color": "tab:orange",
        "label": "TT, no SN, no DB",
    },

    {
        "pattern": "*_2.csv",
        "color": "tab:blue",
        "label": "TT, SN, no DB",
    },

    {
        "pattern": "*_3.csv",
        "color": "tab:green",
        "label": "TT, SN, DB",
    },
]

for g in groups:
    assert_group_format(g)

labels = [
    ["clobber_1xn", "# Moves For Player"],
    ["nogo_1xn", "# Moves For Player"],
    ["elephants", "Total Stones"],
    ["2xn clobber", "# Columns"],
    ["clobber_1xn with holes", "# of Subgames"],
]


for i in range(len(labels)):
    label_set = labels[i]
    title = label_set[0]

    plt.close()
    process_timeouts(groups, i, label_set)
    if output_dir is not None:
        plt.savefig(f"{output_dir.absolute() / title}_timeouts.png")
    else:
        _ = input("")
    plt.close()

    for g in groups:
        process_files(g, i, label_set)

    if output_dir is not None:
        plt.savefig(f"{output_dir.absolute() / title}.png")
    else:
        _ = input("")

    plt.close()

