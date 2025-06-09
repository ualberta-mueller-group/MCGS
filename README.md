# MCGS V1.2

A **M**inimax-based **C**ombinatorial **G**ame **S**olver

Taylor Folkersen, [Martin Müller](https://webdocs.cs.ualberta.ca/~mmueller/) and Henry Du, 2024-25

MCGS is an efficient minimax search-based solver for sums of combinatorial games. Given a sum of games and a first player, MCGS determines the winner. The code is modular and extensible, allowing users to easily add new types of games and benefit from existing game-independent optimizations. Future versions will include hooks for game-specific optimizations, and implement many general search optimizations.

The overall approach and future plans will be described in a forthcoming document "The Design of MCGS: A Minimax Search-based Solver for Combinatorial Games". A brief high-level overview is given in the talk
[A Search-based Approach for Solving Sum Games](https://webdocs.cs.ualberta.ca/~mmueller/cgt/talks/2025-CGTC-MCGS.pdf).


### Sections
- [Building MCGS](#building-mcgs)
- [Using MCGS](#using-mcgs)
- [Using the Testing Framework](#using-the-testing-framework)
- [Source Code and Extending MCGS](#source-code-and-extending-mcgs)
  - [MCGS Data Types](#mcgs-data-types)
  - [Implementing a New Game](#implementing-a-new-game)
  - [Implementing Game-Specific Optimizations](#implementing-game-specific-optimizations)
    - [Splitting Into Subgames](#splitting-into-subgames-subgame_split)
    - [Simplifying Sums of Games](#simplifying-sums-of-games-simplify_basic_cgt)
    - [Hashing-Related Hooks](#hashing-related-hooks)

### Version 1.2 Additions
#### New Features
- Transposition tables speed up search
- Impartial games support
    - Impartial game solving algorithm
    - Input language version `1.1` --> `1.2`
        - Impartial game variants are automatically created for all games i.e. `[impartial clobber_1xn]`
        - New solve command for impartial sums: `{N 2}` computes nim-value, expects it to be `2`
- More games: `nogo`, `clobber`, `kayles`
- More `.test` files. Most have been verified by external solvers

#### Major Code Additions
- `game` and `sumgame` hashing
    - Several hashing-related hooks for `game`s to implement
- `grid` class
    - `grid` helper classes (`grid_utils.h`)
- `impartial_game` class
- `mcgs_init()` initializes global data
- More code safety (clang-tidy checks, debugging checks)
- More scripts in the `utils` directory
    - CGSuite scripts
    - More random game generation scripts

### Building MCGS
First download this repository, and enter its directory.

To build the program, `./MCGS`, run:
```
make
```

To run all unit tests, run:
```
make test
```
This will build and then run `./MCGS_test`, and on successful completion of unit tests, the text "SUCCESS" should appear. Running all tests can take several seconds, depending on your hardware.

### Using MCGS
`MCGS` can read input from a file, or as a quoted command line argument, or interactively from the command line via stdin. The example below solves a linear clobber game `XOXOXO` twice, once with black playing first, and once with white playing first: 
```
./MCGS "[clobber_1xn] XOXOXO {B, W}"
```

For details about using `./MCGS`, see `./MCGS --help`. Some command line arguments are meant to be used together, and will have no effect when they aren't. This is detailed in the `--help` output, and may change in future versions. 

For a full description of input syntax, including game-specific input syntax, see [input/info.test](input/info.test).

### Using the Testing Framework
The testing framework included in MCGS is used to generate, compare, and analyze performance and correctness data from MCGS. The command:
```
./MCGS --run-tests
```
will do two things:
- run all `.test` files in the `input/autotests` directory
- output all performance and correctness data to a file `out.csv`, with one row of data per game sum. 

To show test results in a form suitable for viewing in a web browser, run:
```
python3 create-table.py out.csv -o table.html
```
This will generate a HTML file `table.html` with a table, with one row for each row in `out.csv`. The script `create-table.py` contains more functionality, such as comparing two CSV result files. For information on the options and an explanation of the HTML output, run:
```
python3 create-table.py --help
```
For information about MCGS test options such as timeout duration, input directory, and output file, run:
```
./MCGS --help
```

The `input` directory of MCGS includes other sample input files. For example, the `input/main_tests` directory contains a larger set of tests than `input/autotests`.

NOTE: in the HTML output, 2 tests are expected to fail, as they intentionally have incorrect expected results.

## Source Code and Extending MCGS
The following sections are for programmers who wish to add functionality to MCGS. MCGS has a modular design, allowing users to implement new games, and define a text input format for them. The following sections first describe key internal data types, and then describe the steps for adding a new game. The reader is assumed to be familiar with C++, the programming language MCGS is written in.

The code is organized in a mostly "flat" way; most source code files are in the `./src` directory. The `main.cpp` file is in `./src/main` and all unit tests are in `./test`. In the following sections, all files mentioned by name are assumed to be in `./src` unless specified otherwise.

For more information about ongoing development, see [development-notes.md](docs/development-notes.md).
When contributing to this project, follow the style guide: [style.md](docs/style.md).

Developers can check for memory leaks and other memory errors by overriding the `ASAN` makefile variable, setting it to either `leak` or `address`, i.e:
```
make MCGS ASAN=leak
```
or
```
make MCGS ASAN=address
```
This will compile source files with either the `-fsanitize=leak` or `-fsanitize=address` flags respectively, and requires a clean build. `-fsanitize=leak` links against LeakSanitizer and has very little overhead, but detects fewer errors, whereas `-fsanitize=address` links LeakSanitizer and additionally instruments compiled code, to detect more memory errors and give more detailed diagnostics.

See: [AddressSanitizer](https://clang.llvm.org/docs/AddressSanitizer.html) for more information.

Additionally, the `DEBUG` makefile variable adds or removes debugging checks, to the extent that's sensible (you can't build MCGS_test with NDEBUG).
- `make DEBUG=0` removes debugging code (expects lots of warnings -- this is still experimental)
- `make DEBUG=1` adds more debugging code
- `make` (leaving `DEBUG` undefined) builds with default debugging code

### MCGS data types
#### game (game.h)
The abstract base type for all combinatorial games supported by MCGS.

#### strip (strip.h)
An abstract game type derived from `game`, for games played on a "line" (1 dimensional board), consisting of black stones, white stones, and empty tiles. The games `clobber_1xn`, `nogo_1xn`, and `elephants` extend `strip` and can be used as examples for new implementations.

#### grid (grid.h)
Analogue of `strip` for games played on an `MxN` grid. Used by `clobber` and `nogo`.

#### move (cgt_move.h)
Represents a move that can be played within a `game`. In this version, `move` is an integer with at least 32 bits. Each game defines the encoding of legal moves into `move`. The highest order bit is always used to encode the color of the player making the move, leaving 31 bits for the move itself. File `cgt_move.h` defines utility functions for packing and unpacking `move`s, to deal with the color bit and the "rest" of each `move`. This includes functions to encode and decode a `move` consisting of two smaller integers, for example to store a "from" and a "to" coordinate, or to encode a fraction.

#### move_generator (game.h)
This abstract type defines the interface for an iterator over all legal moves in a position derived from `game`, for a given player.

#### split_result (game.h)
Typedef of `std::optional<std::vector<game*>>`. The (possibly absent) result of splitting a `game` into subgames whose sum equals the `game` being split. During search, `game`s are split and replaced by their subgames. When `has_value()` is true and the vector is empty, the `game` being split is equal to 0. When `has_value()` is false, the vector is absent, and the split has no effect.

#### file_parser (file_parser.h)
Used by MCGS to read games from files, stdin, and quoted input strings passed as arguments to `./MCGS`. Passes string tokens representing games to a ```game_token_parser``` to get back a ```game``` object.

#### game_token_parser (game_token_parsers.h)
Abstract type converting input tokens into `game`s.

### Implementing a new game
To implement a new game `x`:
- Create 4 files: `x.h` and `x.cpp` to implement the game, and `test/x_test.h` and `test/x_test.cpp` to implement unit tests.
- Define `class x` in `x.h`, derive from `game` or `strip`.
- Each new game must implement several virtual methods: `play()`, `undo_move()`, `create_move_generator()`, `print()`, `inverse()`, and `_init_hash()`. See comments in `game.h` for notes on important implementation details.
    - For notes on `_init_hash()`, see [development-notes.md (Adding Hashing to Games, subsection 1)](docs/development-notes.md#adding-hashing-to-games).
- Define `class x_move_generator`, derive from `move_generator`.
- At the bottom of `file_parser.cpp`, add a line to the `init_game_parsers()` function, calling `add_game_parser()`, with your game name as it should appear in input files, and a `game_token_parser`. You may be able to reuse an existing `game_token_parser`, or you may need to create a new one (see `game_token_parsers.h` and `game_token_parsers.cpp`).
    - This will automatically create the impartial variant of your game.
- Document the syntax for your game in `input/info.test`, in the `Game syntax` section in the lower half of the file.
- In `test/x_test.cpp`, write a function `x_test_all` to call all unit tests for your game. Add the declaration in `test/x_test.h` 
- Call `x_test_all` from `test/main_test.cpp`.

The `test/input` directory contains input files used by unit tests. Add your new tests there.

### Implementing Game-Specific Optimizations
There are several game-specific optimizations. More will be added in future versions. Optimizations are enabled by default, and some can be toggled off (see `./MCGS --help` for details on disabling optimizations). Some optimizations introduce significant overhead.

#### Splitting Into Subgames (`subgame_split`)
In your game `x`, override and implement `game::split_implementation()`. See `game.h` for important implementation details, and add unit tests.
`split_implementation()` is used to break apart a `game` into a list of subgames whose sum is equal to the original `game`. This may speed up search by allowing MCGS to reason about smaller independent subproblems.

This optimization has significant overhead. It should improve performance if your game splits into "basic" CGT games, and `simplify_basic_cgt` remains enabled, or when using
the `{N}` solve command to get its nim value (assuming the game is either already impartial, or created as its impartial variant (i.e. `[impartial YOUR_GAME_NAME]` in
input.

NOTE: `clobber`'s split method is especially slow, and is only enabled for the `MCGS_test` executable, or when building with `DEBUG=1`. You can re-enable it by editing the
makefile, to include `-DCLOBBER_SPLIT` as a compilation argument. `clobber_1xn`'s split method remains enabled.

#### Simplifying Sums of Games (`simplify_basic_cgt`)
Currently MCGS simplifies sums containing "basic" CGT games (`integer_game`, `dyadic_rational`, `up_star`, `switch_game`, and `nimber`), by summing together their values, resulting in fewer subgames. If your game's `split_implementation()` method returns subgames of these types, they will be included in this simplification step. 

Currently there is no hook to write your own similar simplification steps, but you can modify existing functions. See the following functions:
- `sumgame::simplify_basic` (sumgame.cpp)
- `sumgame::undo_simplify_basic` (sumgame.cpp)
- `simplify_basic_all` (cgt_game_simplification.cpp)

This optimization has low overhead, as MCGS avoids running these steps unless necessary.

#### Hashing-Related Hooks
`game`s must implement `_init_hash()`, but there are other (optional) optimizations to be implemented by `game`s:
- `_order_impl()` implements lexicographical ordering of `game`s of the same type. It's used to sort `game`s before hashing sums, increasing the frequency of transposition table hits.
- `_normalize_impl()` and `_undo_normalize_impl()` transpose the `game`'s state, so that equivalent states produce the same hashes. This also increases the frequency of transposition table hits.
- Methods which modify the state of a `game` can incrementally update the hash, so that it need not be recomputed in its entirety after every state change.

For complete details, with examples, see [development-notes.md (Adding Hashing to Games)](docs/development-notes.md#adding-hashing-to-games).
