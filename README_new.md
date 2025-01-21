# MCGS V1.0

A **M**inimax-based **C**ombinatorial **G**ame **S**olver

Taylor Folkersen, Martin Müller and Henry Du, 2024

MCGS is an efficient minimax search-based solver for sums of combinatorial games. Given a sum of games and a first player, MCGS determines the winner. The code is modular and extensible, allowing users to easily add new types of games and benefit from existing game-independent optimizations. Future versions will include hooks for game-specific optimizations, and implement many general search optimizations.

For the overall approach and future plans, see the document "The Design of MCGS: A Minimax Search-based Solver for Combinatorial Games".

# TODO IN THIS README
- The intro says MCGS is efficient even though this release is very basic
- Link the formal design document "The Design of MCGS..."?
- "MCGS" is used a lot...
- License?

### Sections
- [Building MCGS](#building-mcgs)
- [Using MCGS](#using-mcgs)
- [Using the Testing Framework](#using-the-testing-framework)
- [Extending MCGS](#extending-mcgs)
- [MCGS Data Types](#mcgs-data-types)
- [Implementing a New Game](#implementing-a-new-game)

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
This will build and then run `./MCGS_test`, and on successful completion of unit tests, no output should appear.

### Using MCGS
```MCGS``` can read input from a file, or as a quoted command line argument, or interactively from the command line via stdin. Example usage solving a linear clobber game ```XOXOXO``` twice, once with black playing first, and once with white playing first: 
```
./MCGS "[clobber_1xn] XOXOXO {B, W}"
```

For details about using ```./MCGS```, see ```./MCGS --help```. For a full description of input syntax, see [input/info.test](input/info.test)

### Using the Testing Framework
Included is a testing framework which is used to generate, compare, and analyze performance and correctness data from MCGS. Running:
```
./MCGS --run-tests
```
will run all ```.test``` files in the ```autotests``` directory, outputting performance and correctness data to ```out.csv```, with one row of data per game sum. Then, running:
```
python3 create-table.py out.csv -o table.html
```
will generate ```table.html```, which is to be viewed in a web browser. The output HTML includes one row for each row in ```out.csv```. ```create-table.py``` can also compare two CSV files. For more information (i.e. comparing CSV files, explanation of output HTML, etc.), see:
```
python3 create-table.py --help
```
and for information about test options (i.e. timeout duration, input directory, output file, etc.), see:
```
./MCGS --help
```

## Extending MCGS
The following sections are for programmers who wish to add functionality to MCGS. MCGS has a modular design, allowing users to implement new kinds of games, and have them be recognized as input. The following sections first describe internal data types of interest to this goal, and then walk the reader through adding a new game. The reader is assumed to be familiar with C++, the programming language MCGS is written in.

### MCGS data types
#### game (game.h)
The abstract base type for all combinatorial games supported by MCGS.

#### strip (strip.h)
An abstract game type derived from ```game```, for games played on a "line" (1 dimensional board), consisting of black stones, white stones, and empty tiles. Used by games ```clobber_1xn```, ```nogo_1xn```, ```elephants```, etc. 

#### move (cgt_move.h)
Represents a move that can be played within a ```game```. In this version, ```move``` is an at least 32 bit integer. Games define the meaning of their ```move```s but can only use 31 bits, as the color of a player is also encoded in a ```move```. 

#### move_generator (game.h)
An abstract type implementing an iterator over a ```game```'s moves, for a specific position and player.

#### split_result (game.h)
Typedef of ```std::optional<std::vector<game*>>```. The (possibly absent) result of splitting a ```game``` into subgames whose sum equals the ```game``` being split. During search, ```game```s are split and replaced by their subgames. When ```has_value()``` is true and the vector is empty, the ```game``` being split is equal to 0. When ```has_value()``` is false, the vector is absent, and the split has no effect.

#### file_parser (file_parser.h)
Used by MCGS to read games from files, stdin, and quoted input strings passed as arguments to ```./MCGS```.

#### game_token_parser (game_token_parsers.h)
Abstract type converting input tokens into ```game```s.






### Implementing a new game

