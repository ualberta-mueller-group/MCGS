# Computes sums of random CGT games, outputting a valid MCGS input file.
# Includes equivalent expressions to create the games directly in C++, and
# in CGSuite
import random
import sys

if len(sys.argv) != 2 or "-h" in sys.argv or "--help" in sys.argv:
    print(f"Usage: python3 {sys.argv[0]} <out file name>")
    exit(0)


outfile_name = sys.argv[1]
outfile = open(outfile_name, "w")

ngames = 14
nsums = 6


def convert_cpp_arg_single(arg):
    if type(arg) is bool:
        return "true" if arg else "false"
    return f"{arg}"


def convert_cpp_args(args):
    line = ""
    N = len(args)
    for i in range(N):
        line += convert_cpp_arg_single(args[i])
        if (i + 1) < N:
            line += ", "
    return line


def get_create_expr(cpp_type, args):
    line = f"make_shared<{cpp_type}>("
    line += f"{convert_cpp_args(args)}),"
    return line


class integer_game:
    def __init__(self):
        self.val = random.randint(-10, 10)

    def as_mcgs_input(self):
        return f"[integer_game] {self.val}"

    def as_cgsuite_input(self, non_canonical):
        line = f"({self.val})"
        if non_canonical:
            line = f"nc{line}"
        return line

    def as_cpp(self):
        return get_create_expr("integer_game", [self.val])


class up_star:
    def __init__(self):
        self.ups = random.randint(-20, 20)
        self.star = True if (random.randint(0, 1) == 0) else False

    def as_mcgs_input(self):
        line = f"[up_star] ({self.ups}"
        if self.star:
            line += " *)"
        else:
            line += ")"
        return line

    def as_cgsuite_input(self, non_canonical):
        line = f"(^({self.ups})"
        if self.star:
            line += " + *)"
        else:
            line += ")"
        if non_canonical:
            line = f"nc{line}"
        return line

    def as_cpp(self):
        return get_create_expr("up_star", [self.ups, self.star])


class dyadic_rational:
    def __init__(self):
        self.top = random.randint(-48, 48)
        self.bottom = 1 << random.randint(0, 5)

    def as_mcgs_input(self):
        line = f"[dyadic_rational] ({self.top}/{self.bottom})"
        return line

    def as_cgsuite_input(self, non_canonical):
        line = f"({self.top}/{self.bottom})"
        if non_canonical:
            line = f"nc{line}"
        return line

    def as_cpp(self):
        return get_create_expr("dyadic_rational", [self.top, self.bottom])


class nimber:
    def __init__(self):
        self.val = random.randint(0, 16)

    def as_mcgs_input(self):
        return f"[nimber] {self.val}"

    def as_cgsuite_input(self, non_canonical):
        line = f"*{self.val}"
        if non_canonical:
            line = f"nc({line})"
        return line

    def as_cpp(self):
        return get_create_expr("nimber", [self.val])


class switch_game:
    def __init__(self):
        self.left_top = random.randint(-48, 48)
        self.left_bottom = 1 << random.randint(0, 5)

        self.right_top = random.randint(-48, 48)
        self.right_bottom = 1 << random.randint(0, 5)

    def as_mcgs_input(self):
        return f"[switch_game] ({self.left_top}/{self.left_bottom}, {self.right_top}/{self.right_bottom})"

    def as_cgsuite_input(self, non_canonical):
        left_option = f"{self.left_top}/{self.left_bottom}"
        right_option = f"{self.right_top}/{self.right_bottom}"

        if non_canonical:
            left_option = f"nc({left_option})"
            right_option = f"nc({right_option})"

        line = "{" + left_option + " | " + right_option + "}"

        return line

    def as_cpp(self):
        args = []
        args.append(f"fraction({self.left_top}, {self.left_bottom})")
        args.append(f"fraction({self.right_top}, {self.right_bottom})")
        return get_create_expr("switch_game", args)


classes = [integer_game, up_star, dyadic_rational, nimber, switch_game]


def get_random_game():
    game_class = random.choice(classes)
    return game_class()


outfile.write("{version 1.2}\n")

cgsuite_string = ""
cpp_string = ""

for i in range(nsums):
    cgsuite_string += "["

    outfile.write(f"/* Sum #{i} */\n")
    for j in range(ngames):
        g = get_random_game()
        outfile.write(g.as_mcgs_input() + "\n")

        cgsuite_string += g.as_cgsuite_input(True)
        if (j + 1) in range(ngames):
            cgsuite_string += ", "

        #cpp_string += g.as_cpp()
        #if (j + 1) in range(ngames):
        #    cpp_string += "\n"
    outfile.write("{B, W}\n\n")
    cgsuite_string += "],\n"

#outfile.write("{B, W}\n")

outfile.write("\n")
outfile.write(f"/*_ CGSUITE INPUT\n{cgsuite_string}\n*/\n")
#outfile.write("\n")
#outfile.write(f"/*_ CPP STRING\n{cpp_string}\n*/\n")

outfile.close()
