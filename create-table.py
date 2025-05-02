import csv
import sys
import pathlib
import datetime
import hashlib

"""
ttables persist between tests, which causes 2 new problems:

    1: If the tests, or execution order of tests represented in two .csvs
        differ, then times can't be meaningfully compared

    2: create-table.py currently reads the entire comparison .csv file row by
        row and stores the row data in a dict. This may overwrite the first
        instance of a test with the second, which now have different times
        due to ttable persistence. This causes slowdowns (i.e. 10,000,000%) to
        be (incorrectly) reported, when there are no slowdowns, as the
        comparison column will be the time for a single ttable lookup

For 1: Have create-table.py print a warning if the sequence of tests differ
    between .csv files. This can either be done by looking at the sequence
    of input hashes, OR by comparing the sequence of row keys. Or do both and
    mention which happened in the warning?

    Also include the warning in the .html output

For 2: Store all instances of the same test from the comparison file.

    For now I add an assert against duplicate input hashes...

"""

print("TODO: Resolve note at top of create-table.py")


######################################## Warnings
# Track warnings already printed
warned_dict = {
}


def warn_common(warning_name):
    global warned_dict

    warned = warned_dict.get(warning_name)
    if warned is None:
        warned = False

    assert type(warned) is bool

    warned_dict[warning_name] = True

    if not warned:
        print("WARNING: ", end="")

    return warned


def warn_input_file_duplicate_tests():
    if warn_common("input_file_duplicate_tests"):
        return
    print("Primary input file contains duplicate tests")


def warn_comparison_file_duplicate_tests():
    if warn_common("comparison_file_duplicate_tests"):
        return
    print("Comparison file contains duplicate tests")


######################################## Constants
time_threshold_frac = 0.1
time_threshold_abs = 5

# Highlight times that differ by at least time_threshold_abs milliseconds AND have a percent difference
# of at least time_threshold_frac * 100

src_dir = "src"

######################################## Parse args


# Break up text in help string to roughly keep it in a column
def format_print(string, tabs, width):
    words = string.split()
    lines = []
    line = ""

    for word in words:
        if len(line) + len(word) >= width:
            if len(line) > 0:
                lines.append(line)
                line = ""

        if len(line) > 0:
            line += " "
        line += word

    if len(line) > 0:
        lines.append(line)

    for line in lines:
        print(("\t" * tabs) + line)


def print_flag(flag_string, flag_description):
    print(f"\t{flag_string}")

    format_print(flag_description, 2, 40)
    print("")


def print_usage():
    print(f"Usage: python3 {sys.argv[0]} <input csv file> [--compare-to <comparison csv file>] -o <output HTML file>")
    print("")
    print("\tConverts data from MCGS's \"--run-tests\" mode into interactive HTML data tables.")

    print("")
    print("Flags:")
    print_flag("-h, --help", "Print this message and exit.")
    print_flag("-o <output HTML file>", "Specify output file name.")

    print_flag("--compare-to <comparison csv file>", "Specify file to compare \
input data to. Rows from the comparison file not matching any row from the input file \
have no effect. Rows from the input file not matching any row from the \
comparison file will still be output.")

    print("")
    print("Possible values for \"Status\" column:")
    print("")
    print_flag("PASS", "The search result (win/loss) matched the expected result defined by the test case.")
    print_flag("FAIL", "The search result (win/loss) did not match the expected result defined by the test case.")
    print_flag("COMPLETED", "Search finished but the test case doesn't define an expected result.")
    print_flag("TIMEOUT", "Search was aborted due to taking too long, and did not complete.")

    print("")


    print("Possible values for \"Regression\" column:")
    print("")

    print_flag("N/A", "The test case is missing from the comparison file.")
    print_flag("STILL (PASS | FAIL | COMPLETED | TIMEOUT)",
"The test case's current \"Status\", which is the same in both files. On the \
occurrence of \"STILL COMPLETED\", the \"Result\" of the test is consistent across both files.")

    print_flag("NEW (PASS | FAIL | COMPLETED | TIMEOUT)",
"The test case's current \"Status\", which differs between files.")

    print_flag("NEW DIVERGING RESULT", "The test case's \"Status\" is \"COMPLETED\" \
in both the input and comparison files, but with a differing \"Result\".")

    format_print("""\
The \"Regression\" column is shown when using \"--compare-to\", and \
compares the \"Status\" of a test to that of the same test from the comparison file. \
""", 1, 50)

    print("")
    print("")

    format_print("""\
The \"COMBINE AND TAG\" column search option is for power users and should be
used with regex mode. It matches a row based on the text generated by concatenating
all its columns together and prepending them with column number identifiers. i.e.
\"(COL0)somefile.test(COL1)clobber_1xn:XOXO(COL3)B...\". With this option, a row
of column number identifiers is shown in the table, and upon clicking one of its elements, regular
expression text will be appended to the \"Search\" box which will consume all
remaining input up to that column.\
""", 0, 50)


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
    if present, is the row from the comparison file.

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
def row_style(input_rows, output_row):
    assert "games" in output_row
    output_row["games"]["css_classes"].append("break-anywhere")

    assert "status" in output_row
    status = output_row["status"]
    if status["text"] == "FAIL":
        status["css_classes"].append("cell-fail")
        output_row["css_classes"].append("row-fail")
    elif status["text"] == "TIMEOUT":
        status["css_classes"].append("cell-timeout")
        output_row["css_classes"].append("row-timeout")
    elif status["text"] == "COMPLETED":
        status["css_classes"].append("cell-completed")
    elif status["text"] == "PASS":
        status["css_classes"].append("cell-pass")


# Populate/style output row when comparison_file is defined
def row_populate_double_mode(input_rows, output_row):
    input_row = input_rows[0]
    comparison_row = input_rows[1] if len(input_rows) >= 2 else None

    # Populate simple fields
    simple_fields = ["file", "case", "games", "player", "expected_result", "result", "time", "status", "comments", "hash"]
    for alias in simple_fields:
        output_row[alias] = new_default_cell(input_row[alias])


    # Populate and style comparison fields

    # style hash
    if comparison_row is not None:
        if input_row["hash"] != comparison_row["hash"]:
            output_row["hash"]["css_classes"].append("cell-wrong-hash")
            output_row["hash"]["text"] += " BAD HASH"
            output_row["css_classes"].append("row-bad-hash")

    # old_status
    old_status_text = comparison_row["status"] if (comparison_row is not None) else "N/A"
    output_row["old_status"] = new_default_cell(old_status_text)

    # old_result
    old_result_text = comparison_row["result"] if (comparison_row is not None) else "N/A"
    output_row["old_result"] = new_default_cell(old_result_text)

    #old_time
    old_time_text = comparison_row["time"] if (comparison_row is not None) else "N/A"
    output_row["old_time"] = new_default_cell(old_time_text)

    # faster
    faster_by_string = "N/A"
    faster_css = None
    if comparison_row is not None:
        if input_row["status"] == "TIMEOUT" or comparison_row["status"] == "TIMEOUT":
            faster_by_string = "???"
        else:
            num1 = float(input_row["time"])
            num2 = float(comparison_row["time"])
            faster_by_string, faster_css = get_faster(num1, num2)
    output_row["faster"] = new_default_cell(faster_by_string)
    if faster_css is not None:
        output_row["faster"]["css_classes"].append(faster_css)

    # regression
    regression = "N/A"
    regression_css = None
    regression_css_row = None
    if comparison_row is not None:
        regression, regression_css, regression_css_row = get_regression(input_row, comparison_row)
    output_row["regression"] = new_default_cell(regression)
    if regression_css is not None:
        output_row["regression"]["css_classes"].append(regression_css)
    if regression_css_row is not None:
        output_row["css_classes"].append(regression_css_row)


def get_faster(new_time, old_time):
    new_time = max(0.0001, new_time)
    old_time = max(0.0001, old_time)

    time_max = max(new_time, old_time)
    time_min = min(new_time, old_time)

    frac = time_max / time_min
    assert frac >= 1.0

    text = "{:.2f}x ".format(frac)
    text += "AS FAST" if new_time < old_time else "AS SLOW"

    css = None

    diff = new_time - old_time
    if frac >= (1.0 + time_threshold_frac) and abs(diff) >= time_threshold_abs:
        css = "cell-slower" if diff > 0 else "cell-faster"

    return text, css


def get_regression(input_row, comparison_row):
    stat1 = input_row["status"]
    stat2 = comparison_row["status"]

    result1 = input_row["result"]
    result2 = comparison_row["result"]

    # FAIL, PASS, TIMEOUT, COMPLETED
    if stat1 == stat2:
        css = None
        css_row = None
        if stat1 == "FAIL":
            css = "cell-fail"
        if stat1 == "TIMEOUT":
            css = "cell-timeout"
        if stat1 == "PASS":
            css = "cell-pass"
        if stat1 == "COMPLETED":
            if result1 == result2:
                css = "cell-completed"
            else:
                css = "cell-diverging-result"
                css_row = "row-diverging-result"
                return "NEW DIVERGING RESULT", css, css_row
        return f"STILL {stat1}", css, css_row

    if stat1 != stat2:
        css = None
        css_row = None
        if stat1 == "FAIL":
            css = "cell-new-fail"
        if stat1 == "PASS":
            css = "cell-new-pass"
        if stat1 == "COMPLETED":
            css = "cell-new-completed"
        if stat1 == "TIMEOUT":
            css = "cell-new-timeout"
        return f"NEW {stat1}", css, css_row


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

# Map containing all rows from the comparison file. Indexed according to row_match_key
# Elements are lists of rows which have the same
# key (to handle duplicate tests).
# The end of the list contains the first test in the comparison .csv
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
add_input_col("expected_result", "Expected Result")
add_input_col("result", "Result")
add_input_col("time", "Time (ms)")
add_input_col("status", "Status")
add_input_col("comments", "Comments")
add_input_col("hash", "Input hash")

# The order of these output columns defines the output order
if comparison_file_name is None:
    add_output_col("file", "File")
    add_output_col("case", "Case")
    add_output_col("games", "Games")
    add_output_col("player", "Player")
    add_output_col("expected_result", "Expected Result")
    add_output_col("result", "Result")
    add_output_col("time", "Time (ms)")
    add_output_col("status", "Status")
    add_output_col("comments", "Comments")

    add_row_function(row_populate_single_mode)
    add_row_function(row_style)

else:
    assert comparison_file_name is not None
    add_output_col("file", "File")
    add_output_col("case", "Case")
    add_output_col("games", "Games")
    add_output_col("player", "Player")
    add_output_col("expected_result", "Expected Result")
    add_output_col("result", "Result")
    add_output_col("time", "Time (ms)")
    add_output_col("faster", "Time Improvement") #
    add_output_col("old_time", "Old Time (ms)")
    add_output_col("status", "Status")
    add_output_col("regression", "Regression") #
    add_output_col("old_status", "Old Status")
    add_output_col("old_result", "Old Result")
    add_output_col("comments", "Comments")
    add_output_col("hash", "Input hash")

    add_row_function(row_populate_double_mode)
    add_row_function(row_style)
    row_match_key = ["games", "player", "expected_result"]


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

        row_list = comparison_rows.get(row_key)
        if row_list is None:
            comparison_rows[row_key] = []
        else:
            warn_comparison_file_duplicate_tests()

        comparison_rows[row_key].append(input_row)

    comparison_file.close()

for key in comparison_rows:
    comparison_rows[key].reverse()


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
seen_input_row_keys = set()
for reader_row in reader:
    input_row = reader_row_to_input_row(reader_row)

    input_row_key = input_row_get_key(input_row) if comparison_file_name is not None else None

    if input_row_key in seen_input_row_keys:
        warn_input_file_duplicate_tests()
    seen_input_row_keys.add(input_row_key)

    comparison_row_list = comparison_rows.get(input_row_key)
    comparison_row = None
    if comparison_row_list is not None:
        comparison_row = comparison_row_list.pop()

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

html_template_file = open(src_dir + "/table-template.html", "r")
html_template_string = html_template_file.read()
html_template_file.close()

script_file = open(src_dir + "/table-template.js", "r")
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

