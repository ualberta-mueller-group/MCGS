# BUGS
- `clobber_1xn_move_generator` does not encode toplay in move
- need test cases that play and undo two black moves in a row, two white moves in a row
- how to deal with negative integers in `two_part_game`, e.g. for integers, downs, and fractions

# DESIGN ISSUES
- play() can change the type of game
    - how to handle?
    - e.g. play in `switch_game` leads to a number
    - e.g. some dyadic rationals are integers
    - what if a game such as clobber is equal to a simpler game such as up
        - what if we do not know it at first, but learn it after evaluation?
    - should zero be its own type??
- how to deal with signed integers in two part games? sign bit?

# TODO
- simplify() hook for game
    - simplify nim - remove equal pairs
- move ordering hook for game
    - move ordering in nim? match other game value?
- search stats: node count, leaf count, time, depth
    - later: transposition hits, simplifications, zero removal, inverse removal
- use a proper unit test framework, maybe https://github.com/siu/minunit

- random testing for nim 
    - increase size limit as program becomes better
    - generate 2nd player win game by adding nim sum
- add random in general? how to check results? size parameter for random games?
game.generate_random(size)?

- scaling experiments, scaling test suites, e.g. scale size, scale number of subgames
- make board implementations (char, int, bitset, list?) separate from game classes, with common interface - allow composition of different board implementations with game mechanics
- `rule_set` class as in CGSuite?

## TODO TESTS
- `test/cgt_basics_test`
# DONE
- in game:
add to move stack
- change toplay
- everyone must call game::play()
- clobber board conversion char `x,o,.` vs board as `EMPTY, BLACK, WHITE`
- `move_generator clobber_1xn`
- clobber unit tests
    - move generator, solve, convert from/to string
- test write clobber board
- move generators are memory leaks - use `std::unique_ptr`
- add nim formula
- nim move generator could go into cpp. Only needed by `nim_test` - it could - just use the generic movegenerator interface?
- add random testing for nim
- nim static solver tests


## TEST CASES
- get, convert existing small clobber and NoGo tests
- should just read one by one, no need for vector. write an iterator???

- From class in `~/Projects/ualberta-mueller-group/combinatorial_game_solver/PriorWork`

- From Taylor's solver 

## TALK
- cgt solver talk
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

