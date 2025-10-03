"""
Diagram maker for MCGS paper experiments.

The "groups" and "labels" variables at the bottom can be edited.

IMPORTANT: an item's position in the labels list must correspond to the
"diagram" variable in the test case metadata (encoded in the comments of the
.test2 file and resulting csvs after running).
"""

import csv
import random
import matplotlib.pyplot as plt
import numpy as np
import pathlib
import math
from collections import Counter
import sys
import seaborn as sns

pal = sns.color_palette("colorblind")

plt.rcParams.update({"font.size": 14})
plt.close()

DATA_OFFSET = None

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

    expected_fields = set(["pattern", "color", "label", "marker"])
    actual_fields = set([key for key in group])
    assert expected_fields == actual_fields

    for v in group.values():
        assert type(v) is str or (type(v) is tuple and len(v) == 3)


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
        if i == 0:
            max_key = max([k for k in counts.keys()], key=lambda x: counts[x])
            print(f"{title} Max timeout bin: {max_key}, timeouts: {counts[max_key]}")
        counts = [counts[x] for x in bins]


        #plt.hist(ds[0], label=label, color=color, bins=bins)
        plt.bar(bins, counts, label=label, color=color)
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


def process_files(group, diagram_id, label_set, offset):
    assert_group_format(group)
    assert type(diagram_id) is int
    assert type(label_set) is list and len(label_set) == 2

    global DATA_OFFSET

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

    plt.xticks(x_scale)

    fig = plt.gcf()
    fig.canvas.draw()

    if DATA_OFFSET is None:
        ax = plt.gca()
        tf = ax.transData
        tf_inv = tf.inverted()

        disp = tf_inv.transform([(8, 0), (0, 0)])
        DATA_OFFSET = disp[0][0] - disp[1][0]

    plt.errorbar(x_scale + offset * DATA_OFFSET, means, yerr=errors, fmt="o", label=group["label"],
                 color=group["color"], capsize=3.4, capthick=2, elinewidth=2)

    #plt.plot(x_scale + offset, means, color=group["color"], marker="o", linestyle="")
    #plt.plot(x_scale + offset, np.add(means, errors), color=group["color"], marker=10, linestyle="")
    #plt.plot(x_scale + offset, np.subtract(means, errors), color=group["color"], marker=11, linestyle="")

    #assert len(x_scale) == len(means) and len(means) == len(errors)
    #for i in range(len(means)):
    #    x_pos = x_scale[i] + offset
    #    y_center = means[i]
    #    y_diff = errors[i]
    #    plt.plot([x_pos, x_pos], [y_center - y_diff, y_center + y_diff], color=group["color"], linestyle=":")


    plt.title(title)
    plt.legend(framealpha=0.5)
    plt.xlabel(x_axis_name)
    plt.ylabel("# Nodes (log)")
    #plt.xticks(x_scale)

    if show_figures:
        plt.show(block=False)




############################################################
groups = [
    {
        "pattern": "*_0.csv",
        "color": (0, 0, 0),
        "label": "No TT, no SN, no DB",
        "marker": "$0-0$"
    },

    {
        "pattern": "*_1.csv",
        #"color": pal[0],
        "color": "#015887",
        "label": "TT, no SN, no DB",
        "marker": "$1-1$"
    },

    {
        "pattern": "*_2.csv",
        "color": pal[3],
        "label": "TT, SN, no DB",
        "marker": "$2-2$"
    },

    {
        "pattern": "*_3.csv",
        #"color": pal[4],
        "color": "#bf8ad6",
        #"color": (0.8, 0.471, 0.737),
        "label": "TT, SN, DB",
        "marker": "$3-3$"
    },
]

for g in groups:
    assert_group_format(g)

labels = [
    ["clobber_1xn", "# Moves For Player"],
    ["nogo_1xn", "# Moves For Player"],
    ["elephants", "Total Stones"],
    ["2xn clobber", "# Columns"],
    ["clobber_1xn subgames", "# of Subgames"],
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
    fig = plt.gcf()
    fig.subplots_adjust(
        bottom = 0.11,
        top = 0.92,

        left = 0.085,
        right = 0.97,
    )
    fig.set_size_inches(10, 5)

    ax = plt.gca()
    ax.margins(x = 0.01)

    DATA_OFFSET = None
    for j in range(len(groups)):
        g = groups[j]
        process_files(g, i, label_set, [-1.5, -0.5, 0.5, 1.5][j])

    if output_dir is not None:
        #plt.tight_layout()
        
        fig.savefig(f"{output_dir.absolute() / title}.png")
        #plt.savefig(f"{output_dir.absolute() / title}.png")
    else:
        _ = input("")

    plt.close()

