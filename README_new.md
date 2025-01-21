# MCGS V1.0

A **M**inimax-based **C**ombinatorial **G**ame **S**olver

Taylor Folkersen, Martin Müller and Henry Du, 2024

MCGS is an efficient minimax search-based solver for sums of combinatorial games. Given a sum of games and a first player, MCGS determines the winner. The code is modular and extensible, allowing users to easily add new types of games and benefit from existing game-independent optimizations. Future versions will include hooks for game-specific optimizations, and implement many general search optimizations.

For the overall approach and future plans, see the document "The Design of MCGS: A Minimax Search-based Solver for Combinatorial Games".

# TODO IN THIS README
- The intro says MCGS is efficient even though this release is very basic
- Link the formal design document "The Design of MCGS..."?

### Building MCGS
First download this repository.

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
```MCGS``` can read input from a file, or as a command line argument, or interactively from the command line via stdin. Example usage running a linear clobber game "XOXOXO" twice, once with black playing first, and once with white playing first: 
```./MCGS "[clobber_1xn] XOXOXO {B, W}"```

For details about using ```./MCGS```, see ```./MCGS --help```. For a full description of input syntax, see [info.test](input/info.test)

SECTION TEXT
### Using testing framework
SECTION TEXT
## Extending MCGS
INTRO TEXT
### Implementing a new game
SECTION TEXT
### MCGS data types
SECTION TEXT
