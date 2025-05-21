# Generate a random strip game based on various parameters

import random
import sys

BLACK = "X"
WHITE = "O"
EMPTY = "."
SEP = "|"

if "-h" in sys.argv or "--help" in sys.argv:
    print(f"Usage: python3 {sys.argv[0]}")
    exit(0)


def einput(prompt):
    print(prompt, file=sys.stderr, end="")
    return input()


boardRows = int(einput("Board rows: ", ))
boardCols = int(einput("Board cols: ", ))
boardCells = boardRows * boardCols

minBlackStones = int(einput("Min black stones: "))
maxBlackStones = int(einput("Max black stones: "))

minWhiteStones = int(einput("Min white stones: "))
maxWhiteStones = int(einput("Max white stones: "))
count = int(einput("Number of cases: "))

assert minBlackStones <= maxBlackStones
assert minWhiteStones <= maxWhiteStones
assert maxBlackStones + maxWhiteStones <= boardCells

for i in range(count):
    grid = [[EMPTY for c in range(boardCols)] for r in range(boardRows)]

    blackStones = random.randint(minBlackStones, maxBlackStones)
    whiteStones = random.randint(minWhiteStones, maxWhiteStones)

    # Randomly add stones
    remainingB = blackStones
    remainingW = whiteStones
    choices = [(r, c) for r in range(boardRows) for c in range(boardCols)]

    while len(choices) > 0 and remainingB + remainingW > 0:
        colors = []
        if remainingB:
            colors.append(BLACK)

        if remainingW:
            colors.append(WHITE)

        color = random.choice(colors)

        if color == BLACK:
            remainingB -= 1
        if color == WHITE:
            remainingW -= 1

        to = random.choice(choices)
        choices.remove(to)

        r, c = to
        row = grid[r]
        row[c] = color

    row_strings = ["".join(row) for row in grid]
    grid_string = "|".join(row_strings)
    print(grid_string)

