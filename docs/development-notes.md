# Development notes
This document includes more info about design choices and implementation details not mentioned in the README.


## Search and solving a game
- Two classes implement game solving: `alternating_move_game`
and `sumgame`
- `alternating_move_game` is used for solving a single game
    - `alternating_move_game::solve` is a basic boolean negamax search
- `sumgame` is used to store and solve a sum of games. 
It is derived from `alternating_move_game`.
    - `sumgame::solve` is a basic boolean negamax search for sums
    - `sumgame::solve_with_timeout` can search with a time limit
    - enforced const-ness of games when calling solver
        - game state may modify during solve but is restored 
        at the end in any case
        - a checking function to make sure state is unchanged
            - a naive first implementation just checks length of move stack
            - TODO it probably is broken for sumgame, since it uses a different stack
            - to be replaced by a full hash


## `sumgame_move` struct (sumgame.h)
Represents a move made in a `sumgame`
- contains a subgame index and the `move` made in the subgame
- The index is into the vector `sumgame::_subgames`
- The `move` is for the subgame stored there


## `play_record` struct (sumgame.h)
- Holds information about a `sumgame_move` played in a `sumgame`, such as the `sumgame_move` itself, and whether or not the move resulted in a split, and which games were created from the split


## `sumgame` class (sumgame.h)
A `sumgame` represents a (possibly empty) set of subgames. 
It derives from `alternating_move_game` and reimplements the
`solve` method to take advantage of sum structure
- Main data structure: `vector<game*> _subgames`
    - TODO sumgame should be owner of these games? Use `std::unique_ptr`
    - TODO copy on add?
- derived from `alternating_move_game` but reimplements solve 
    - it uses `sumgame_move`
    - keeps its own `_play_record_stack` with sum-level info; subgames keep their own stacks as well


## More on extending the `game` class
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


### Version 1 completed
- `sumgame` class
    - game-dependent `split` into 0,1,2 or more subgames after a move
    - supports both keeping the old `game`, and replacing it after a `split`
    - supports changes of game type, such as `switch_game` to `integer_game`
- first experiments with sums of simple games, Clobber and NoGo
- Current limitation: games need to be defined beforehand, cannot create/delete new games on the fly
- performance testing framework
    - performance tests
    - other options to compare?
- created classes `impartial_game` and `impartial_sumgame`
    - moved some functionality from obsolete `nim` class here
    - rewrite all `nim` tests to use `sumgame` and `nimber`
    - remove `nim` class after converting tests
- implemented game::inverse() for all game types


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

        - change read from string functions to directly create sumgame
        - compare with/without subgame split


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
