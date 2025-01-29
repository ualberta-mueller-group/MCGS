# BUGS
- none known

# Current tasks

## V1.0 (In progress)
- V1 github release
    - license?
- Polish, document, test until Jan 25/31
- Jan 28 public release on github?
- Talk on Feb 1, announce V1 to world

## Next steps? (V1.1? Multiple V1.X?)
Tentative next steps/general improvements. TODO: this may need to be broken down into more versions, and expanded

### High priority (Probably important for V1.1):
- Add code linter
    - Resolve linter errors
    - Summarize our style guide in some document somewhere?

- Use a proper unit testing framework?
    - Easier to change this now rather than later

- Game simplification rules: see development-notes.md

### Medium priority (Important/good to have before V2):
- Test framework improvements
    - More detailed timeouts
        - Per file? Per test?
    - Sort rows by column value (ascending/descending)
    - Look into improving table performance for large data sets
        - Bottleneck is UI reflow/repaint, not search time
        - May need to paginate results, which may be undesired. Could also disable automatic update and add "Search" button
            - Do both in some way?

- Write more .test files
    - Interesting cases (i.e. "BBW"^N Clobber, integer-like NoGo, other conjectures)
    - Clobber: from class in `~/Projects/ualberta-mueller-group/combinatorial_game_solver/PriorWork`
    - Random test generation
        - Totally independent .py tool? Or partly built into MCGS?
            - Could add "random_game()" function to game class?
                - Size parameter? random_game(size)
        - Use other solvers to validate results with "adapter" functions/scripts
            - clobber_1xn: Taylor Clobber solver
            - nogo_1xn: Henry NoGo solver
            - elephants: CGSuite
            - simple games (integer_game, dyadic_rational, switch_game, up_star): CGSuite
            - nimber: use nim sum
            - Small sums of these: CGSuite
        - Use MCGS to validate games (i.e. feed it a game as a CLI arg along with "--dry-run", check if it crashed)
    - Some way to scale tests? Just generate new batches of increasingly large random tests?
        - Other solvers may be unable to verify these (i.e. CGSuite)

- Resolve code problems
    - sumgame ownership of games
    - Consider alternatives to `move` being an int?
    - Some code comments should be split between the code and development-notes.md
    - Ugly/confusing std::optional usage
        - i.e. split_result() vs split_result(vector<game*>()), 1st has no value, 2nd has a value
            - Try to make this both clearer AND less verbose
    - "Result" refers to solve() function's return value, but "Outcome" is still used in a few places (mostly variable names)
    - Add missing unit tests
        - NoGo: add unit tests for move generator

- Performance profiling for older solvers, and MCGS?
    - Quick and dirty method just to get intuition and some basic data
    - Good to know how much time is spent doing transposition table lookups, DB lookups, etc, before designing these
        - i.e. if in-memory DB lookups already dominate the run time, then adding disk reads may be especially costly

- Database components and utilities as per development-notes.md, V1.2

### Low priority (Consider doing if not behind):
- Improve MCGS CLI args
    - Some arg combinations don't make sense and should maybe throw or print a warning?
        - i.e. "--test-timeout" without "--run-tests"
    - Arg parsing loop could be cleaned up/abstracted a bit
    - Unify "--help" page styles across scripts/executables

# Future tasks
## V2 and beyond
- Databases
    - "Hierarchical hash buckets" default case?
    - Only in memory, or dynamic loading of "chunks" from disk?
    - DB diff tool
    - Add "DB lookup" command to input language
- Transposition table
    - "Random seed" Henry mentioned (each game has random data added to hash)?
        - Related to using game ID to modify hash
    - Replacement policy?
- Search heuristics
    - Iterative deepening approach from Clobber solver?
    - Heuristic functions?
        - Opponent's number of moves (as in Clobber solver)?

# Current discussion topics
- todo.md organization 
    - "Tasks" and "Discussion topics" sections?
        - "Current"/"Future" prefixes?
    - What does each version denote?
- Discuss immediate next steps
    - Add simplification rules to next steps as per development-notes.md?
    - Tentative "Next steps" stuff listed above. Are these all important right now? Should some be deferred/skipped?
        - Use a proper unit test framework, maybe https://github.com/siu/minunit
    - What discussion topics should be deferred, if any? (There's a lot of stuff in this section)

## General design questions
- Should we allow fractions in `switch_game`? e.g. 1/2 | -3/4
- Rename `x_1xn` to `x_strip` ?
- Rename `x_game` to `game_x`? E.g. `integer_game` to `game_integer` ?
- Scaling experiments, scaling test suites, e.g. scale size, scale number of subgames
- Make board implementations (char, int, bitset, list?) separate from game classes, with common interface - allow composition of different board implementations with game mechanics
- `cgt_game` class - define game by left+right options, read from string
- `rule_set` class as in CGSuite?

## Search features
- Search stats: node count, leaf count, time, depth
    - Later: transposition hits, simplifications, zero removal, inverse removal
- Simplify() hook for game
    - Simplify nim - remove equal pairs
- Move ordering hook for game
    - Move ordering in nim? match other game value?
- Replace a game (e.g. clobber position) by an equal game of simpler type
    - When we learn equality after evaluation or database lookup
    - Who manages the memory of the new and old games?

# Future discussion topics

## Database/next version stuff
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

# Publications
- Clobber paper, based on 701 report
    - Where to publish?
    - What work needs to be done to turn report into paper?
- New paper with MCGS design and results
    - What goals? Match game-specific performance in 1xn Clobber, Nogo? 
    - Get good performance on 2-D boards?
    
# Code to do/finish for Martin
- New base class `impartial_game`
    - Knowledge of nimbers
    - In future: specialised search algorithms
        - Mex rule
        - Lemoine and Viennot, Nimbers are inevitable (2012)
- From email:
A few other things that I was planning to tackle myself, but we can also discuss them:
    - Document sumgame and implementation choices.
    - Semantics of game-in-sumgame. Does the sumgame become the owner (e.g. with unique_ptr)? Or should it copy the game?
    - Can we have multiple references to the same subgame in a sum? Probably not a good idea, then we should guard against that. 
        - E.g. sumgame s; game(of some sort) g; s.add(&g); s.add(&g); (add twice)
        - Write test cases and documentation for these.

# Resolved?
Things in this section can probably be deleted

## Nim
- random testing for nim 
    - increase size limit as program becomes better
    - generate 2nd player win game by adding nim sum

## To discuss
- Testing and documentation
    - coding style, "simple C++"
    - Use Google coding style document?
        - https://google.github.io/styleguide/cppguide.html
        - I looked at it several years back, and it was good then. 
        Look at it again?
    - documentation style. For now it is somewhat minimal
- What are things we can adapt from previous clobber, Nogo solvers? What is general, what is game-specific?

## V1 sumgame design questions
- Should handle type conversion from game to option within a sum game

## V1 todo other
- rewrite `unused/nim` to use sumgame and nimbers classes
- rewrite `unused/nim_test` and `unused/nim_random_test`

## Code to do/finish for Martin
- New nim implementation with sumgame and nimber
- Re-use parts of old nim code
    - read nim sums from file
    - unit test cases
- Remove old nim code

## Todo Design and Naming Issues
- Should zero be its own type??

## Design questions
- play() can change the type of game
    - How to handle?
    - What if a game such as clobber is equal to a simpler game such as up
- `alternating_move_game` add constructor without game; 
    - Add a `set_game` and assert there is a game before solving.

