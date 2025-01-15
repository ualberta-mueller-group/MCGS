import csv
import sys

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


def row_style_single_mode(input_rows, output_row):
    output_row["games"]["css_classes"].append("break-anywhere")

    outcome = output_row["outcome"]
    if outcome["text"] == "FAIL":
        outcome["css_classes"].append("cell-fail")
        output_row["css_classes"].append("row-fail")
    elif outcome["text"] == "TIMEOUT":
        outcome["css_classes"].append("cell-timeout")
        output_row["css_classes"].append("row-timeout")
    elif outcome["text"] == "COMPLETED":
        outcome["css_classes"].append("cell-completed")


######################################## Define input/output formats

input_field_list = []
input_field_dict = {

}

output_field_list = []
output_field_dict = {
}

row_functions = []


def add_input_row(alias, column_name):
    input_field_list.append(column_name)
    input_field_dict[alias] = column_name


def add_output_row(alias, column_name):
    output_field_list.append(column_name)
    output_field_dict[alias] = column_name


def add_row_function(fn):
    row_functions.append(fn)


def new_default_cell(text):
    return {"css_classes": ["cell"], "text": text}


add_input_row("file", "File")
add_input_row("case", "Case")
add_input_row("games", "Games")
add_input_row("player", "Player")
add_input_row("expected", "Expected")
add_input_row("got", "Got")
add_input_row("time", "Time (ms)")
add_input_row("outcome", "Outcome")
add_input_row("comments", "Comments")
add_input_row("hash", "Input hash")

if len(infile_names) == 1:
    add_output_row("file", "File")
    add_output_row("case", "Case")
    add_output_row("games", "Games")
    add_output_row("player", "Player")
    add_output_row("expected", "Expected")
    add_output_row("got", "Got")
    add_output_row("time", "Time (ms)")
    add_output_row("outcome", "Outcome")
    add_output_row("comments", "Comments")

    add_row_function(row_populate_single_mode)
    add_row_function(row_style_single_mode)


######################################## process rows

infile = open(infile_names[0], "r")
reader = csv.DictReader(infile)

assert len(input_field_list) == len(reader.fieldnames)
for i in range(len(input_field_list)):
    assert input_field_list[i] == reader.fieldnames[i]


table = "<table>\n"

table += "<tr class=\"row-header\">\n"
for field in output_field_list:
    table += f"<th>{field}</th>\n"
table += "</tr>\n"

for reader_row in reader:
    input_row = {}
    for alias in input_field_dict:
        input_row[alias] = reader_row[input_field_dict[alias]]

    output_row = {}
    output_row["css_classes"] = ["row"]

    for fn in row_functions:
        fn([input_row], output_row)

    assert len(output_row) == len(output_field_dict) + 1
    for alias in output_row:
        if alias == "css_classes":
            continue
        assert alias in output_field_dict

    row_class_string = " ".join(output_row["css_classes"])
    row_text = f"<tr class=\"{row_class_string}\">\n"
    for alias in output_row:
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

html_template_file = open("table-template.html", "r")
html_template_string = html_template_file.read()
html_template_file.close()

script_file = open("table-template.js", "r")
script_string = script_file.read()
script_file.close()


html_template_string = html_template_string.replace("<!-- REPLACE WITH TABLE -->", table)
html_template_string = html_template_string.replace("<!-- REPLACE WITH SCRIPT -->", script_string)

outfile = open(outfile_name, "w")
outfile.write(html_template_string)
outfile.close()


