import random

BLACK = "X"
WHITE = "O"
EMPTY = "."


boardLen = int(input("Strip length: "))
minBlackStones = int(input("Min black stones: "))
maxBlackStones = int(input("Max black stones: "))

minWhiteStones = int(input("Min white stones: "))
maxWhiteStones = int(input("Max white stones: "))
count = int(input("Number of cases: "))

blackStones = random.randint(minBlackStones, maxBlackStones)
whiteStones = random.randint(minWhiteStones, maxWhiteStones)


assert blackStones + whiteStones <= boardLen


for i in range(count):
    strip = [EMPTY for i in range(boardLen)]

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
