# Generate a random strip game based on various parameters

import random
import sys

BLACK = "X"
WHITE = "O"
EMPTY = "."

if "-h" in sys.argv or "--help" in sys.argv:
    print(f"Usage: python3 {sys.argv[0]}")
    exit(0)


def einput(prompt):
    print(prompt, file=sys.stderr, end="")
    return input()


boardLen = int(einput("Strip length: ", ))
minBlackStones = int(einput("Min black stones: "))
maxBlackStones = int(einput("Max black stones: "))

minWhiteStones = int(einput("Min white stones: "))
maxWhiteStones = int(einput("Max white stones: "))
count = int(einput("Number of cases: "))

assert minBlackStones <= maxBlackStones
assert minWhiteStones <= maxWhiteStones
assert maxBlackStones + maxWhiteStones <= boardLen

for i in range(count):
    strip = [EMPTY for i in range(boardLen)]

    blackStones = random.randint(minBlackStones, maxBlackStones)
    whiteStones = random.randint(minWhiteStones, maxWhiteStones)

    # Randomly add stones
    remainingB = blackStones
    remainingW = whiteStones
    choices = [i for i in range(len(strip))]

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

        strip[to] = color

    print("".join(strip))
