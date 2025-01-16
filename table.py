import csv
import sys
import pathlib
import datetime
import hashlib


######################################## Constants
time_threshold_frac = 0.1
time_threshold_abs = 5

######################################## Parse args


def print_usage():
    print(f"Usage: python3 {sys.argv[0]} <input file 1> [<input file 2>] -o <output HTML file>")


if len(sys.argv) < 2:
    print_usage()
    exit(-1)

infile_names = []
outfile_name = ""


# Parsing loop
skip_next = False
for i in range(1, len(sys.argv)):
    # This arg was already consumed
    if skip_next:
        skip_next = False
        continue

    arg = sys.argv[i]
    arg_next = sys.argv[i + 1] if i + 1 < len(sys.argv) else None

    if arg == "-o":
        skip_next = True
        second_last_arg = i == len(sys.argv) - 2

        if arg_next is None or not second_last_arg:
            print_usage()
            exit(-1)

        outfile_name = arg_next
        continue

    if arg in ["-h", "--help"]:
        print_usage()
        exit(0)

    infile_names.append(arg)

if len(outfile_name) == 0 or not (len(infile_names) > 0 and len(infile_names) <= 2):
    print_usage()
    exit(-1)

######################################## row functions


def row_populate_single_mode(input_rows, output_row):
    input_row = input_rows[0]

    for alias in output_field_dict:
        output_row[alias] = new_default_cell(input_row[alias])


def row_style(_input_rows, output_row):
    if "games" in output_row:
        output_row["games"]["css_classes"].append("break-anywhere")

    if "outcome" in output_row:
        outcome = output_row["outcome"]
        if outcome["text"] == "FAIL":
            outcome["css_classes"].append("cell-fail")
            output_row["css_classes"].append("row-fail")
        elif outcome["text"] == "TIMEOUT":
            outcome["css_classes"].append("cell-timeout")
            output_row["css_classes"].append("row-timeout")
        elif outcome["text"] == "COMPLETED":
            outcome["css_classes"].append("cell-completed")
        elif outcome["text"] == "PASS":
            outcome["css_classes"].append("cell-pass")



def row_populate_double_mode(input_rows, output_row):
    input_row = input_rows[0]
    comparison_row = input_rows[1] if len(input_rows) >= 2 else None

    # Populate simple fields
    simple_fields = ["file", "case", "games", "player", "expected", "got", "time", "outcome", "comments"]
    for alias in simple_fields:
        output_row[alias] = new_default_cell(input_row[alias])

    #oldoutcome
    oldoutcome_text = comparison_row["outcome"] if (comparison_row is not None) else "N/A"
    output_row["oldoutcome"] = new_default_cell(oldoutcome_text)

    # Populate comparison fields
    # faster
    faster_by_string = "N/A"
    faster_css_class = None
    if comparison_row is not None:
        if input_row["outcome"] == "TIMEOUT" or comparison_row["outcome"] == "TIMEOUT":
            faster_by_string = "???"
        else:
            num1 = float(input_row["time"])
            num2 = float(comparison_row["time"])
            faster_by = num2 - num1
            faster_by_string = "{:.2f}".format(faster_by)
            faster_css_class = get_faster_css(num1, num2)
    output_row["faster"] = new_default_cell(faster_by_string)
    if faster_css_class is not None:
        output_row["faster"]["css_classes"].append(faster_css_class)

    # regression
    regression = "N/A"
    regression_css = None
    if comparison_row is not None:
        outcome1 = input_row["outcome"]
        outcome2 = comparison_row["outcome"]
        regression, regression_css = get_regression(outcome1, outcome2)
    output_row["regression"] = new_default_cell(regression)
    if regression_css is not None:
        output_row["regression"]["css_classes"].append(regression_css)


def get_faster_css(new, old):
    percent_diff = abs(new - old) / ((new + old) / 2)
    diff = new - old
    if percent_diff >= time_threshold_frac and abs(diff) >= time_threshold_abs:
        return "cell-slower" if diff > 0 else "cell-faster"
    return None

def get_regression(oc1, oc2):
    # FAIL, PASS, TIMEOUT, COMPLETED
    if oc1 == oc2:
        css = None
        if oc1 == "FAIL":
            css = "cell-fail"
        if oc1 == "TIMEOUT":
            css = "cell-timeout"
        if oc1 == "PASS":
            css = "cell-pass"
        if oc1 == "COMPLETED":
            css = "cell-completed"
        return f"STILL {oc1}", css

    if oc1 != oc2:
        css = None
        if oc1 == "FAIL":
            css = "cell-new-fail"
        if oc1 == "PASS":
            css = "cell-new-pass"
        if oc1 == "COMPLETED":
            css = "cell-new-completed"
        if oc1 == "TIMEOUT":
            css = "cell-new-timeout"
        return f"NEW {oc1}", css


######################################## Define input/output formats

input_field_list = []
input_field_dict = {

}

output_field_list = []
output_field_dict = {
}

row_functions = []

row_match_key = None

comparison_rows = {
}


def add_input_col(alias, column_name):
    input_field_list.append(column_name)
    input_field_dict[alias] = column_name


def add_output_col(alias, column_name):
    output_field_list.append(column_name)
    output_field_dict[alias] = column_name


def add_row_function(fn):
    row_functions.append(fn)


def new_default_cell(text):
    return {"css_classes": ["cell"], "text": text}


add_input_col("file", "File")
add_input_col("case", "Case")
add_input_col("games", "Games")
add_input_col("player", "Player")
add_input_col("expected", "Expected")
add_input_col("got", "Got")
add_input_col("time", "Time (ms)")
add_input_col("outcome", "Outcome")
add_input_col("comments", "Comments")
add_input_col("hash", "Input hash")

if len(infile_names) == 1:
    add_output_col("file", "File")
    add_output_col("case", "Case")
    add_output_col("games", "Games")
    add_output_col("player", "Player")
    add_output_col("expected", "Expected")
    add_output_col("got", "Got")
    add_output_col("time", "Time (ms)")
    add_output_col("outcome", "Outcome")
    add_output_col("comments", "Comments")

    add_row_function(row_populate_single_mode)
    add_row_function(row_style)
else:
    assert len(infile_names) == 2
    add_output_col("file", "File")
    add_output_col("case", "Case")
    add_output_col("games", "Games")
    add_output_col("player", "Player")
    add_output_col("expected", "Expected")
    add_output_col("got", "Got")
    add_output_col("time", "Time (ms)")
    add_output_col("faster", "Faster by") #
    add_output_col("outcome", "Outcome")
    add_output_col("regression", "Regression") #
    add_output_col("oldoutcome", "Old Outcome")
    add_output_col("comments", "Comments")

    add_row_function(row_populate_double_mode)
    add_row_function(row_style)
    row_match_key = ["games", "player", "expected"]


######################################## process rows

if row_match_key is not None:
    for alias in row_match_key:
        assert alias in input_field_dict

def assert_correct_reader_fields(reader):
    assert len(input_field_list) == len(reader.fieldnames)
    for i in range(len(input_field_list)):
        assert input_field_list[i] == reader.fieldnames[i]


def reader_row_to_input_row(reader_row):
    input_row = {}
    for alias in input_field_dict:
        input_row[alias] = reader_row[input_field_dict[alias]]

    return input_row

def input_row_get_key(input_row):
    assert row_match_key is not None

    key = []
    for alias in row_match_key:
        key.append(input_row[alias])

    return tuple(key)

if len(infile_names) >= 2:
    comparison_file_name = infile_names[1]
    comparison_file = open(comparison_file_name, "r")
    reader = csv.DictReader(comparison_file)
    assert_correct_reader_fields(reader)

    for reader_row in reader:
        input_row = reader_row_to_input_row(reader_row)
        row_key = input_row_get_key(input_row)
        comparison_rows[row_key] = input_row

    comparison_file.close()


infile = open(infile_names[0], "r")
reader = csv.DictReader(infile)
assert_correct_reader_fields(reader)



table = "<table id=\"data-table\">\n"

table += "<tr class=\"row-header\">\n"
for field in output_field_list:
    table += f"<th>{field}</th>\n"
table += "</tr>\n"

for reader_row in reader:
    input_row = reader_row_to_input_row(reader_row)

    comparison_row = None
    if len(infile_names) >= 2:
        input_row_key = input_row_get_key(input_row)
        comparison_row = comparison_rows.get(input_row_key)

    input_rows = [input_row]
    if comparison_row is not None:
        input_rows.append(comparison_row)

    output_row = {}
    output_row["css_classes"] = ["row"]

    for fn in row_functions:
        fn(input_rows, output_row)

    assert len(output_row) == len(output_field_dict) + 1
    for alias in output_row:
        if alias == "css_classes":
            continue
        assert alias in output_field_dict

    row_class_string = " ".join(output_row["css_classes"])
    row_text = f"<tr class=\"{row_class_string}\">\n"
    for alias in output_field_dict:
        if alias == "css_classes":
            continue
        cell = output_row[alias]
        cell_class_string = " ".join(cell["css_classes"])
        row_text += f"<td class=\"{cell_class_string}\"><div>"
        row_text += cell["text"]
        row_text += "</div></td>\n"
    row_text += "</tr>\n"

    table += row_text

table += "</table>\n"
infile.close()

######################################## Write HTML file

def get_metadata_string():
    file_names = [x for x in infile_names]
    dates = []
    hashes = []

    date_format = "%Y-%m-%d %H:%M:%S"
    for fname in file_names:
        path = pathlib.Path(fname)
        last_modified = path.stat().st_mtime
        date = datetime.datetime.fromtimestamp(int(last_modified))
        date_string = date.strftime(date_format)
        dates.append(date_string)

        f = open(fname, "rb")
        hash = hashlib.file_digest(f, hashlib.md5)
        f.close()
        hashes.append(hash.hexdigest())

    assert len(file_names) == len(dates)

    result = "<p id=\"metadata-string\">"
    for i in range(len(file_names)):
        fname = file_names[i]
        date = dates[i]
        hash = hashes[i]
        result += f"{fname} ({date} MD5:{hash})"

        if i + 1 < len(file_names):
            result += "\n"

    now = datetime.datetime.now().strftime(date_format)
    result += f"\nGenerated {now}"


    result += "</p>\n"
    return result





metadata_string = get_metadata_string()


column_options_string = "<option value=-1>All</option>\n"
for i in range(len(output_field_list)):
    column_name = output_field_list[i]
    column_options_string += f"<option value={i}>{column_name}</option>\n"

html_template_file = open("table-template.html", "r")
html_template_string = html_template_file.read()
html_template_file.close()

script_file = open("table-template.js", "r")
script_string = script_file.read()
script_file.close()

html_template_string = html_template_string.replace("<!-- REPLACE WITH TABLE -->", table)
html_template_string = html_template_string.replace("<!-- REPLACE WITH SCRIPT -->", script_string)
html_template_string = html_template_string.replace("<!-- REPLACE WITH COLUMN OPTIONS -->", column_options_string)
html_template_string = html_template_string.replace("<!-- REPLACE WITH METADATA -->", metadata_string)

outfile = open(outfile_name, "w")
outfile.write(html_template_string)
outfile.close()


