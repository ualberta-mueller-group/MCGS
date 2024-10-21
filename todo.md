# BUGS
- `switch_game` is incomplete, cannot play out integers

# V0 todo
- review, finish in-file comments, headers
- Tag version 0

# V1 todo
- define goals for first, minimalistic sumgame class
- rewrite nim to use sumgame and nimbers classes

# Design and Naming Issues
- rename `x_1xn` to `x_strip` ?
- play() can change the type of game
    - how to handle?
    - e.g. play in `switch_game` leads to a number
    - e.g. some dyadic rationals are integers
    - what if a game such as clobber is equal to a simpler game such as up
        - what if we do not know it at first, 
        but learn it after evaluation?
        - what if we have those games in a database?
    - should zero be its own type??
    - Can handle in V1 with sum game - move creates list of new subgames,
        can include a new type of game
    - Could do even in V0 if game::play() returns a new game. How
        to manage memory?
# TODO
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

## TODO TESTS
- `test/cgt_basics_test`

### Nogo
- Nogo - add test cases for move generator
- Nogo - add test cases in text file, reading/running them
- get, convert existing NoGo tests

### Nim
- random testing for nim 
    - increase size limit as program becomes better
    - generate 2nd player win game by adding nim sum
- add random in general? how to check results? 
- size parameter for random games? `game.generate_random(size)`?

### Clobber
- get, convert existing small clobber tests
- From class in `~/Projects/ualberta-mueller-group/combinatorial_game_solver/PriorWork`
- From Taylor's solver 

## Test Cases Implementation
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

