# MCGS

A Minimax Search-based Solver for Combinatorial Games

## Documentation
See the document "The Design of MCGS:
A Minimax Search-based Solver for Combinatorial Games"

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

## Versions
### Version 0
- Now working on version 0
- Version 0 done:
    - Nim: `nim` implementation done
    - Utility class `strip` for 1xn boards
    - Clobber: `clobber_1xn` implementation done
    - Basic minimax implementation in `solve.cpp` done
- Version 0 still to do:
    - Nogo: `nogo_1xn` class
    - Basic test cases in files, run automatically

#### Version 0 Design Choices and Remaining Uglynesses
- A `move` must be an `int`. 
    - I tried to make a generic abstract move class, but could not implement it in a "nice" and efficient way.
    - There are some utilities in `cgt_move.h` which help pack and unpack moves from/to int
    - Plan: probably keep it this way unless I find an elegant general solution
- `move_generator` objects are dynamically allocated.
    - This is very ugly but could not solve it in a better way. I would love to have move generators just in local variables.
    - A workaround to prevent memory leaks is to always wrap a move generator in a `std::unique_ptr` - see examples in `nim_test.cpp`, function `nim_move_generator_test_1`, and in `solve.cpp`

## TODO - test cases
From class in `~/Projects/ualberta-mueller-group/combinatorial_game_solver/PriorWork`

From Taylor's solver 