# MCGS V1.4.1

A **M**inimax-based **C**ombinatorial **G**ame **S**olver

Taylor Folkersen, [Martin Müller](https://webdocs.cs.ualberta.ca/~mmueller/) and Henry Du, 2024-25

MCGS is an efficient minimax search-based solver for sums of combinatorial
games. Given a sum of games and a first player, MCGS determines the winner. The
code is modular and extensible, allowing users to easily add new types of games
and benefit from existing game-independent optimizations. Future versions will
include hooks for game-specific optimizations, and implement many general
search optimizations.

The overall approach and future plans will be described in a forthcoming
document "The Design of MCGS: A Minimax Search-based Solver for Combinatorial
Games". A brief high-level overview is given in the talk
[A Search-based Approach for Solving Sum Games](https://webdocs.cs.ualberta.ca/~mmueller/cgt/talks/2025-CGTC-MCGS.pdf).


### Sections
- [Version 1.4 Additions](#version-14-additions)
- [Building MCGS](#building-mcgs)
- [Using MCGS](#using-mcgs)
- [Using the Database](#using-the-database)
- [Using the Testing Framework](#using-the-testing-framework)
- [Source Code and Extending MCGS](#source-code-and-extending-mcgs)
  - [MCGS Data Types](#mcgs-data-types)
  - [Implementing a New Game](#implementing-a-new-game)
  - [Implementing Game-Specific Optimizations](#implementing-game-specific-optimizations)
    - [Splitting Into Subgames](#splitting-into-subgames-subgame_split)
    - [Simplifying Sums of Games](#simplifying-sums-of-games-simplify_basic_cgt)
    - [Hashing-Related Hooks](#hashing-related-hooks)
    - [Adding A Game To the Database](#adding-a-game-to-the-database)

### Version 1.4 Additions
#### 1.4.1 Fixes
- Fixed compilation error on Mac OS
- Very long unit tests made faster
- Compatibility warnings added to [Using the Database](#using-the-database) section
    - Added extra checks for loading database files
- Increase default transposition table size to match previous versions
#### New Features
- New games (see `input/info.test` for syntax)
    - Amazons
    - Domineering
    - Fission
    - Battle Sheep
        - Implementation of: https://www.blueorangegames.com/games/battle-sheep
    - Toppling Dominoes
    - Generalized Toads and Frogs
- Split functions for Amazons, Domineering, and Sheep
- Basic player. Interactively play games against MCGS with the `--play-mcgs`
  option!
    - Optionally log games played to a file, i.e. `--play-log log.txt`
- Configurable database
    - `--db-file-create <file name> <DB config string>`
    - See README for details
- `--print-winning-moves` CLI option prints winning moves for input sums
    - Has limitations, see `./MCGS -h`
- MCGS web site (`docs/index.html`)
    - Contains computational results
    - Links to a user guide, our ACG 2025 paper, and our ACG 2025 presentation
      slides
- More data in `input` directory
- Input language version `1.3` --> `1.4`

#### Major Code Additions
- `game::_order_impl` (lexicographic comparison method) is no longer used
- Refactored `cgt_move.h` and `cgt_basics.h`
    - `cgt_basics.h` defines colors, and functions to convert between `int` and
      `char` representations of colors
    - `cgt_move.h` defines several multi part move functions
        - Move layout structs simplify the work of adding new multi part moves
- Grid generator classes merged into one single `grid_generator` class
    - See development notes for important usage details
- `grid_hash` class maps grid games having rotation/transpose symmetry to the
  same local hashes
  - Manually added to grid games on a per-game basis. See development notes for
    instructions.
  - Significantly speeds up database creation for grid games
  - Decreases the number of entries in the database
  - Creates more transposition table hits in some cases
- Experimental WebAssembly build using Emscripten. `WASM=1` makefile variable
    - Must first copy contents of `utils/wasm` to project root

### Building MCGS
First download this repository, and enter its directory.

To build the program, `./MCGS`, run:
```
make
```
This may take a few minutes. Depending on your hardware, you may build using
more threads, i.e. to build using 4 threads:
```
make -j 4
```

To run all unit tests, run:
```
make test
```
or:
```
make test -j 4
```
This will build and then run `./MCGS_test`, and on successful completion of
unit tests, the text "SUCCESS" should appear. Running all tests can take
several seconds, depending on your hardware. The `test_extra` makefile target
runs some unit tests on larger ranges of values, which takes much longer.

### Using MCGS
`MCGS` can read input from a file, or as a quoted command line argument, or
interactively from the command line via stdin. The example below solves a
linear clobber game `XOXOXO` twice, once with black playing first, and once
with white playing first: 
```
./MCGS "[clobber_1xn] XOXOXO {B, W}"
```

For details about using `./MCGS`, see `./MCGS
--help`. Some command line arguments are meant to be used together, and will
have no effect when they aren't. This is detailed in the `--help` output, and
may change in future versions.

For a full description of input syntax, including game-specific input syntax,
see [input/info.test](input/info.test).

### Using the Database
NOTE: Database files from previous versions are not compatible with version 1.4.

The database is loaded from `database.bin` automatically on startup if it
exists and `--no-use-db` is not specified. `--db-file-load <file name>` is used
to specify a different file. This file is not included in the repository and
must be generated for specific games manually.

`./MCGS --db-file-create <file name> <DB config string>` is used to create the
database. The config string specifies games to include in the database, and
their maximum sizes.

Example:
```
./MCGS --db-file-create database.bin "[clobber] max_dims = 3,3; [nogo_1xn] max_dims = 8;"
```
This creates the file `database.bin` and populates it with Clobber games whose
dimensions are at most 3x3, and linear NoGo games whose lengths are at most 8.
The database contains single subgames which are normalized and do not split
into more subgames.

Games currently supported by the database:
- Strip games:
    - `clobber_1xn`
    - `nogo_1xn`
    - `elephants`
    - `toppling_dominoes`
- Grid games
    - `amazons`
    - `nogo`
    - `clobber`
    - `domineering`
    - `fission`
    - `sheep`

The `max_dims` parameter must be specified for all of these games. For strip
games, it should be specified as a single non-negative integer, and for grid
games it should be specified as a pair of non-negative integers. For grid
games, the order of the max dimensions does not matter: `max_dims = 2,3;` has
the same effect as `max_dims = 3,2;` (all 2x3 and 3x2 games are generated in
both cases).

`sheep` requires an additional parameter `max_sheep`, similarly specified as a
pair of non-negative integers. The elements of the pair indicate the maximum
number of black and white sheep respectively. i.e. for `max_sheep = 1,2;`,
all `sheep` games having at most 1 black sheep, and at most 2 white sheep, will
be generated.

The command line option `--print-db-info` can be specified to print database
info for the loaded database file. This includes the date created, number of
each game type contained by the file, and the config string used to generate
the file.

NOTE: Database files are not portable, and may only work on the same machine
that generated them, assuming the same compiler and C++ standard library
implementation is used. If you are a developer looking to extend MCGS, see:
[development-notes.md (Database File Portability)](docs/development-notes.md#database-file-portability).

### Using the Testing Framework
The testing framework included in MCGS is used to generate, compare, and
analyze performance and correctness data from MCGS. The command:
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
This will generate a HTML file `table.html` with a table, with one row for each
row in `out.csv`. The script `create-table.py` contains more functionality,
such as comparing two CSV result files. For information on the options and an
explanation of the HTML output, run:
```
python3 create-table.py --help
```
For information about MCGS test options such as timeout duration, input
directory, and output file, run:
```
./MCGS --help
```

The `input` directory of MCGS includes other sample input files. For example,
the `input/main_tests` directory contains a larger set of tests than
`input/autotests`.

NOTE: in the HTML output, 2 tests are expected to fail, as they intentionally
have incorrect expected results.

## Source Code and Extending MCGS
The following sections are for programmers who wish to add functionality to
MCGS. MCGS has a modular design, allowing users to implement new games, and
define a text input format for them. The following sections first describe key
internal data types, and then describe the steps for adding a new game. The
reader is assumed to be familiar with C++, the programming language MCGS is
written in.

The code is organized in a mostly "flat" way; most source code files are in the
`./src` directory. The `main.cpp` file is in `./src/main` and all unit tests
are in `./test`. In the following sections, all files mentioned by name are
assumed to be in `./src` unless specified otherwise.

For more information about ongoing development, see
[development-notes.md](docs/development-notes.md). When contributing to this
project, follow the style guide: [style.md](docs/style.md).

Developers can check for memory leaks and other memory errors by overriding the
`ASAN` makefile variable, setting it to either `leak` or `address`, i.e:
```
make MCGS ASAN=leak
```
or
```
make MCGS ASAN=address
```
This will compile source files with either the `-fsanitize=leak` or
`-fsanitize=address` flags respectively, and requires a clean build.
`-fsanitize=leak` links against LeakSanitizer and has very little overhead, but
detects fewer errors, whereas `-fsanitize=address` links LeakSanitizer and
additionally instruments compiled code, to detect more memory errors and give
more detailed diagnostics.

See: [AddressSanitizer](https://clang.llvm.org/docs/AddressSanitizer.html) for
more information.

Additionally, the `DEBUG` makefile variable adds or removes debugging checks,
to the extent that's sensible (you can't build MCGS_test with NDEBUG).
- `make DEBUG=0` removes debugging code (expects lots of warnings -- this is
  still experimental)
- `make DEBUG=1` adds more debugging code
- `make` (leaving `DEBUG` undefined) builds with default debugging code

### MCGS data types
#### game (game.h)
The abstract base type for all combinatorial games supported by MCGS.

#### impartial_game (impartial_game.h)
The abstract base type for all impartial combinatorial games supported by MCGS.
Derives from `game`.

#### strip (strip.h)
An abstract game type derived from `game`, for games played on a "line" (1
dimensional board), consisting of black stones, white stones, and empty tiles.
The games `clobber_1xn`, `nogo_1xn`, and `elephants` extend `strip` and can be
used as examples for new implementations.

#### grid (grid.h)
Analogue of `strip` for games played on an `MxN` grid. Used by `clobber` and `nogo`.

#### move (cgt_move.h)
Represents a move that can be played within a `game`. In this version, `move`
is an integer with 32 bits. Each game defines the encoding of legal
moves into `move`. The highest order bit is always used to encode the color of
the player making the move, leaving 31 bits for the move itself. File
`cgt_move.h` defines utility functions for packing and unpacking `move`s, to
deal with the color bit and the "rest" of each `move`. This includes functions
to encode/decode multiple small integers to/from a `move`.

#### move_generator (game.h)
This abstract type defines the interface for an iterator over all legal moves
in a position derived from `game`, for a given player.

#### split_result (game.h)
Typedef of `std::optional<std::vector<game*>>`. The (possibly absent) result of
splitting a `game` into subgames whose sum equals the `game` being split.
During search, `game`s are split and replaced by their subgames. When
`has_value()` is true and the vector is empty, the `game` being split is equal
to 0. When `has_value()` is false, the vector is absent, and the split has no
effect.

#### file_parser (file_parser.h)
Used by MCGS to read games from files, stdin, and quoted input strings passed
as arguments to `./MCGS`. Passes string tokens representing games to a
```game_token_parser``` to get back a ```game``` object.

#### game_token_parser (game_token_parsers.h)
Abstract type converting input tokens into `game`s.

### Implementing a new game
The following is a brief list of instructions for implementing a new game. For
more information, see
[development-notes.md (More On Extending the `game` Class)](docs/development-notes.md#more-on-extending-the-game-class) and
[development-notes.md (Impartial Games)](docs/development-notes.md#impartial-games).

To implement a new game `x`:
- Create 4 files: `x.h` and `x.cpp` to implement the game, and `test/x_test.h` and `test/x_test.cpp` to implement unit tests.
- Define `class x` in `x.h`, derive from `game`, `strip` or `grid`.
    - Impartial games should instead derive from `impartial_game`
- Each new game must implement several virtual methods: `play()`, `undo_move()`, `create_move_generator()`, `print()`, `inverse()`, and `_init_hash()`. See comments in `game.h` for notes on important implementation details.
    - Impartial games have a slightly different set of methods to implement
        - `create_move_generator()` doesn't take a color argument
        - Both `play` methods must be implemented: one with a color argument, and one without
        - See `cgt_nimber.h` and `cgt_nimber.cpp` for an example implementation
    - For notes on `_init_hash()`, see [development-notes.md (Adding Hashing to Games, subsection 1)](docs/development-notes.md#adding-hashing-to-games).
- Define `class x_move_generator`, derive from `move_generator`.
- At the bottom of `file_parser.cpp`, add a line to the `init_game_parsers()` function, calling `add_game_parser()`, with your game name as it should appear in input files, and a `game_token_parser`. You may be able to reuse an existing `game_token_parser`, or you may need to create a new one (see `game_token_parsers.h` and `game_token_parsers.cpp`).
    - This will automatically create the impartial variant of your game.
- Document the syntax for your game in `input/info.test`, in the `Game syntax` section in the lower half of the file.
- In `test/x_test.cpp`, write a function `x_test_all` to call all unit tests for your game. Add the declaration in `test/x_test.h` 
- Call `x_test_all` from `test/main_test.cpp`.

The `test/input` directory contains input files used by unit tests. Add your new tests there.

### Implementing Game-Specific Optimizations
There are several game-specific optimizations. More will be added in future
versions. Optimizations are enabled by default, and some can be toggled off
(see `./MCGS --help` for details on disabling optimizations). Some
optimizations introduce significant overhead.

#### Splitting Into Subgames (`subgame_split`)
In your game `x`, override and implement `game::split_implementation()`. See
`game.h` for important implementation details, and add unit tests.
`split_implementation()` is used to break apart a `game` into a list of
subgames whose sum is equal to the original `game`. This may speed up search by
allowing MCGS to reason about smaller independent subproblems.

This optimization has significant overhead. It should improve performance if
your game splits into "basic" CGT games, and `simplify_basic_cgt` remains
enabled, or when using the `{N}` solve command to get its nim value (assuming
the game is either already impartial, or created as its impartial variant (i.e.
`[impartial YOUR_GAME_NAME]` in input. It should also improve minimax search
time if subgames become smaller after splitting (i.e. break into boards with
smaller dimensions), as smaller boards are more likely to be in the database.

#### Simplifying Sums of Games (`simplify_basic_cgt`)
Currently MCGS simplifies sums containing "basic" CGT games (`integer_game`,
`dyadic_rational`, `up_star`, `switch_game`, and `nimber`), by summing together
their values, resulting in fewer subgames. If your game's
`split_implementation()` method returns subgames of these types, they will be
included in this simplification step. 

Currently there is no hook to write your own similar simplification steps, but
you can modify existing functions. See the following functions:
- `sumgame::simplify_basic` (sumgame.cpp)
- `sumgame::undo_simplify_basic` (sumgame.cpp)
- `simplify_basic_all` (cgt_game_simplification.cpp)

This optimization has low overhead, as MCGS avoids running these steps unless
necessary.

#### Hashing-Related Hooks
`game`s must implement `_init_hash()`, but there are other (optional)
optimizations to be implemented by `game`s:
- `_normalize_impl()` and `_undo_normalize_impl()` transpose the `game`'s state, so that equivalent states produce the same hashes. This also increases the frequency of transposition table hits.
- Methods which modify the state of a `game` can incrementally update the hash, so that it need not be recomputed in its entirety after every state change.
- As of MCGS version 1.4, `game::_order_impl()` is unused and doesn't need to be implemented.

For complete details, with examples, see [development-notes.md (Adding Hashing to Games)](docs/development-notes.md#adding-hashing-to-games).

#### Adding A Game To the Database
To add your game to the database, you must register your game class with the database,
and define or reuse a function to create a game generator. See
[development-notes.md (Adding A Game To the Database)](docs/development-notes.md#adding-a-game-to-the-database)
for details.
