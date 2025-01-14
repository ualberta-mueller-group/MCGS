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

INCLUDE_ALL = "ALL"
INCLUDE_FAIL = "FAIL"
INCLUDE_TIMEOUT = "TIMEOUT"

filter_outcome = INCLUDE_ALL



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
# These return: [<displayed text>, <CSS class list>]

def style_basic(text):
    return [text, []]


def style_outcome(text):
    class_names = []

    if text == "FAIL":
        class_names.append("bg-red")
    elif text == "TIMEOUT":
        class_names.append("bg-orange")
    elif text == "COMPLETED":
        class_names.append("bg-yellow")

    return [text, class_names]

######################################## FieldTemplates
# If a style function is missing, the field will be omitted from the output

class FieldTemplate:
    def __init__(self, field_name, style_function):
        self.field_name = field_name
        self.style_function = style_function


field_templates = [
    FieldTemplate("File", style_basic),
    FieldTemplate("Case", style_basic),
    FieldTemplate("Games", style_basic),
    FieldTemplate("Player", style_basic),
    FieldTemplate("Expected", style_basic),
    FieldTemplate("Got", style_basic),
    FieldTemplate("Time (ms)", style_basic),
    FieldTemplate("Outcome", style_outcome),
    FieldTemplate("Comments", style_basic),
    FieldTemplate("Input hash", None),
]

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
    outtext += "<tr>\n"
    for ft in field_templates:
        if ft.style_function is None:
            continue
        data = row[ft.field_name]

        display_text, class_list = ft.style_function(data)
        class_list_string = " ".join(class_list)

        outtext += f"<td class=\"{class_list_string}\"><div class=\"data-div\">{display_text}</div></td>"

    outtext += "</tr>\n"

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

