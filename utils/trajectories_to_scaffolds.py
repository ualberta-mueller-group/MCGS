# Converts CGSuite thermograph strings to MCGS/cgt_lib scaffolds.
# Example usage:
# python3 cgs_thermograph_to_scaffolds.py "Thermograph(-5,[],[0],-5,[],[0])"

from fractions import Fraction as frac
import sys

assert len(sys.argv) == 2
cgs_thermograph = sys.argv[1]

# string -> string
def remove_thermograph_prefix(in_string):
    assert in_string.find("Thermograph") == 0
    return in_string[len("Thermograph"):]

# string, char, char -> string
def remove_enclosing(in_string, left, right):
    assert in_string[0] == left and in_string[-1] == right
    return in_string[1 : len(in_string) - 1]

# string -> [int, string] or None
def get_int(in_string):
    l = len(in_string)

    if l == 0:
        return None

    i = 0

    if in_string[0] == "-":
        i = i + 1

    has_number = False

    while i < l:
        c = in_string[i]
        if c.isnumeric():
            i = i + 1
            has_number = True
        else:
            break

    if has_number:
        return [int(in_string[0:i]), in_string[i:]]

    assert i == 0
    return None

# string -> [frac, string] or None
def get_frac(in_string):
    res1 = get_int(in_string)

    if res1 is None:
        return None

    top, in_string = res1

    if len(in_string) == 0 or in_string[0] != "/":
        return [frac(top, 1), in_string]

    in_string = in_string[1 :]
    res2 = get_int(in_string)
    assert res2 is not None

    bot, in_string = res2
    return [frac(top, bot), in_string]

# string -> [list<frac>, string]
def get_frac_list(in_string):
    fracs = []

    assert in_string[0] == "["
    in_string = in_string[1 : ]

    while True:
        res = get_frac(in_string)

        if res is None:
            break

        f, in_string = res
        fracs.append(f)

        if in_string[0] == ",":
            in_string = in_string[1 :]
        else:
            break

    assert in_string[0] == "]"
    in_string = in_string[1 :]

    return [fracs, in_string]

# string -> [cgsuite_trajectory, string]
# where cgsuite_trajectory is a list of 3 items:
#   1. Mast X
#   2. List of Y values of all inflection points (descending, omits Y=-1).
#      May be empty. When not empty, the first value is Mast Y.
#   3. List of slopes (dx/dy), (above each corresponding inflection point,
#      looking upward). There is 1 more slope than inflection point; the caller
#      should add Y=-1 to the inflection point list to remedy this.
def get_trajectory(in_string):
    mast_value, in_string = get_frac(in_string)

    assert in_string[0] == ","
    in_string = in_string[1:]

    critical_points, in_string = get_frac_list(in_string)

    assert in_string[0] == ","
    in_string = in_string[1:]

    slopes, in_string = get_frac_list(in_string)

    trajectory = [mast_value, critical_points, slopes]

    return [trajectory, in_string]

# cgsuite_trajectory -> scaffold
def get_scaffold(trajectory):
    t_mx, t_ys, t_slopes = trajectory

    t_ys.append(frac(-1, 1))
    assert len(t_ys) == len(t_slopes)

    t_my = t_ys[0]

    # Start Y=1 above mast, work downward to Y=-1
    points = [(t_mx, t_my + 1), (t_mx, t_my)]

    for i in range(1, len(t_ys)):
        last_x, last_y = points[-1]

        py = t_ys[i]
        s = t_slopes[i]
        
        # Note: slope is (dx/dy)
        # last_x = px + s * (last_y - py) -->
        px = last_x - s * (last_y - py)

        points.append((px, py))

    # cgt_lib format goes bottom to top
    points.reverse()
    return points

# string -> [scaffold, scaffold]
def parse_cgsuite_thermograph(in_string):
    in_string = remove_thermograph_prefix(in_string)
    in_string = remove_enclosing(in_string, "(", ")")

    t1, in_string = get_trajectory(in_string)

    assert in_string[0] == ","
    in_string = in_string[1:]

    t2, in_string = get_trajectory(in_string)

    assert len(in_string) == 0

    sc1 = get_scaffold(t1)
    sc2 = get_scaffold(t2)

    return [sc1, sc2]


def print_frac(f, end=""):
    if f.denominator == 1:
        print(str(f.numerator), end=end)
    else:
        print(str(f.numerator) + "/" + str(f.denominator), end=end)

def print_point(p, end=""):
    print("(", end="")
    print_frac(p[0], end="")
    print(", ", end="")
    print_frac(p[1], end="")
    print(")", end=end)

def print_scaffold(sc, end=""):
    for i in range(len(sc)):
        if i > 0:
            print(" ", end="")
        print_point(sc[i])
    print("", end=end)

##################################################
sc1, sc2 = parse_cgsuite_thermograph(cgs_thermograph)

print("L: ", end="")
print_scaffold(sc1, end="\n")

print("R: ", end="")
print_scaffold(sc2, end="\n")
