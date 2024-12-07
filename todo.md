# BUGS
- none known

# V1 todo

## To discuss with Taylor
- any questions on code, or document?
- next steps:
    - github repo, full access
    - what to work on this week?
    - meet Henry? Define agenda first, or just chat?
    - Weekly meeting - when?
- from email: For me, a few high priority things 
that you could work on are:
    - extending the sumgame class to support changes after a move, 
    i.e. split game into two subgames, or change the type of game.
    - design and implementation of database
    - design and implementation of hash codes and transposition table
    - performance tests and test cases. Tests with time limits.
    - I think you should just work on one at a time, in a git branch, and then merge it back in when completed.

- recent changes by Martin: 
    - const interface for solve()
    - check that game is restored after search - first simple implementation
- Admin things? Work from home regulations?
- Testing and documentation
    - unit tests
    - coding style, "simple C++"
    - Use Google coding style document?
        - https://google.github.io/styleguide/cppguide.html
        - I looked at it several years back, and it was good then. 
        Look at it again?
    - documentation style. For now it is somewhat minimal
- What are things we can adapt from previous clobber, Nogo solvers? What is general, what is game-specific?

### Code to do for Taylor
- what to do first? DB? hashing? small things from clobber solver?
- design small complete units to code+test
- review in next meeting after it is ready

### Design questions to discuss
- hashing for sum games
    - in general, similar to Taylor's and Henry's approach
        - main problem: mixing games, or game + simple abstract values
    - game-specific hash functions?
    - game ID decides which hash to use?
- database
    - internal and external database format
        - general design? game-specific?
    
- "Value scales" and bounds on game values
    - E.g. clobber: multiples of up, up-star
    - binary search to find confusion interval
- simplification of sums
    - mostly, this will need the DB first
    
### Publications
- Clobber paper, based on 701 report
    - Where to publish?
    - What work needs to be done to turn report into paper?
- New paper with MCGS design and results
    - What goals? Match game-specific performance in 1xn Clobber, Nogo? 
    - Get good performance on 2-D boards?
    
### Code to do/finish for Martin
- New nim implementation with sumgame and nimber
- new base class `impartial_game`
    - knowledge of nimbers
    - In future: specialised search algorithms
        - Mex rule
        - Lemoine and Viennot, Nimbers are inevitable (2012)
- Re-use parts of old nim code
    - read nim sums from file
    - unit test cases
- Remove old nim code
- from email:
A few other things that I was planning to tackle myself, but we can also discuss them:
    - document sumgame and implementation choices.
    - Semantics of game-in-sumgame. Does the sumgame become the owner (e.g. with unique_ptr)? Or should it copy the game?
    - Can we have multiple references to the same subgame in a sum? Probably not a good idea, then we should guard against that. 
        - E.g. sumgame s; game(of some sort) g; s.add(&g); s.add(&g); (add twice)
        - write test cases and documentation for these.

## V1 sumgame design questions
- define goals for first, minimalistic sumgame class
- how to implement play() that can split a game into 0,1,2,... subgames
    - option 1: approach in Martin's old clobber code
        - sumgame has list of active and inactive subgames
        - to play in subgame `G_i`
        - play returns list of 0 or more new subgames
        - old subgame `G_i` is always deactivated
        - all new subgames are created as active, and appended to sum
    - option 2:
        - similar, but allow `G_i` itself to be modified, and included in result of play(). In this case, `G_i` would stay active. This makes sense e.g. when splits into two or more subgames are rare.
    - option 3: allow play() to directly modify sumgame, i.e. deactivate, ad new subgames. Probably not a good idea for design/encapsulation
- Should handle type conversion from game to option within a sum game
    - new subgames can include a new type of game
        - e.g. play in a `switch_game` or `dyadic_rational` 
        can create an integer

## V1 todo other
## V1 todo other
- rewrite `unused/nim` to use sumgame and nimbers classes
- rewrite `unused/nim_test` and `unused/nim_random_test`
- replace a game (e.g. clobber position) by an equal game of simpler type
    - when we learn equality after evaluation or database lookup
    - who manages the memory of the new and old games?

# Todo Design and Naming Issues
- rename `x_1xn` to `x_strip` ?
- play() can change the type of game
    - how to handle?
    - what if a game such as clobber is equal to a simpler game such as up
    - should zero be its own type??
- `alternating_move_game` add constructor without game; 
    - add a `set_game` and assert there is a game before solving.
    - remove `empty_game` in `sumgame`
    - remove old `nim` implementation once we have a sum of nimbers replacement for it

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
- should we allow fractions in `switch_game`? e.g. 1/2 | -3/4

## Todo - Tests

### sum games
- A test game on file can be a sum, enclosed in quotes. 
- modify reading to build a sum

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

