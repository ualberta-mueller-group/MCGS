"""
    Generates stubs for unit tests spanning several game types.

    Edit these variables below:
        "base_names": List of games to generate stubs for. See comment
                      explaining format above the variable.
        "test_name": Name of the test, i.e. "split_test".

    Then run this script. Possible output files:
        split_test_stubs (directory)
            split_test.h
            split_test.cpp <- Defines split_test_all(), which calls tests
                              for all game types.

            split_test_clobber.h
            split_test_clobber.cpp <- Defines split_test_clobber_all()

            split_test_dyadic_rational.h
            split_test_dyadic_rational.cpp
"""

import os

##################################################

# List of file names with .h/.cpp omitted. The name of the game class within
# should be the base name, minus the cgt_ prefix. i.e:
# cgt_dyadic_rational --> cgt_dyadic_rational.h/.cpp, with class dyadic_rational
base_names = [
    "cgt_dyadic_rational",
    "cgt_integer_game",
    "cgt_nimber",
    "cgt_switch",
    "cgt_up_star",
    "clobber_1xn",
    "elephants",
    "nogo_1xn",
    "nogo",
    "clobber",
    "kayles",
]

test_name = "order_test"

##################################################

dir_name = f"{test_name}_stubs"
assert not os.path.exists(dir_name)

os.mkdir(dir_name)


def write_file(file_name, contents):
    assert type(file_name) is str
    assert type(contents) is str
    real_file_name = f"{dir_name}/{file_name}"
    assert not os.path.exists(real_file_name)

    f = open(real_file_name, "w")
    f.write(contents)
    f.close()


# 2-tuples: (header name, function name)
references = []


def assert_is_reference(ref):
    assert type(ref) is tuple
    assert len(ref) == 2
    for x in ref:
        assert type(x) is str


for base_name in base_names:
    stripped_name = base_name.replace("cgt_", "")

    root_name = f"{test_name}_{stripped_name}"
    func_name = f"{root_name}_all"
    h_name = f"{root_name}.h"
    cpp_name = f"{root_name}.cpp"

    references.append((h_name, func_name))

    h_contents = f"#pragma once\nvoid {func_name}();\n"
    cpp_contents = f"#include \"{h_name}\"\n#include <iostream>\n"
    cpp_contents += f"#include \"{base_name}.h\"\n\n"
    cpp_contents += f"void {func_name}()\n"
    cpp_contents += "{\n    "
    cpp_contents += "std::cout << __FILE__ << std::endl;\n"
    cpp_contents += "}"

    write_file(h_name, h_contents)
    write_file(cpp_name, cpp_contents)


# now make main file
h_all_name = f"{test_name}.h"
cpp_all_name = f"{test_name}.cpp"
func_all_name = f"{test_name}_all"

h_contents = f"#pragma once\nvoid {func_all_name}();\n"
cpp_contents = f"#include \"{h_all_name}\"\n#include <iostream>\n"
for ref in references:
    assert_is_reference(ref)

    cpp_contents += f"#include \"{ref[0]}\"\n"

cpp_contents += f"\nvoid {func_all_name}()\n"
cpp_contents += "{\n    std::cout << __FILE__ << std::endl;\n\n"

for ref in references:
    assert_is_reference(ref)
    cpp_contents += f"    {ref[1]}();\n"

cpp_contents += "}"

write_file(h_all_name, h_contents)
write_file(cpp_all_name, cpp_contents)
