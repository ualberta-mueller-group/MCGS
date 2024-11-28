# MCGS

A **M**inimax-based **C**ombinatorial **G**ame **S**olver

Martin Müller and Henry Du, 2024

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

## How to implement a new game
- Also see `nim` and `clobber_1xn` as examples
- For game name `x`:
- Create 4 files: `x.h, x.cpp, test/x_test.h, test/x_test.cpp`
    - Define `class x` in `x.h`, derive from `game` or `strip`
    - Each new game must implement at least 3 virtual methods: 
    `play, undo_move, create_move_generator`
    - Class `x_move_generator` - I have made the move generators private, only in the `x.cpp` files. The only access is through the game's `create_move_generator` method
- In `x_test.cpp`, write a function `x_test_all` to call all unit tests for your game. 
    - Add the declaration in `x_test.h` 
    - Call `x_test_all` from `test/test_main.cpp`

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

## Some Implementation Notes
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
- Nim: `nim` implementation done
- Utility class `strip` for 1xn boards
- Clobber on a strip: `clobber_1xn`
- Basic minimax implementation
- Basic test cases in files, run automatically
- Nogo on a strip: `nogo_1xn` class
- Simple game classes: integer, dyadic rational, up-star, switch, nimber

### Version 1 in progress
- Done so far:
    - minimalistic `sumgame` class
    - first experiments with sums of simple games, Clobber and NoGo
    - Current limitation: games need to be defined beforehand, cannot create/delete new games on the fly

- Plan for early steps:
    - rewrite `nim` to use sumgame and nimber
    - change read from string functions to directly create sumgame

#### General improvements in Version 1 - changes not specifically related to sumgame

- implement game::inverse() for all game types
- created classes `impartial_game` and `impartial_sumgame`, moved some funcxtionality from obsolete `nim` class here
    
## Design Choices and Remaining Uglinesses
#### A `move` must be an `int` 
- I tried to make a generic abstract move class, but could not implement it in a "nice" and efficient way.
- There are some utilities in `cgt_move.h` which help pack and unpack moves from/to int
- Plan: probably keep it this way unless I find an elegant general solution
- Move stored in game's move stack always includes a "color bit"
- utilities to deal with color bit and "rest" of move
- Rest of move (after the color bit) can be further broken up
    - Two smaller integers (first one signed, second one unsigned)
    - A sign bit for the first part of two part move
    - Utilities to encode and decode move from/to color, 
    and two parts including sign bit

#### `move_generator` objects are dynamically allocated
- This is ugly but I could not solve it in a better way. 
- I would love to have move generators just as local variables.
- A workaround to prevent memory leaks is to always wrap 
a move generator in a `std::unique_ptr` 
    - Example in `nim_test.cpp`, 
    function `nim_move_generator_test_1`
    - Example in `alternating_move_game::solve`

