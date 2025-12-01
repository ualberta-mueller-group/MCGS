# Generate a random grid game based on various parameters

import random
import sys

BLACK = "X"
WHITE = "O"
EMPTY = "."
BORDER = "#"
SEP = "|"

if "-h" in sys.argv or "--help" in sys.argv:
    print(f"Usage: python3 {sys.argv[0]}")
    exit(0)


def einput(prompt):
    print(prompt, file=sys.stderr, end="")
    return input()


useColor = int(einput("Use color? (0, 1): "))
assert 0 <= useColor and useColor <= 1
useColor = False if useColor == 0 else True

boardRows = int(einput("Board rows: ", ))
boardCols = int(einput("Board cols: ", ))
boardCells = boardRows * boardCols

minBorders = int(einput("Min borders: "))
maxBorders = int(einput("Max borders: "))
assert minBorders <= maxBorders

if useColor:
    minBlackStones = int(einput("Min black stones: "))
    maxBlackStones = int(einput("Max black stones: "))

    minWhiteStones = int(einput("Min white stones: "))
    maxWhiteStones = int(einput("Max white stones: "))

    assert minBlackStones <= maxBlackStones
    assert minWhiteStones <= maxWhiteStones
    assert maxBorders + maxBlackStones + maxWhiteStones <= boardCells
else:
    minStones = int(einput("Min stones: "))
    maxStones = int(einput("Max stones: "))

    assert minStones <= maxStones
    assert maxBorders + maxStones <= boardCells


count = int(einput("Number of cases: "))

def getRandomColorBoard():
    assert useColor

    blackStones = random.randint(minBlackStones, maxBlackStones)
    whiteStones = random.randint(minWhiteStones, maxWhiteStones)
    borders = random.randint(minBorders, maxBorders)

    grid = [[EMPTY for c in range(boardCols)] for r in range(boardRows)]

    remainingB = blackStones
    remainingW = whiteStones
    remainingBorders = borders

    choices = [(r, c) for r in range(boardRows) for c in range(boardCols)]

    while len(choices) > 0 and remainingB + remainingW + remainingBorders > 0:
        colors = []
        if remainingB:
            colors.append(BLACK)

        if remainingW:
            colors.append(WHITE)

        if remainingBorders:
            colors.append(BORDER)

        color = random.choice(colors)

        if color == BLACK:
            remainingB -= 1
        if color == WHITE:
            remainingW -= 1
        if color == BORDER:
            remainingBorders -= 1

        to = random.choice(choices)
        choices.remove(to)

        r, c = to
        row = grid[r]
        row[c] = color
    return grid


def getRandomMonoBoard():
    assert not useColor
    stones = random.randint(minStones, maxStones)

    grid = [[EMPTY for c in range(boardCols)] for r in range(boardRows)]

    remaining = stones

    choices = [(r, c) for r in range(boardRows) for c in range(boardCols)]

    while len(choices) > 0 and remaining > 0:
        color = BORDER
        remaining -= 1

        to = random.choice(choices)
        choices.remove(to)

        r, c = to
        row = grid[r]
        row[c] = color
    return grid

def getRandomBoard():
    if useColor:
        return getRandomColorBoard()
    return getRandomMonoBoard()


for i in range(count):
    grid = getRandomBoard()

    row_strings = ["".join(row) for row in grid]
    grid_string = "|".join(row_strings)
    print(grid_string)

