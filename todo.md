# BUGS

# V0 todo
- Tag version 0

# V1 todo
- define goals for first, minimalistic sumgame class
- rewrite nim to use sumgame and nimbers classes
- Can handle type conversion from game to option with sum game
    - move creates list of new subgames
    - those can include a new type of game
        - e.g. play in a `switch_game`  or `dyadic_rational` 
        can create an integer
- replace a game (e.g. clobber position) by an equal game of simpler type
    - when we learn equality after evaluation or database lookup
    - who manages the memory of the new and old games?

# Todo Design and Naming Issues
- rename `x_1xn` to `x_strip` ?
- play() can change the type of game
    - how to handle?
    - what if a game such as clobber is equal to a simpler game such as up
    - should zero be its own type??

# Todo Coding - deferred to V1
- simplify() hook for game
    - simplify nim - remove equal pairs
- move ordering hook for game
    - move ordering in nim? match other game value?
- search stats: node count, leaf count, time, depth
    - later: transposition hits, simplifications, zero removal, inverse removal
- use a proper unit test framework, maybe https://github.com/siu/minunit
- `cgt_game` class - define game by left+right options, read from string
- scaling experiments, scaling test suites, e.g. scale size, scale number of subgames
- make board implementations (char, int, bitset, list?) separate from game classes, with common interface - allow composition of different board implementations with game mechanics
- `rule_set` class as in CGSuite?
- implement generic game::inverse(), with overrides?
    h = inverse(g) ???
- should we allow fractions in `switch_game`? e.g. 1/2 | -3/4

## Todo - Tests

### Nogo
- Nogo - add test cases for move generator

### Nim
- random testing for nim 
    - increase size limit as program becomes better
    - generate 2nd player win game by adding nim sum

### Clobber
- get, convert existing small clobber tests
- From class in `~/Projects/ualberta-mueller-group/combinatorial_game_solver/PriorWork`
- From Taylor's solver 

### Randomised Testing
- add random game generation in general? 
    - size parameter for random games? `game.generate_random(size)`?
- how to check results of testing with random games?

### Test Cases Implementation
- should just read one by one, no need for vector. write an iterator???
- `DO_SLOWER_TESTS` in `main_test.cpp` - implement as a command line option

## Cgt Solver Talk for Portugal
- check old talk from BIRS
- intro - math cgt vs cs cgt:
    - math: prove theorems about class of games
    - cs: efficiently compute result for a specific game
    - cs for math: explore, try to find patterns, verify conjectures
    - math for cs: improve efficiency of computation, simplify, shortcuts
- goals of solver:
    - general, efficient
    - avoid canonical form
    - use sum game structure
    - get more from cgt-search writeup

