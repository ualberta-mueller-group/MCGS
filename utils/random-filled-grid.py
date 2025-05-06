import random

CASES = 20

MIN_R = 2
MAX_R = 10

MIN_C = 2
MAX_C = 10

MAX_AREA = 36

def get_char():
    return ["X", "O"][random.randint(0, 1)]

for case in range(CASES):
    area = MAX_AREA
    while (area >= MAX_AREA):
        n_rows = random.randint(MIN_R, MAX_R)
        n_cols = random.randint(MIN_C, MAX_C)
        area = n_rows * n_cols

    grid = [[get_char() for c in range(n_cols)] for r in range(n_rows)]

    row_strings = ["".join(row) for row in grid]
    grid_string = "|".join(row_strings)

    print(f"/* case {case} */")
    print(f"/* random filled {n_rows}x{n_cols} */")
    print(grid_string)
    print("\t{B, W}")
    print("")


