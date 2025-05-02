# BUGS
- none known

# Limitations and Annoyances
- `alternating_move_game` should complain if the game is "splittable", since it can only handle a single game that does not split

# User Comments and Notes
Suggestions from audience of talk given at CGTC, or from MCGS users

- implement general graph structure not just `strip`. E.g. play col or snort on an arbitrary graph, or on "triangular" graphs
    - col/snort: "Lessons in Play" p.313

- check: does CGSuite OutcomeClass computation also rely on computing canonical form?

- After `solve`, output a winning strategy
    - Have a player that can play through a proof.
    - Needs to handle all simplifications
        - plays in pruned subgame
        - plays in game that has been simplified in the solver
        - need to keep a kind of parallel structure of the proven game and the real game
            - play in pruned G + (-G), follow mirror strategy
            - find and follow "at least as good" move when G has been simplified to G'
            - similary when simple games have been combined, e.g. numbers have been added up, and opponent plays in some specific fraction
            - Issue: optimal vs good enough play
                - Solver can stop at a win, even if the winning move is not "optimal"

# Current tasks

# Future tasks
## V1.2
### V1.2 done
- hashing for games and sum games
- `grid` and `nogo` on a grid classes
- impartial games and simple impartial game solver
- kayles, a simple impartial game
- Impartial game wrapper to play impartial version of any `game`
- transposition table

### V1.2 to do
- cleanup impartial games

## V1.3
- transposition table
    - design documentation?
    - compare to discussion in design document
    - Replacement policy?
- performance testing
    - with/without table
    - How close to our special purpose solvers?

## Possible steps for Versions 1.X
- Important or good to have before V2
- Medium priority

### Move ordering heuristics
- Can we define some in a game-independent way?
- Play in the middle heuristic
    - simple hack: generate all moves, create list,
      and pick from list starting in the middle
- sort by game size heuristic (already used implicitly in sumgame?)
- any heuristics for impartial games?
    - the move generator for impartial game wrapper generates all moves for black first, using the underlying game generator, followed by all moves for white.
        - If there is a good game-specific move generator, then this is inefficient. It will create all bad black moves before any good white move
        - choose moves alternatingly for black and white instead
    - Problem: a good move in the underlying game may not be a good move in the impartial version

### Use a proper unit testing framework?
- Easier to change this now rather than later
- Could possibly show code coverage and number of tests run
- Possible option: MinUnit `https://github.com/siu/minunit`
- Possible option: GoogleTest `https://google.github.io/googletest`

### Test framework improvements
- More detailed timeouts
    - Per file? Per test?
- Sort rows by column value (ascending/descending)
- Look into improving table performance for large data sets
    - Bottleneck is UI reflow/repaint, not search time
    - May need to paginate results, which may be undesired. Could also disable automatic update and add "Search" button
        - Do both in some way?

### Write more .test files
- Interesting cases (i.e. "BBW"^N Clobber, integer-like NoGo, other conjectures)
- Clobber: from graduate course in `~/Projects/ualberta-mueller-group/combinatorial_game_solver/PriorWork`
- Random test generation
    - Totally independent .py tool? Or partly built into MCGS?
        - Could add `random_game()` function to game class?
            - Size parameter? `random_game(size)`
    - Use other solvers to validate results with "adapter" functions/scripts
        - `clobber_1xn`: Taylor Clobber solver
        - `nogo_1xn`: Henry NoGo solver
        - `elephants`: CGSuite
        - simple games (`integer_game, dyadic_rational, switch_game, up_star`): CGSuite
        - Small sums of these: CGSuite
    - Use MCGS to validate game text representations
        - feed it a game as a CLI arg along with "--dry-run", check if it crashed
- Some way to scale tests? Just generate new batches of increasingly large random tests?
    - Other solvers may be unable to verify these (i.e. CGSuite)

### Ownership of games in `sumgame`
- Decide and document semantics of game-in-sumgame. 
- Current status: the caller who adds a game g to a sum s 
  owns g. New games that are created by `split()` are owned by s.
- One suggestion: The sumgame becomes the owner. Make this explicit e.g. with `unique_ptr`. 
- Another alternative: always copy the game
- Possible bug: can we have multiple references to the same subgame in a sum? Probably not a good idea, then we should guard against that. 
    - Example `sumgame s; game g; s.add(&g); s.add(&g);`
        - currently there is an `assert` protecting against adding twice
    - Write test cases and documentation for these.

### Resolve Code Problems

- Consider alternatives to `move` being an int?
    - Pass around pointers to heap-allocated moves, whose actual types
    are defined by each game?
    - Have an interface type for storing "to play"
- Some code comments should be split between the code and `development-notes.md`
- Ugly/confusing `std::optional` usage
    - i.e. `split_result()` vs `split_result(vector<game*>())`, 1st has no value, 2nd has a value
    - Try to make this both clearer AND less verbose
- "Result" refers to `solve()` function's return value, but "Outcome" is still used in a few places (mostly variable names)
- Add missing unit tests
    - NoGo: add unit tests for move generator
    - Do a systematic check

- Performance profiling for older solvers, and MCGS
    - Quick and dirty method just to get intuition and some basic data
    - Good to know how much time is spent doing transposition table lookups, DB lookups, etc, before designing these
        - i.e. if in-memory DB lookups already dominate the run time, then adding disk reads may be especially costly

### Lower priority tasks, may push to after Version 2
- Improve MCGS CLI args
    - Some arg combinations don't make sense and should maybe throw or print a warning?
        - i.e. "--test-timeout" without "--run-tests"
    - Arg parsing loop could be cleaned up/abstracted a bit
    - Unify "--help" page styles across scripts/executables
- Rename `x_1xn` to `x_strip` ?
- Rename `x_game` to `game_x`? E.g. `integer_game` to `game_integer` ?


## Version 2 and beyond
- Grab a CGT seminar spot to talk about MCGS and give a demo
- Databases
    - "Hierarchical hash buckets" default case?
    - Only in memory, or dynamic loading of "chunks" from disk?
    - DB diff tool
    - Add "DB lookup" command to input language

## Beyond Version 2
- Search heuristics
    - Iterative deepening approach from Clobber solver?
    - Heuristic functions?
        - Opponent's number of moves (as in Clobber solver)?
- Computational cost model
    - Can help determine when it may be beneficial to compute bounds, or other information,
    during search


## General design questions
- Scaling experiments, scaling test suites, e.g. scale size, scale number of subgames
- Make board implementations (char, int, bitset, list?) separate from game classes, with common interface - allow composition of different board implementations with game mechanics
- `cgt_game` class - define game by left+right options, read from string
- `rule_set` class as in CGSuite?
- Should zero be its own type??
- Should we have "type info" for each game type, 
  such as is splittable, is impartial, etc. Right now some 
  of this information is expressed through inheritance, 
  but e.g. splittable cannot easily be done that way.
    - Why splittable? See note on `alternating_move_game`.
      There are three cases: 
        1. games that never split, such as nimber, integer
        2. Games that must split, e.g. kayles
        3. Games that work without split, but are probably 
           less efficient. Nogo, Clobber?

## Search features
- Search stats: node count, leaf count, time, depth
    - Later: transposition hits, simplifications, zero removal, inverse removal
- `simplify()` hook for game
    - use cases? Use inverse()?
    - Will become really important after we have database
- Move ordering hook for game
    - Move ordering in nim? match other game value?
- Replace a game (e.g. clobber position) by an equal game of simpler type
    - When we learn equality after evaluation or database lookup
    - Who manages the memory of the new and old games?

# Future discussion topics
## Database/next version
- What can be reused from previous solvers? What's game-specific?
- Hashing for sum games
    - In general, similar to Taylor's and Henry's approach
        - Main problem: mixing games, or game + simple abstract values
    - Game-specific hash functions?
    - Game ID decides which hash to use?
- Simplification of sums
    - Mostly, this will need the DB first
- Database
    - Internal and external database format
        - General design? game-specific?
- "Value scales" and bounds on game values
    - E.g. clobber: multiples of up, up-star
    - Binary search to find confusion interval

## Publications
- Clobber paper, based on Taylor's 701 report
    - Where to publish?
    - What work needs to be done to turn report into paper?
- New paper with MCGS design and results
    - What goals? Match game-specific performance in 1xn Clobber, Nogo? 
    - Get good performance on 2-D boards?
    
## More Impartial Games Support

- Small todo's
    - Remove duplication between performance and unit tests:
      Move tables of expected results into new header file.
    - performance tests should have "impartial" in the name
- Search algorithms
    - Lemoine and Viennot, Nimbers are inevitable (2012)
    - Also see, compare with 2022 course report
    "Impartial Clobber Solver" by Dai and Chen

- Random testing for nim sums 
    - increase size limit as program becomes better
    - generate 2nd player win game by adding nim sum

