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


######################################## Style functions
# These functions return: [<displayed text>, <local CSS class list>, <row CSS class list>]
# Local CSS classes are applied to <td> elements. Row CSS classes are applied to <tr> elements

def style_basic(text):
    return [text, [], []]


def style_games(text):
    return [text, ["break-anywhere"], []]


def style_outcome(text):
    class_names = []
    class_names_row = []

    if text == "FAIL":
        class_names.append("bg-red")
        class_names_row.append("row-fail")
    elif text == "TIMEOUT":
        class_names.append("bg-orange")
        class_names_row.append("row-timeout")
    elif text == "COMPLETED":
        class_names.append("bg-yellow")

    return [text, class_names, class_names_row]

######################################## FieldTemplates
# If a style function is missing, the field will be omitted from the output


class FieldTemplate:
    def __init__(self, field_name, style_function):
        self.field_name = field_name
        self.style_function = style_function


field_templates = [
    FieldTemplate("File", style_basic),
    FieldTemplate("Case", style_basic),
    FieldTemplate("Games", style_games),
    FieldTemplate("Player", style_basic),
    FieldTemplate("Expected", style_basic),
    FieldTemplate("Got", style_basic),
    FieldTemplate("Time (ms)", style_basic),
    FieldTemplate("Outcome", style_outcome),
    FieldTemplate("Comments", style_basic),
    FieldTemplate("Input hash", None),
]


######################################## Read files and build output

for x in field_templates:
    assert type(x) is FieldTemplate

infile1 = open(infile_names[0], "r")
reader = csv.DictReader(infile1)


# Check that format is correct
assert len(field_templates) == len(reader.fieldnames)

for i in range(len(field_templates)):
    assert field_templates[i].field_name == reader.fieldnames[i]

outtext = "<table id=\"data-table\">\n"

# Make header row
outtext += "<tr>\n"
for ft in field_templates:
    if ft.style_function is None:
        continue

    outtext += f"<th><div class=header-div>{ft.field_name}</div></th>\n"

outtext += "</tr>\n"

# Make table rows
for row in reader:
    rowtext = ""
    row_classes = ["row-data"]

    for ft in field_templates:
        if ft.style_function is None:
            continue
        data = row[ft.field_name]

        display_text, class_list, class_list_row = ft.style_function(data)
        class_list_string = " ".join(class_list)
        for c in class_list_row:
            row_classes.append(c)

        rowtext += f"<td class=\"{class_list_string}\"><div class=\"data-div\">{display_text}</div></td>"

    row_classes_string = " ".join(row_classes)

    rowtext = f"<tr class=\"{row_classes_string}\">" + "\n" + rowtext + "</tr>\n"
    outtext += rowtext

outtext += "</table>\n"


templatefile = open("table-template.html", "r")

# Read JS file
scriptfile = open("table-template.js", "r")
scripttext = scriptfile.read()
scriptfile.close()


outfile = open(outfile_name, "w")

for line in templatefile:
    line = line.replace("<!-- REPLACE WITH TABLE -->", outtext)
    line = line.replace("<!-- REPLACE WITH SCRIPT -->", scripttext)
    outfile.write(line)

outfile.close()
templatefile.close()

infile1.close()

