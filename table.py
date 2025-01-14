import csv


def identity(text):
    html_data = f"<div class=\"data-div\">{text}</div>"
    return html_data


def outcome(text):
    class_names = ["data-div"]

    if text == "FAIL":
        class_names.append("bg-red")
    elif text == "TIMEOUT":
        class_names.append("bg-orange")
    elif text == "UNSPECIFIED":
        class_names.append("bg-yellow")



    class_names_string = " ".join(class_names)

    html_data = f"<div class=\"{class_names_string}\">{text}</div>"
    return html_data




class FieldTemplate:
    def __init__(self, field_name, function):
        self.field_name = field_name
        self.function = function


field_templates = [
    FieldTemplate("File", identity),
    FieldTemplate("Case", identity),
    FieldTemplate("Games", identity),
    FieldTemplate("Player", identity),
    FieldTemplate("Expected", identity),
    FieldTemplate("Got", identity),
    FieldTemplate("Time (ms)", identity),
    FieldTemplate("Outcome", outcome),
    FieldTemplate("Comments", identity),
    FieldTemplate("Input hash", None),
]

for x in field_templates:
    assert type(x) is FieldTemplate


infile = open("out.csv", "r")
reader = csv.DictReader(infile)


# Check that format is correct
assert len(field_templates) == len(reader.fieldnames)
for i in range(len(field_templates)):
    assert field_templates[i].field_name == reader.fieldnames[i]

outtext = "<table>\n"

outtext += "<tr>\n"
for ft in field_templates:
    if ft.function is None:
        continue
    outtext += f"<th><div class=header-div>{ft.field_name}</div></th>\n"
outtext += "</tr>\n"

for row in reader:
    outtext += "<tr>\n"
    for ft in field_templates:
        if ft.function is None:
            continue
        data = row[ft.field_name]
        outtext += f"<td>{ft.function(data)}</td>"


    outtext += "</tr>\n"

outtext += "</table>\n"


templatefile = open("table-template.html", "r")
outfile = open("table.html", "w")

for line in templatefile:
    line = line.replace("<!-- REPLACE ME -->", outtext)
    outfile.write(line)



outfile.close()
templatefile.close()

infile.close()

