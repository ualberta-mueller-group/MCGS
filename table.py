import csv
import sys
import pathlib
import datetime
import hashlib


######################################## Constants
time_threshold_frac = 0.1
time_threshold_abs = 5

# Highlight times that differ by at least time_threshold_abs milliseconds AND have a percent difference
# of at least time_threshold_frac * 100

######################################## Parse args


def print_flag(flag_string, flag_description):
    print(f"\t{flag_string}")
    print(f"\t\t{flag_description}")


def print_usage():
    print(f"Usage: python3 {sys.argv[0]} <input csv file> [--compare-to <comparison csv file>] -o <output HTML file>")
    print("")
    print("\tConverts test data from MCGS into interactive HTML data tables.")
    print("")
    print(f"Flags:")
    print_flag("-h, --help", "Print this message and exit.")
    print_flag("-o <output HTML file>", "Specify output file name.")

    print_flag("--compare-to <comparison csv file>", "Specify file to compare \
input data to. Rows from the comparison file not matching any row from the input file \
have no effect. Rows from the input file not matching any row from the \
comparison file will still be output.")


# Mandatory:
infile_name = None
outfile_name = None

# Optional; could still be None later:
comparison_file_name = None

# Parsing loop
skip_next = False
for i in range(1, len(sys.argv)): # skip source file name argument
    # Skip this arg if it was already consumed
    if skip_next:
        skip_next = False
        continue

    arg = sys.argv[i]
    arg_next = sys.argv[i + 1] if i + 1 < len(sys.argv) else None

    if arg == "-o":
        skip_next = True

        if arg_next is None or outfile_name is not None:
            print_usage()
            exit(-1)

        outfile_name = arg_next
        continue

    if arg in ["-h", "--help"]:
        print_usage()
        exit(0)

    if arg == "--compare-to":
        skip_next = True
        if arg_next is None or comparison_file_name is not None:
            print_usage()
            exit(-1)
        comparison_file_name = arg_next
        continue

    # Should only have one input file not attached to a flag
    if infile_name is not None:
        print_usage()
        exit(-1)
    infile_name = arg

if infile_name is None or outfile_name is None:
    print_usage()
    exit(-1)


######################################## row functions

"""
Row functions collectively define a pipeline to convert data for 1 test
    case into 1 output row for that test case, and receive 2 arguments:

input_rows: A list of dicts representing relevant rows from the input files.
    The first dict is the row from the main input file, and the second dict,
    if present, is the row from from the comparison file.

output_row: A dict representing the output row.

No value is returned; output_row is instead modified, and the same output_row
    dict object is passed to each subsequent function in the pipeline.

    The input row format is:
{
    <input field alias (string)>: <field value (string)>,
}

i.e.:
{
    "game": "clobber_1xn:XO",
    "time": "12.12",
}

    The output row format is:
{
    "css_classes": <row CSS classes (list of strings)>,

    <output field alias (string)>: {
            "css_classes": <cell CSS classes (list of strings)>,
            "text": <output field value (string)>,
    },
}

i.e.:
{
    "css_classes": ["row", "row_fail"],

    "games": {
        "css_classes": ["cell", "break-anywhere"],
        "text": "clobber_1xn:XOXOXO",
    },

    "status": {
        "css_classes": ["cell", "cell-timeout"],
        "text": "TIMEOUT",
    },
}

The output_row passed to the first function in the pipeline only contains
    the "css_classes" pair.

"""


# Default values for a cell contained by a row
def new_default_cell(text):
    return {"css_classes": ["cell"], "text": text}


# Populate output row when comparison_file is absent
def row_populate_single_mode(input_rows, output_row):
    input_row = input_rows[0]

    # Just copy the fields (assumes aliases are the same for input and output)
    for alias in output_field_dict:
        output_row[alias] = new_default_cell(input_row[alias])


# Add some CSS classes
def row_style(_input_rows, output_row):
    assert "games" in output_row
    output_row["games"]["css_classes"].append("break-anywhere")

    assert "outcome" in output_row
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


# Populate/style output row when comparison_file is defined
def row_populate_double_mode(input_rows, output_row):
    input_row = input_rows[0]
    comparison_row = input_rows[1] if len(input_rows) >= 2 else None

    # Populate simple fields
    simple_fields = ["file", "case", "games", "player", "expected", "got", "time", "outcome", "comments", "hash"]
    for alias in simple_fields:
        output_row[alias] = new_default_cell(input_row[alias])


    # Populate and style comparison fields

    # style hash
    if comparison_row is not None:
        if input_row["hash"] != comparison_row["hash"]:
            output_row["hash"]["css_classes"].append("cell-wrong-hash")
            output_row["hash"]["text"] += " BAD HASH"
            output_row["css_classes"].append("row-bad-hash")

    # oldoutcome
    oldoutcome_text = comparison_row["outcome"] if (comparison_row is not None) else "N/A"
    output_row["oldoutcome"] = new_default_cell(oldoutcome_text)

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
            faster_by_string = "{:.2f}".format(faster_by) # 2 decimal places
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


def get_faster_css(new_time, old_time):
    frac_diff = abs(new_time - old_time) / ((new_time + old_time) / 2)
    diff = new_time - old_time
    if frac_diff >= time_threshold_frac and abs(diff) >= time_threshold_abs:
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
# Input and output fields have aliases to make them easy to refer to in the
# "input row" and "output row" formats

# Input field names, as they appear in input .csv files
input_field_list = []
# Input alias --> input field name
input_field_dict = {

}

# Output field names, as they will appear in output .csv
output_field_list = []
# Output alias --> output field name
output_field_dict = {
}

# Pipeline of functions to convert data for a test case into a single output row
row_functions = []

# List of input field aliases used to match rows between main input file
# and comparison file. Or None
row_match_key = None

# Map containing all rows from the comparison file, indexed according to row_match_key
# Rows are in "input row" format
comparison_rows = {
}


def add_input_col(alias, column_name):
    assert column_name not in input_field_list
    assert alias not in input_field_dict

    input_field_list.append(column_name)
    input_field_dict[alias] = column_name


def add_output_col(alias, column_name):
    assert column_name not in output_field_list
    assert alias not in output_field_dict

    output_field_list.append(column_name)
    output_field_dict[alias] = column_name


def add_row_function(fn):
    row_functions.append(fn)


# The order of these input columns must match the input files
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

# The order of these output columns defines the output order
if comparison_file_name is None:
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
    assert comparison_file_name is not None
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
    add_output_col("hash", "Input hash")

    add_row_function(row_populate_double_mode)
    add_row_function(row_style)
    row_match_key = ["games", "player", "expected"]


######################################## process rows

# Validate row_match_key
if row_match_key is not None:
    for alias in row_match_key:
        assert alias in input_field_dict


# Validate input file columns (reader is a csv.DictReader)
def assert_correct_reader_fields(reader):
    assert len(input_field_list) == len(reader.fieldnames)
    for i in range(len(input_field_list)):
        assert input_field_list[i] == reader.fieldnames[i]


# Row format conversion
def reader_row_to_input_row(reader_row):
    input_row = {}
    for alias in input_field_dict:
        input_row[alias] = reader_row[input_field_dict[alias]]

    return input_row


# Get the key (according to row_match_key) of an input row
def input_row_get_key(input_row):
    assert row_match_key is not None

    key = []
    for alias in row_match_key:
        key.append(input_row[alias])

    return tuple(key)


# Read all rows from the comparison file, if specified
if comparison_file_name is not None:
    comparison_file = open(comparison_file_name, "r")
    reader = csv.DictReader(comparison_file)
    assert_correct_reader_fields(reader)

    for reader_row in reader:
        input_row = reader_row_to_input_row(reader_row)
        row_key = input_row_get_key(input_row)
        comparison_rows[row_key] = input_row

    comparison_file.close()


# Start processing the input file
infile = open(infile_name, "r")
reader = csv.DictReader(infile)
assert_correct_reader_fields(reader)

# Start HTML table string
table_string = "<table id=\"data-table\">\n"

# Column names header
table_string += "<tr class=\"row-header\">\n"
for field in output_field_list:
    table_string += f"<th>{field}</th>\n"
table_string += "</tr>\n"

# Column index row (hidden by default)
table_string += "<tr class=\"row-header\" id=\"col-indices\">\n"
for i in range(len(output_field_list)):
    table_string += f"<th class=\"th-index\">(COL{i})</th>\n"
table_string += "</tr>\n"


# Read each input row, making an output row
for reader_row in reader:
    input_row = reader_row_to_input_row(reader_row)

    input_row_key = input_row_get_key(input_row) if comparison_file_name is not None else None
    comparison_row = comparison_rows.get(input_row_key)

    input_rows = [input_row]
    if comparison_row is not None:
        input_rows.append(comparison_row)

    output_row = {}
    output_row["css_classes"] = ["row"]

    for fn in row_functions:
        fn(input_rows, output_row)

    # Validate output row
    assert len(output_row) == len(output_field_dict) + 1 # +1 because of css_classes
    for alias in output_row:
        if alias == "css_classes":
            continue
        assert alias in output_field_dict # Only need to check this in one direction because keys are unique

    # Create HTML for row
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

    table_string += row_text

table_string += "</table>\n"
infile.close()

######################################## Write HTML file


# Timestamps and MD5s of input files (and date of output file)
def get_metadata_string():
    file_names = [infile_name]
    if comparison_file_name is not None:
        file_names.append(comparison_file_name)

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
    assert len(file_names) == len(hashes)

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

# Search column dropdown menu options
column_options_string = "<option value=-1>All</option>\n"

for i in range(len(output_field_list)):
    column_name = output_field_list[i]
    column_options_string += f"<option value={i}>{column_name}</option>\n"
column_options_string += "<option value=-2>COMBINE AND TAG</option>\n"

html_template_file = open("table-template.html", "r")
html_template_string = html_template_file.read()
html_template_file.close()

script_file = open("table-template.js", "r")
script_string = script_file.read()
script_file.close()

# Replace the smaller things first (order of these affects performance)
html_template_string = html_template_string.replace("<!-- REPLACE WITH METADATA -->", metadata_string)
html_template_string = html_template_string.replace("<!-- REPLACE WITH COLUMN OPTIONS -->", column_options_string)
html_template_string = html_template_string.replace("<!-- REPLACE WITH SCRIPT -->", script_string)
html_template_string = html_template_string.replace("<!-- REPLACE WITH TABLE -->", table_string)

outfile = open(outfile_name, "w")
outfile.write(html_template_string)
outfile.close()

