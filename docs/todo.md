# BUGS
- none known

# Limitations and Annoyances
- `alternating_move_game` should complain if the game is "splittable", since it can only handle a single game that does not split

# User Comments and Notes
Suggestions from audience of talk given at CGTC, or from MCGS users

- implement general graph structure not just `strip`. E.g. play col or snort on an arbitrary graph, or on "triangular" graphs
    - col/snort: "Lessons in Play" p.313

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


# Remaining Issues from previous versions

## Issue management
- There are five issues on github in https://github.com/ualberta-mueller-group/MCGS/issues - started June 2025, never updated. Close those?
- How should we manage issues? In this document?

## Remaining Issues from V1.5
- try to alternate black/white moves in impartial generator
- try storing both nimbers and boolean values in tt
- get rid of `_nim_value` in impartial game object, just use tt?
- use database for impartial solvers
    - test performance

## Remaining Issues from V1.4
- transposition table
    - Replacement policy?
- performance testing
    - How close to our special purpose solvers?
- database
    - design remaining DB features
    - plan remaining DB implementation steps
    - "Hierarchical hash buckets" default case?
    - DB diff tool?
    - Add "DB lookup" command to input language?
- gather other solvers into one repository
    - SBHSolver needs some slight modification
    - use CGSuite for impartial 2D nogo

## Remaining Issues from merging impartial-games branch into v1.2-develop
- Needs review: in impartial games, there are still too many "two version" 
  methods and move generators,
  both with and without a color argument.
- Needs review: `impartial_game` stores a `_nim_value` after it is solved.
  Do we need it, or should we make the game object smaller and 
  just use the hash table to store the value?
  It is a bit confusing since a `impartial_game` is an
  object with changable state (can play/undo moves), and
  the `_nim_value_` is only valid for the root state (0 moves played).
  This is checked by assertions, but still questionable design?
- The new `performance_test_...` functions are not called.
    Should we have an interface to call them?

# Upcoming Versions
## February/March 2026: Documentation and Publication
- Slides for CGT Japan
- More complete results page
    - clobber results
    - clobber\_1xn results
    - nogo results
    - nogo\_1xn results

## Version 1.6: focus on more information in DB
- simplest equal game
- thermograph DB
- "Value scales" and bounds on game values
    - E.g. clobber: multiples of up, up-star
    - Binary search to find confusion interval

### Version 1.6 "other"
- sort test cases (semi-)automatically by difficulty
    - Taylor to experiment
- keep the separate "dbg" and "opt" builds, or preferably these plus a few more
    - T: I regularly compile MCGS with AddressSanitizer to check for memory errors

## Version 1.7: integrate thermographs in solving algorithms
- compute TG
- store TG in TT and DB
- use bounds from TG for solving hot games
- Compare with Amazons solver

## Version 1.8: cleanup and performance improvement
- No big new features
- Focus on performance testing, tuning, code review and cleanup



# Other TODO, Unsorted

## User interface and API
- Python interface to MCGS?
- Develop interface to Kyle's Javascript UI?
- Interface for new databases? I.e. view confusion interval, thermograph
- Look into ludii.games - can it interface with MCGS?

## Implement more games
- Col, Snort
- Cannibal clobber. It is like Clobber but you can also eat your own stones. But you can only move with your own pieces.
    - impartial Cannibal Clobber would be one of the simplest possible games. The color of stones does not matter anymore, and winning boils down to the parity of the number of isolated stones at the end. What kind of nimbers do occur? Can the outcome be related to graph properties?

## Port more algorithms to MCGS
- Locally informed...(code from Mueller and Li paper)
- Amazons solver? Compare after we have thermographs?
- Kao's mean and temperature search
    - 2022 student project for the simple case, single move option

## Complexity Score
- Measure complexity of a subgame
- Implemented a hook - `game::complexity_score()` - in Version 1.5
    - equal to `size()` for strip and grid
    - 1 for all other games
- Compare with Complexity Score 1..4 in SEGClobber
- Currently only used in impartial games, LV algorithm, to select the
   last subgame to search
- Can be used to select "simplest" subgame in other circumstances
- Can be used for move ordering within a subgame, with a 1 ply search
- Use `complexity_score` better
    - only used to find "largest" subgame in LV
    - T: Perhaps we could look into using move counts as the complexity score? Not sure what the overhead of generating all moves would be...
    - Results Martin 2025-12-07: 
        - easy: results are mixed. 256 are slower, 266 are faster. Most of the ones with large nimbers are slower though, up to 6x in some cases, but it is not consistent.
        - medium: works well. Speedup between 0-40%, with 30% typical
        - hard: about 2x faster
- would like to use for move ordering, to find well-splitting moves
- T: Maybe we should leave exploration of this with partisan games until later (when we add thermographs?)
- T: I wonder if game-specific heuristics would still work okay when game types are mixed. 
    - add a simple complexity score to the database (just move count?) and  use one of these cheaper heuristics for games not in the database? 
- T: Is there some sensible rule for choosing a most complex game when there are mixed types, and some games are in the database while others aren't?

## Move ordering and heuristics
- Can we define some in a game-independent way?
- Play in the middle heuristic
    - simple hack: generate all moves, create list,
      and pick from list starting in the middle
- sort by game size heuristic (already used implicitly in sumgame?)

### Use bounds on subgames from db
- add upper and lower bounds, use LS and RS from thermographs, considering toplay, as in Amazons paper.

### Almost solved sums
- If all subgames are in the DB, but the sum is not solved (e.g. N+N, L+R, N+opponent), then:
    - Play on the less (least) complex subgame, to try to reduce it to 0 and change the sum to a single game

    - More complicated schemes are possible if we have more than two subgames, all in DB. For example, if we can "almost" solve the sum statically, except for one subgame Gi, then play in Gi and try to get rid of it or change its outcome class.

## Code maintenance, build, C++ issues

- CMake build for MCGS

### Use a proper unit testing framework?
- Easier to change this now rather than later
- Could possibly show code coverage and number of tests run
- Possible option: MinUnit `https://github.com/siu/minunit`
- Possible option: GoogleTest `https://google.github.io/googletest`

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
    - nogo_1xn: add unit tests for move generator
    - Do a systematic check

- Performance profiling for older solvers, and MCGS
    - Quick and dirty method just to get intuition and some basic data
    - Good to know how much time is spent doing transposition table lookups, DB lookups, etc, before designing these
        - i.e. if in-memory DB lookups already dominate the run time, then adding disk reads may be especially costly

### Lower priority tasks, may push to after Version 2
- Improve MCGS CLI args
    - Some arg combinations don't make sense and should maybe throw or print a warning?
        - i.e. "--test-timeout" without "--run-tests"
        - Martin: yes I once wasted 20 minutes trying to figure out why my tests did not run, when I forgot a --run-tests. Either the other test-related flags should silently add the --run-tests, or it should complain.
    - Arg parsing loop could be cleaned up/abstracted a bit
    - Unify "--help" page styles across scripts/executables
- Rename `x_1xn` to `x_strip` ?
- Rename `x_game` to `game_x`? E.g. `integer_game` to `game_integer` ?


## More tasks, unsorted
### Test framework improvements
- More detailed timeouts
    - Per file? Per test?
- Look into improving table performance for large data sets
    - Bottleneck is UI reflow/repaint, not search time
    - May need to paginate results, which may be undesired. Could also disable automatic update and add "Search" button
        - Do both in some way?

### Write more .test files
- Interesting cases (i.e. "BBW"^N Clobber, integer-like NoGo, other conjectures)
- Clobber: from graduate course in `~/Projects/ualberta-mueller-group/combinatorial_game_solver/PriorWork`
- Random test generation
    - Totally independent .py tool? Or partly built into MCGS?
        - We have some scripts for this in the `utils` directory
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
- How to use test results? Standard criteria for when program Version A is better than Version B? Cumulative total number of problems solved? Which time limit?


## Publications and Talks
- Grab a CGT online seminar spot to talk about MCGS and give a demo
- Feb 20, 2026: CG 2026 submission
- Mar 19-22, 2026: CG conference Japan

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

# Search
## Search algorithm
- Try other search algorithms? EWS, proof number search, MCTS-solver
    - Can we dropin-replace the main search engine from minimax to one of those?
    
## Search features

- Search stats: node count, leaf count, time, depth
    - transposition hits, simplifications, zero removal, inverse removal
    - histogram - number of subgames as function of depth

    - Partially complete, mostly just need to expand, and include in csv output
    - Option to use node limit instead of time limit
        - Useful to run multiple tests concurrently
- `simplify()` hook for game
    - use cases? Use inverse()?
    - Will become really important after we have database
- Move ordering hook for game
    - Move ordering in nim? match other game value?
- Replace a game (e.g. clobber position) by an equal game of simpler type
    - When we learn equality after evaluation or database lookup
    - Who manages the memory of the new and old games?

- search single new subgame independently, check whether it is zero
    - very good for sheep?
    - worth doing in other games? which?
    - can also determine L, R, N
    - stop after first search if it is a win? - then it cannot be zero
    - Also worth doing in special circumstances, e.g. if all but one subgames are in db?
        - Compare SEGClobber

## Search Algorithms for Specialised Tasks
- Search to identify zero subgames
    just do one or two searches to see if both lose.
    - In sheep, most balanced subgames (same number of sheep, or both have more sheep than empty cells) are zero.
- To solve a sum, should we first search each subgame individually to check if it is zero?
- For games in general, we could use e.g. statistics from the database to decide if it is worth doing.
- Specialised algorithm to recognise integers?
    - is there anything better than to check if G-n = 0?
    - What to do if we do not know n, but suspect that G is integer?
        - build thermograph
        - binary search for n
- Specialised algorithm to recognise fractions?
    - how? thermograph?
- Specialised algorithm to recognise all small game?
    - build thermograph. stop if any subgame is not all-small
    

# Program Evaluation and Benchmarks
- Evaluate/compare program versions
    - Define standard test sets, run them after each major change
    - Number of problems solved as function of time - standard in planning literature, nice robust measure if we have a good test set.
    - Taylor: separate the data by game type in some way, to see if specific games are more affected by changes

# Future discussion topics TODO reorganise all content by topic
## Databases
- What can be reused from previous solvers? What's game-specific?
- Simplification of sums using DB
- Document internal and external database format
    - General design? game-specific?
- Offer some pre-built databases for download
    - Size limitations in github? Put them locally?
    - Must be re-built with each new version? Script?
    - naming scheme? e.g. `database-clobber-4x5.bin, database-clobber_1xn-20.bin`

## Building Databases
- Support for merging DB that were built separately
    - Or support multiple DB
        - Which one to choose for loopup if they overlap? Does it matter?
- build databases cumulatively? 
    - T: Currently a file must be built from scratch
    - I don't think it should be too difficult to add this if I store some extra metadata in the DB files
- Support more than one rectangle dimension for the same game in one DB
    - Example: `[impartial domineering] max_dims = 5,5; [impartial domineering] max_dims = 2,10;`
    - Motivation: for example, if 5x5 is too large, but say 4x5, 3x7, and 2x10 are all reasonably small DB, then having all those could help

# Small Todo Items
- Remove duplication between performance and unit tests
- Add one-line documentation for these options in `global_options.h`
- Should DB hits also be stored in the TT? Do some tests.
- rename the current `use_complexity_score` flag to make it clear that it is only for the LV algorithm
- https://github.com/ualberta-mueller-group/MCGS/issues - use issues tracker, or delete/resolve

## improve output
- print improvement: `up_star:0* → *`, suppress 0 up
- `up_star`:-1 → 1 down? In html can use uparrow and downarrow, &uarr; &darr
- missing hours in html output conversion?
    - Total time: 22,372,154 ms ＝ 52s, 154ms
# Impartial games
- improved lookup, avoid redundant lookups for LV
    - store nimber as well as boolean for g = *n ?
- re-test complexity_score
    - make it the default for impartial games if it works better

## Algorithms
- Re-read Beling's paper
- Read Beling's implementation, compare
- Trace to find inefficiencies in LV
- Is it useful to limit size of nimbers in search, e.g. search if G in `{*0, *1, ...*n}` with parameter n? E.g. iterate G = *i
    - problem with split, e.g. G = G1+G2 = *0, but e.g. G1 = G2 = *50.

## Impartial Move Generator
- Move generator: alternate Black and White moves.
    - the move generator for impartial game wrapper generates all moves for black first, using the underlying game generator, followed by all moves for white.
        - If there is a good game-specific move generator, then this is inefficient. It will create all bad black moves before any good white move
        - choose moves alternatingly for black and white instead
    - Problem: a good move in the underlying game may not be a good move in the impartial version
- Move generator: finish testing `complexity_score` based one
- Move generator: finish testing `PITM` based one

## Impartial games performance measurement
- measure number of splits?
- How much more effective with DB?

## Impartial Test cases
- Finish restructure
    - still a few impartial game tests outside the new impartial directory, e.g. random boards
    - make structure inside and outside of `autotests` match
- Add more tests - impartial other games, e.g. elephants, new games, amazons

## Impartial Search

### Impartial Search Statistics
- stats for search depth in LV
- add stats for total number of subgames, or number of subgames added by split?
- add stats for play/undo move

# Specific Games

## Abstract game (text input)
- `abstract_game` or `game_abstract` class
- make abstract game from string e.g. "{ 3 | 5}"
- move generator to move to left or right substring?
    - or convert into an internal representation when reading?
    
## Create Game from Options
- Create game from options GL, GR
- Example: test canonical values such as 1|* in Sheep tests
- functions to implement:
    - `make_game(Gl,Gr)` single option
    - `make_game(GL,GR)` sets of options

## Battle sheep
- run MCGS on CMPUT 657 Assignment 1 and 2 test cases
    - compare with student codes?

## Clobber
- Can we do Clobber 6x6?
    - try some 6x6, estimate the solution time
    - try again after improving DB - replace by SEG
    - Try some openings, see how far back we can go.
- avoid most floodfills for split()?
- Where to get high quality games? 
    - Just start with random games?
    - 2022 student project?

## 1xn Clobber
- Study bounds for G = (xo)^n. 0 < G < up?
- compare SEGClobber with MCGS

## Game-specific Small Todo Items
- domineering/cram: fill in single squares after move even if no split happens

# Documentation and Publication
## Web page
- add to results.html page
    - add more partisan game results
        - Nogo
        - Hard clobber positions
        - elephants
        - new games
        - Amazons

- add other impartial games

## Development Notes
- Move some discussion to there from here, where implementation has started
