# MCGS

A **M**inimax-based **C**ombinatorial **G**ame **S**olver

Martin Müller, 2024

## Design Documentation
For the overall approach and future plans, see the document "The Design of MCGS:
A Minimax-based Combinatorial Game Solver".

## How to build, run and test MCGS
- Download the code and go to the directory
- There is a basic makefile. It supports:
    - `make` builds the program `.\MCGS`. Any tasks you want to run should be called in the main function.
    - `make test` builds and runs all unit tests, in program `./MCGS_test`. No output means that the tests succeeded.

## How the MCGS code is organised
- Currently it uses a "flat" organisation. The only subdirectories are:
    - `main` contains the main program
    - `test` contains all unit tests, and the `main_test.cpp` program
- There are two text/markdown files: this `README.md`, and a `todo.md`

## How to implement a new game
- Also see `nim` and `clobber_1xn` as examples
- Say game name is `x`
- Create 4 files: `x.h, x.cpp, x_test.h, x_test.cpp`
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

## Implementation Notes
- every game implementation:
    `x::play()` must call `game::play()`
    `x::undo_move()` must call `game::undo_move()`
- move generators are accessible only through game `create_move_generator`
    - they are dynamically allocated - wrap each use in a `std::unique_ptr`
    - Example: `solve` in `solve.cpp`
    - move generator is declared and used only in `x.cpp`, not in header file
- game unit tests
    - move generator
        - count moves and details
    - solve
    - convert from/to string
    - test cases in file, read and solve
- play may not assume alternating colors, since games can be subgames 
in a sum. The color of the player is encoded as part of the move 
and is stored in the move stack. Undo must respect and use this 
color information.

## Versions
### Version 0
- Now working on version 0
- Version 0 done:
    - Nim: `nim` implementation done
    - Utility class `strip` for 1xn boards
    - Clobber: `clobber_1xn` implementation done
    - Basic minimax implementation in `solve.cpp` done
    - Basic test cases in files, run automatically
    - Nogo: `nogo_1xn` class
- Version 0 still to do:
    - review, finish in-file comments, headers
    - Tag version 0

#### Version 0 Design Choices and Remaining Uglinesses
- A `move` must be an `int`. 
    - I tried to make a generic abstract move class, but could not implement it in a "nice" and efficient way.
    - There are some utilities in `cgt_move.h` which help pack and unpack moves from/to int
    - Plan: probably keep it this way unless I find an elegant general solution
    - Move stored in game's move stack always includes a "color bit"
    - utilities to deal with color bit and "rest" of move
    - rest of move (without color bit) can be further broken up into
        two smaller integers (first one signed, second one unsigned)
        - utilities to encode and decode move from color, and two parts
    - sign bit for first part of two part move

- `move_generator` objects are dynamically allocated.
    - This is very ugly but could not solve it in a better way. I would love to have move generators just as local variables.
    - A workaround to prevent memory leaks is to always wrap a move generator in a `std::unique_ptr` - see examples in `nim_test.cpp`, function `nim_move_generator_test_1`, and in `solve.cpp`

