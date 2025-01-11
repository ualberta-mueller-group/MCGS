import csv



"""
infile = open("out.txt", "r")
reader = csv.reader(infile)

firstRow = True

outtext = "<table>"
for row in reader:
    outtext += "<tr>\n"

    for col in row:
        openTag = "<th>" if firstRow else "<td>"
        closeTag = "</th>" if firstRow else "</td>"
        outtext += openTag + col + closeTag + "\n"
    outtext += "</tr>"

    firstRow = False

outtext += "</table></body></html>"

print(outtext)

infile.close()
"""



infile = open("out.txt", "r")
reader = csv.reader(infile)

firstRow = True

outtext = "<table>"
for row in reader:
    outtext += "<tr>\n"

    for col in row:
        openTag = "<th>" if firstRow else "<td>"
        closeTag = "</th>" if firstRow else "</td>"
        className = "header-div" if firstRow else "data-div"

        outtext += f"{openTag} <div class=\"{className}\"> {col} </div> {closeTag}\n"
        #outtext += openTag + col + closeTag + "\n"
    outtext += "</tr>"

    firstRow = False

outtext += "</table></body></html>"

print(outtext)

infile.close()


