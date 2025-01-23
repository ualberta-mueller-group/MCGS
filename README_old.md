# MCGS

A **M**inimax-based **C**ombinatorial **G**ame **S**olver

Taylor Folkersen, Martin Müller and Henry Du, 2024

## Design Documentation
For the overall approach and future plans, see the document "The Design of MCGS:
A Minimax-based Combinatorial Game Solver".

## How to build, run and test MCGS
- Download the code and go to the directory
- There is a basic makefile. It supports:
    - `make` builds the program `.\MCGS`. Any tasks you want to run should be called in the main function.
    - `make test` builds and runs all unit tests, in program `./MCGS_test`. No output means that the tests succeeded.

## How the MCGS code is organised
- Currently it uses a "flat" organisation. All game-independent 
codes and all game-specific implementation files are in the `MCGS` directory.  The two subdirectories are:
    - `main` contains a single file, the main program `main.cpp`
    - `test` contains all testing-related code:
        - All unit tests. Each unit test consists of a `x_test.h` file which exports a single function `x_test_all`, and a file `x_test.cpp` with all tests for a file `x.cpp`.
        - the `main_test.cpp` program which calls all 
        - Helper functions for writing tests in `test_case.h`, `test_case.cpp` and `test_utilities.h`
    
- There are two text/markdown files: this `README.md`, and a `todo.md`

## Main data types of MCGS

### `move`
The abstract type `move` is used in all (sub-)games to represent a move in that particular subgame. Also compare with `sumgame_move`
- Implementation: A `move` is typedef as an `int`.
- Each game is free to define its own representation of a move.
- The `move` representation must include space for one "color bit"
- Utilities in `cgt_move.h`
    - help pack and unpack moves from/to int
    - deal with color bit and "rest" of move
    - Rest of move, after the color bit, can be further broken up
    - Two smaller integers (first one signed, second one unsigned)
    - A sign bit for the first part of the two part move
    - Utilities encode and decode move from/to color, 
    and two integer parts including the sign bit
Comments and TODOs:
- TODO: should guarantee that a move is at least 32 bit
- Also see comments about `move` under "Design Choices"

### `game`
A `game` is the base type for all combinatorial games supported by MCGS. 
Its main use is as one subgame in a sum. However, it can also be solved as a
standalone game in combination with `alternating_move_game`.
- Each `move` stored in the move stack of `game` includes the "color bit"

#### `sumgame_move` struct
Represents a move made in a `sumgame`
- contains a subgame index and the move made in the subgame
- The index is into the vector `sumgame::_subgames`
- The `move` is for the subgame stored there

#### `sumgame` class
A `sumgame` represents a (possibly empty) set of subgames. 
It derives from `alternating_move_game` and reimplements the
`solve` method to take advantage of sum structure
- Main data structure: `vector<game*> _subgames`
    - TODO sumgame should be owner of these games? Use `std::unique_ptr`
    - TODO copy on add?
- derived from `alternating_move_game` but reimplements solve 
    - it uses `sumgame_move`
    - keeps its own `_sumgame_move_stack` with sum-level info; subgames keep their own stacks as well
## How to implement a new game
- Also see `nim` and `clobber_1xn` as examples
- For game name `x`:
- Create 4 files: `x.h, x.cpp, test/x_test.h, test/x_test.cpp`
    - Define `class x` in `x.h`, derive from `game` or `strip`
    - Each new game must implement at least 3 virtual methods: 
    `play, undo_move, create_move_generator` (two more now - see below)
    - Class `x_move_generator` - I have made the move generators private, only in the `x.cpp` files. The only access is through the game's `create_move_generator` method
- In `x_test.cpp`, write a function `x_test_all` to call all unit tests for your game. 
    - Add the declaration in `x_test.h` 
    - Call `x_test_all` from `test/test_main.cpp`

## Search and solving a game
- Two classes implement game solving: `alternating_move_game`
and `sumgame`
- `alternating_move_game` is used for solving a single game
    - `alternating_move_game::solve` is a basic boolean negamax search
- `sumgame` is used to store and solve a sum of games. 
It is derived from `alternating_move_game`.
    - `sumgame::solve` is a basic boolean negamax search for sums
    - enforced const-ness of games when calling solver
        - game state may modify during solve but is restored 
        at the end in any case
        - a checking function to make sure state is unchanged
            - a naive first implementation just checks length of move stack
            - TODO it probably is broken for sumgame, since it uses a different stack
            - to be replaced by a full hash


## File Format for Test Cases
- simple file format for tests:
- line 1: game name, file format version, currently 0
- line 2..n:
    - one test per line
    - format of test: game toPlay result
        - game: a single string representing the game
        - toPlay: B or W
        - result: win or loss
- Example:
<pre>
clobber_1xn 0
XO B win
XO W win
OXOXOX B loss
OXOXOX W loss
XXO B win
XXO W loss
</pre>
- A test game can be a sum, enclosed in quotes. 
It is read with `std::quoted`

## Implementation Notes for extending the `game` class
- virtual methods in game that must be implemented:
    - `play`, `undo_move`, `create_move_generator`, `print`, `inverse`
- In every game implementation:
    - `x::play()` must call `game::play()`
    - `x::undo_move()` must call `game::undo_move()`
- Move generators are accessible only through game `create_move_generator`
    - They are dynamically allocated - wrap each use in a `std::unique_ptr`
    - Example: `alternating_move_game::solve`
    - A game-specific move generator is declared and used only in `x.cpp`, not in a header file
- Game unit tests should cover at least:
    - `play` and `undo_move`
    - `solve` for both black and white
    - Convert from/to string
    - Write test cases in file, read and solve
    - Game-specific move generator
        - Count number of moves and details of moves generated
- `play()` may not assume alternating colors, since games can be subgames 
in a sum. 
- The color of the player is encoded as part of the move 
and is stored in the move stack. 
- `undo_move` must respect and use the move player color information.

## Versions
### Version 0 completed
- `move` and `game` classes, `alternating_move_game`
- Nim: `nim` implementation done
- Utility class `strip` for 1xn boards
- Clobber on a strip: `clobber_1xn`
- Basic minimax implementation in `alternating_move_game::solve`
- Basic test cases in files, run automatically
- Nogo on a strip: `nogo_1xn` class
- Simple game classes: integer, dyadic rational, up-star, switch, nimber

### Version 1 in progress

#### Version 1 completed
- `sumgame` class
    - game-dependent `split` into 0,1,2 or more subgames after a move
    - supports both keeping the old `game`, and replacing it after a `split`
    - supports changes of game type, such as `switch_game` to `integer_game`
- first experiments with sums of simple games, Clobber and NoGo
- Current limitation: games need to be defined beforehand, cannot create/delete new games on the fly

#### Version 1 completed - General improvements not specifically related to sumgame
- implemented game::inverse() for all game types

#### Version 1 in progress / to do
- performance testing framework
    - performance tests
    - compare with/without subgame split
    - other options to compare?
- change read from string functions to directly create sumgame
- created classes `impartial_game` and `impartial_sumgame`
    - moved some functionality from obsolete `nim` class here
    - TODO rewrite all `nim` tests to use `sumgame` and `nimber`
    - TODO remove `nim` class after converting tests

### Future: Smaller step Versions 1.x , prepare for Version 2

#### Version 1.1 - simplification rules for `game` and `sumgame`
    - simplify `game` G in `sumgame` S
        - compare G with a simpler game H, replace in S if equal
        - compare G1+G2 with a simpler H, replace in S if equal
        - simplify games of same type in S, e.g.
            - add up integers/rationals
            - add ups+stars
            - add nimbers
        - general sum simplifications
            - remove/deactivate 0
            - find inverse pairs and deactivate

#### Version 1.2 - Tools and Components for database
- `scale` such as multiples of up, or up+star, or integers
    - binary search to find upper/lower bounds for a game G on scale S

## Design Choices and Remaining Uglinesses
#### A `move` must be an `int` 
- I tried to make a generic abstract move class, but could not implement it in a "nice" and efficient way.
- Plan: probably keep it this way unless I find an elegant general solution

#### `move_generator` objects are dynamically allocated
- This is ugly but I could not solve it in a better way. 
- I would love to have move generators just as local variables.
- A workaround to prevent memory leaks is to always wrap 
a move generator in a `std::unique_ptr` 
    - Example in `nim_test.cpp`, 
    function `nim_move_generator_test_1`
    - Example in `alternating_move_game::solve`

#### Reimplementation/duplication of `game` concepts in `sumgame`
- This is a consequence of - A `move` must be an `int`
- A move in a sumgame is specified in `struct sumgame_move` by two parts: index of subgame, and move inside the subgame
- so play() in sumgame takes a `sumgame_move` as argument, not a `move`
- solve() also rewritten to use `sumgame_move`
- `alternating_move_game` currently requires a game 
argument - a ugly dummy game `empty_game`. See todo.md.
