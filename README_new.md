# MCGS V1.0

A **M**inimax-based **C**ombinatorial **G**ame **S**olver

Taylor Folkersen, Martin Müller and Henry Du, 2024

MCGS is an efficient minimax search-based solver for sums of combinatorial games. Given a sum of games and a first player, MCGS determines the winner. The code is modular and extensible, allowing users to easily add new types of games and benefit from existing game-independent optimizations. Future versions will include hooks for game-specific optimizations, and implement many general search optimizations.

For the overall approach and future plans, see the document "The Design of MCGS: A Minimax Search-based Solver for Combinatorial Games".

# TODO IN THIS README
- The intro says MCGS is efficient even though this release is very basic
- Link the formal design document "The Design of MCGS..."?
- "MCGS" is used a lot...

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
```MCGS``` can read input from a file, or as a quoted command line argument, or interactively from the command line via stdin. Example usage running a linear clobber game ```XOXOXO``` twice, once with black playing first, and once with white playing first: 
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
will generate ```table.html```, which is to be viewed in a web browser. The output HTML includes one row for each row in ```out.csv```. ```create-table.py``` can also compare two CSV files. For more information, see:
```
python3 create-table.py --help
```
and for information about test options (i.e. timeout duration, input directory, output file, etc.), see:
```
./MCGS --help
```

## Extending MCGS
MCGS has a modular design, allowing users to implement new kinds of games, and have them be recognized as input. The following sections first describe internal data types of interest to this goal, and then walk the reader through adding a new game. The reader is assumed to be familiar with C++, the programming language MCGS is written in.

### MCGS data types
### Implementing a new game

