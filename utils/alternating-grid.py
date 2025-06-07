
grids = []


for r in range(2, 6 + 1):
    for c in range(2, 6 + 1):
        if (r == c):
            continue
        grid = (r, c)
        grids.append(grid)


def print_grid(grid):
    n_rows, n_cols = grid

    row_strings = []
    for i in range(n_rows):
        row = ""
        for j in range(n_cols):
            row += ["X", "O"][(i + j) % 2]

        row_strings.append(row)

    grid_string = "|".join(row_strings)

    print(f"/* {n_rows}x{n_cols} */")
    print(grid_string)
    print("\t {B, W}")
    print("")

for grid in grids:
    print_grid(grid)
