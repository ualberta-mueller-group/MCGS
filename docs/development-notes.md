# Development notes
This document includes more detailed information than `README.md`, including design choices and tradeoffs, version history, and implementation details.



# Search and Solving a Game
- Two classes implement game solving: `alternating_move_game`
and `sumgame`
- `alternating_move_game` is used for solving a single game, without splitting it into a sum of subgames
    - `alternating_move_game::solve` is a basic boolean negamax search
- `sumgame` is used to store and solve a sum of games. 
It is derived from `alternating_move_game`.
    - `sumgame::solve` is a basic boolean negamax search for sums
    - `sumgame::solve_with_timeout` can search with a time limit
    - TODO `sumgame::solve_with_timeout` could use a macro to check if the result of a recursive call timed out
    - `sumgame::solve_with_games` temporarily adds games to the sum, calls `sumgame::solve`, then removes the games and returns the result.

### "Logically const" interface for solving games
- In both `alternating_move_game` and `sumgame`, the `solve` method is declared as const.
- This means that while the game state may be modified during solve,
it must be restored before the end of `solve` in any case, including timeout or other failure modes.
- `class assert_restore_game` in `alternating_move_game.h` is a stub for
checking that the state is restored
    - a naive first implementation just checks the length of the move stack
    - TODO it probably is broken for sumgame, since it uses a different stack
    - TODO this check will be made functional after hash codes are implemented

# Safe arithmetic functions (safe_arithmetic.h)
This section uses the term "wrapping" to mean either underflow or overflow.

- Defines template functions to do arithmetic without wrapping
- Assumes underlying machine uses two's complement to represent integers
- All functions return a `bool`
    - When `true`, the operation was completed without wrapping
    - When `false`, the operation would have wrapped. No operands were changed
        - Also returned on invalid arguments, i.e. negative bit shift amounts
- Some functions accept either integer types or floating point types, some accept only integer types
    - The lists below use `num` to refer to either, and `int` to refer to integer types. All operands must have the same type.

These functions test whether an operation would wrap, without doing the operation:
- `add_will_wrap(const num x, const num y)`
    - `true` iff `x + y` would wrap
- `subtract_will_wrap(const num x, const num y)`
    - `true` iff `x - y` would wrap
- `negate_will_wrap(const num x)`
    - `true` iff `-x` would wrap

These functions perform operations, and will only change the operands on success:
- `safe_add(num& x, const num y)`
    - `x := x + y`
- `safe_add_negatable(num& x, const num y)`
    - `x := x + y`, also fails if negating the resulting `x` would wrap (i.e. `-x`)
- `safe_subtract(num&, const num)`
    - `x := x - y`
- `safe_subtract_negatable(num&, const num)`
    - `x := x - y` also fails if negating the resulting `x` would wrap
- `safe_negate(num&)`
    - `x := -x`
- `safe_mul2_shift(int& x, const int exponent)`
    - `x := x * 2^exponent` (implemented as left shift)
    - Negative values of `x` are allowed
    - also `false` when the result would flip the sign
- `safe_pow2_mod(int& x, const int pow2)`
    - `x := x % pow2` (implemented as bitwise `&`)
    - `false` if `pow2` is not a power of 2, or `pow2 <= 0`

# More on data types

## `sumgame_move` struct (sumgame.h)
Represents a move made in a `sumgame`
- Contains a subgame index and the `move` made in the subgame
- The index is into the vector `sumgame::_subgames`
- The `move` is for the subgame stored there

## `play_record` struct (sumgame.h)
- Holds information about a `sumgame_move` played in a `sumgame`
    - the `sumgame_move` itself
    - whether or not the move resulted in a split
    - which subgames were created from the split

## `sumgame_impl::change_record` class (sumgame_change_record.h)
- Similar to `play_record`; used to track changes to `sumgame` made by game simplification steps
    - Holds 2 `vector<game*>`s: one for deactivated games, and one for added games
    - Undo operation reactivates games, then pops games from `sumgame` and deletes them
- Simplifications are implemented as `change_record` methods
    - i.e. `change_record::simplify_basic(sumgame&)` which sums together basic CGT games such as `integer_game`, `dyadic_rational`, `switch_game` etc

## `sumgame_map_view` class (sumgame_map_view.h)
- Sorts all `game` objects contained by a `sumgame` using RTTI (run-time type information), acting as a map from game type to `vector<game*>&`
- Has public methods to mutate underlying `sumgame` while keeping it synchronized with the map view
    - These mutations are stored in a `change_record`
    - i.e. `sumgame_map_view::deactivate_game(game*)`, `sumgame_map_view::add_game(game*)`
- Only games with `is_active() == true` are kept in the map

## `sumgame::undo_stack_unwinder` class (sumgame_undo_stack_unwinder.h)
- Private inner class of `sumgame`
- Created as local variable at the start of `sumgame::_solve_with_timeout()`
    - Constructor pushes a marker onto `sumgame`'s undo stack
    - Destructor iteratively pops undo stack, calling undo functions (i.e. `sumgame::undo_move()`, `sumgame::undo_simplify_basic()`) until it sees the marker

## `sumgame` class (sumgame.h)
A `sumgame` represents a (possibly empty) set of subgames. 
It derives from `alternating_move_game` and reimplements the
`solve` method to take advantage of sum structure
- Main data structure: `vector<game*> _subgames`
    - TODO sumgame should be owner of these games? Use `std::unique_ptr`
    - TODO copy on add?
- `sumgame` is derived from `alternating_move_game` but reimplements solve 
    - It uses `sumgame_move`
    - It keeps its own `_play_record_stack` with sum-level info
    - Subgames keep their own stacks with local moves as well

## `fraction` class (fraction.h)
- Simple and lightweight fraction type whose denominator must be a positive power of 2
    - Consists only of 2 public `int`s: `top` and `bottom`, making copying fast
    - Has comparison operators (`<`, `<=`, `==`, `!=`, `>=`, `>`)
        - These will never fail, but are significantly more expensive than `int` comparisons due to needing to make operands compatible (have the same denominator)
        - `fraction(1, 2) == fraction(2, 4)`
    - Has safe operations which may fail and return false, but will not underflow or overflow
        - On failure, `fraction` operands may be left more (or less) simplified, but will still be equivalent to before the operation (i.e. according to `==`)
        - `safe_add_fraction(fraction& x, fraction& y)` and `safe_substract_fraction(fraction& x, fraction& y)`
            - If safe, do `x = x + y` or `x = x - y`
        - Make `fraction`s compatible
        - Negate
        - Raise denominator
            - To target value
            - By number of left bit shifts
    - Has operations which will not fail
        - Simplify
        - Get (and optionally remove) integral part as an `int` (whose denominator is 1)


## More on Extending the `game` Class
- In every game implementation:
    - `x::play()` must call `game::play()`
    - `x::undo_move()` must call `game::undo_move()`
- Move generators are accessible only through the abstract game interface `create_move_generator`
    - Generators are dynamically allocated - wrap each use in a `std::unique_ptr`
    - An example is in `alternating_move_game::solve`
    - A game-specific move generator is declared and used only in `x.cpp`, not in a header file
- Game unit tests should cover at least:
    - `play` and `undo_move`
    - `solve` for both black and white
    - Convert from/to string
    - Write test cases in file, read and solve
    - Game-specific move generator
        - Count the number of moves and details of moves generated, such as move order and specific moves
- `play()` may not assume alternating colors, since a game can be a subgame 
in a sum. 
- The color of the player is encoded as part of the move 
and is stored in the move stack. 
- `undo_move` must respect and use the move player color information in the stack.

# Bounds
`bounds.h` and `bounds.cpp` define functions and types used for finding lower and upper bounds of games.

- The `bound_scale` enum represents a scale of games on which bounds are found.
- `bound_t` is a signed integral type representing an index along a bound scale.
- The functions `get_scale_game()` and `get_inverse_scale_game()` return a new `game` object for a given scale and index along the scale.
- The `relation` enum denotes how two games are related (i.e. less than, equal to, etc).
- The `bounds_options` struct specifies a scale and interval on which bounds should be searched for.

Some scales and their games at select indices are shown in the following table:
| Scale | -2 | -1 | 0 | 1 | 2 |
| --- | --- | --- | --- | --- | --- |
| up_star | vv* | v* | * | ^* | ^^* |
| up | vv | v | 0 | ^ | ^^ |
| dyadic_rational | -2/8 | -1/8 | 0 | 1/8 | 2/8 |

## `game_bounds` class
A pair of lower and upper bounds for a sumgame `S` is represented by the `game_bounds` class. Each bound within is represented by a `bound_t` indicating its location along a scale, a `bool` indicating whether it's valid or invalid, and a `relation` indicating its relation to `S`. The `relation` is interpreted with the bound game `G` being on the left hand side, i.e. `G REL_LESS_OR_EQUAL S`, or `G REL_GREATER S` to denote `G <= S` and `G > S` respectively.

For serializaton purposes, it may be better to encode data for each bound into one `bound_t` using bitmasks, instead of having a `bound_t`, `bool`, and `relation`.

## `find_bounds()` function
`find_bounds()` computes both bounds for a `sumgame`, or `vector<game*>`, or `game*`. It takes a `vector` of one or more `bounds_options` structs, and returns a `vector` containing one `game_bounds` object for each `bounds_options`. The actual return type is `vector<game_bounds_ptr>`, where `game_bounds_ptr` is a typedef of `shared_ptr<game_bounds>`.

`find_bounds()` implements binary search, and calls `sumgame::solve_with_games()` to compare `S` against games on a scale. Fuzzy comparisons (when `S - Gi` for some scale index `i` is a first player win) cause the search space represented by the interval `[MIN, MAX]` to split into two intervals: `[MIN, i)` and `(i, MAX]`, and search continues on both new intervals.

After finding the tightest possible bounds with non-strict inequalities, i.e. (`lower_bound <= S`), the relations are made strict (such that either `lower_bound < S` or `lower_bound == S`). When `S`'s bounds are not within the given scale and search interval, search is aborted after a few iterations, and the returned bounds will either be invalid or too loose to be useful.

For serialization purposes, it may be better if `find_bounds()` returns a `vector<game_bounds>` instead of `vector<game_bounds_ptr>`.

### Search Heuristics
At each binary search iteration, `find_bounds()` compares sumgame `S` to a scale game `Gi` at scale index `i`, by solving the sum `S - Gi` for either black or white to play first. Solving the sum for black tests whether `Gi >= S`, and solving the sum for white tests whether `Gi <= S`. At each search iteration, a guess is made as to which condition should be checked first, based on the assumption that if `i` is below the midpoint of the current tentative bounds, it's more likely that `Gi <= S`, and if `i` is above the midpoint, it's more likely that `Gi >= S`. When the current tentative bounds are not defined on both ends, 0 is used as the midpoint. When `i` is equal to the midpoint, a simple tie-breaking rule is used: always pick the same test to do first, and if the first test is inconclusive (and the relation between `Gi` and `S` is not fuzzy), flip which test will come first at the next occurence of a tie.

### Related Scales
Some scales are related, namely `up_star` and `up`, and knowing a game's bounds on one scale may be useful for computing its bounds on related scales. If a game has the bounds `[lower_bound, upper_bound]` on the scale `up_star`, its bounds along the scale `up` will lie within `[lower_bound - 2, upper_bound + 2]`. Currently there are no optimizations in place to handle this.

The following table gives experimental data of the total number of `sumgame::solve_with_games()` calls used to find non-strict bounds for 80 random `clobber_1xn` games along the interval `up`, using different search intervals and optimizations. Each run used the same 80 games. The optimizations are as follows:
- `Default`
    - Bounds are found without using knowledge of other bounds or scales by calling `find_bounds()` along `up`.
- `Search around edges`
    - Knowing bounds `[lower_bound, upper_bound]` along `up_star`, do linear search for new lower and upper bounds individually, along `up`, between `[lower_bound - 2, lower_bound + 2]` and `[upper_bound - 2, upper_bound + 2]` respectively.
- `Caller shrinks interval`
    - Knowing bounds `[lower_bound, upper_bound]` along `up_star`, call `find_bounds()` along `up` in the interval `[lower_bound - 2, upper_bound + 2]`.


| Search interval | Default | Search around edges | Caller shrinks interval |
| --- | --- | --- | --- |
| [-32, 32] | 874 calls | 725 calls | 577 calls |
| [-16000, 16000] | 1822 calls | 725 calls | 575 calls |

Perhaps a more sophisticated version of "search around edges" would perform better?

### Search Interval Size
Searching for bounds is faster within smaller intervals. When finding bounds for many sumgames, perhaps the caller of `find_bounds()` should dynamically adjust the search interval to be "close" to bounds found for previous games. When the chosen scale and interval don't contain bounds, `find_bounds()` aborts search after a small number of comparisons (~6 `sumgame::solve_with_games()` calls).

Maybe bound generation in the database should be done using a sliding window of statistics for the last `N` games to help size intervals appropriately.

# Game Simplification
`sumgame::simplify_basic()` sums together basic CGT games to simplify the sum (and `sumgame::undo_simplify_basic()` undoes this). At each step, all games of a type are handled. Run-time type information is used to distinguish game types. To ensure that new games produced by a previous step may be included in the next step, steps happen in the following order:

1. `nimber`
2. `switch_game`
3. `integer_game` and `dyadic_rational`
4. `up_star`

Steps producing no useful simplification (i.e. only summing up a single game) will not modify the `sumgame`. i.e. if the `sumgame` has only one non-zero `up_star` game, the game will be left alone, rather than duplicated with one inactive copy. 

## `nimber` Simplification
- All `nimber`s are summed together using `nimber::nim_sum()`
- This step only has an effect if the sum contains at least 2 nimbers, or contains 1 nimber with `value() <= 1`
- May add 1 `nimber`, or 1 star (as `up_star`), or nothing
- Overflow is not a concern, as nimber addition is an XOR operation

## `switch_game` Simplification
`switch_game`s are of the form `{X | Y}` for some rationals `X` and `Y`.

First, all `switch_game`s are sorted based on their `switch_kind`. Only `SWITCH_KIND_PROPER_SWITCH` and `SWITCH_KIND_NUMBER_AS_SWITCH` are used. `SWITCH_KIND_RATIONAL` and `SWITCH_KIND_PROPER_SWITCH_NORMALIZED` are left alone. There are two major cases (with some subcases), described in the next subsections.

Note that operations involve the `fraction` class, and when arithmetic overflow occurs during simplification of a given `switch_game` object, this particular object is skipped and left untouched.

#### Proper Switch
Proper switches are `switch_game`s where `X > Y`. These games are normalized to be of the form `M + {A | -A}`, for rationals `M` and `A`, where `M` is the mean of `X` and `Y`, and `A = X - M`.

#### Number As Switch
Switches representing numbers are `switch_game`s where `X <= Y`. Several subcases occur:

- If `X < 0` and `Y > 0`, the game is 0 (so it is just deactivated)
- If `X == Y`, the game is replaced with `X + *` (a `dyadic_rational` and `up_star`)
- Otherwise the game is replaced by the "simplest" rational `U`, `X < U < Y`

The simplest such value is the unique rational `U = i/2^j` for some integers `i` and `j`, `j >= 0`, having minimal `j`, or if `j == 0`, having `i` with smallest absolute value. This value is found by iteratively increasing `j` and checking if `U` occurs for the current iteration of `j`.


## `integer_game` and `dyadic_rational` Simplification
All `integer_game`s are summed together, then all `dyadic_rational`s are summed together. The two resulting sums are then summed. Within each of these 3 summations, the first summand which would result in overflow causes the summation to stop. For example:
```
INT_MAX + -1 + 2 + -50
```
will only use the first 2 values.



## `up_star` Simplification
`up_star`s are summed together similarly to `integer_game`s and `dyadic_rational`s. The first summand which would cause overflow causes the summation to stop.




# Design Choices and Remaining Uglinesses
## A `move` must be implemented as an `int` 
- It is challenging to make a generic abstract move class, in a "nice" and efficient way.
- Future Plan: probably keep it this way unless we find an elegant general solution
- It will break for games which have complex move descriptions
    - Possible way out: support partial moves which fit an integer

## `move_generator` objects are dynamically allocated
- This is ugly but it is unclear how to solve it in a better way. 
- It would be best to have move generators defined just as local variables.
- A workaround to prevent memory leaks is to always wrap 
a move generator in a `std::unique_ptr` 
    - See the example in `alternating_move_game::solve`

## Reimplementation/duplication of `game` concepts in `sumgame`
- This is a consequence of - A `move` must be an `int`
- A move in a sumgame is specified in `struct sumgame_move` by two parts: the index of the subgame in the `_subgames` vector, and the move inside the subgame (encoded in a game-specific way)
- `play()` in sumgame takes a `sumgame_move` as argument, not a `move`
- `solve()` also uses `sumgame_move`

# Versions
## Version 0 completed
- `move` and `game` classes, `alternating_move_game`
- Nim: `nim` implementation done
- Utility class `strip` for 1xn boards
- Clobber on a strip: `clobber_1xn`
- Basic minimax implementation in `alternating_move_game::solve`
- Basic test cases in files, run automatically
- Nogo on a strip: `nogo_1xn` class
- Simple game classes: integer, dyadic rational, up-star, switch, nimber

## Version 1 completed
- `sumgame` class
    - game-dependent `split` into 0,1,2 or more subgames after a move
    - supports both keeping the old `game`, and replacing it after a `split`
    - supports changes of game type by a move, such as `switch_game` to `integer_game`
- extensive test cases for mixed sums of simple games, Clobber, NoGo, Elephants
- Performance testing framework
    - performance tests
    - option to compare with previous test results
- rewritten all `nim` tests to use `sumgame` and `nimber`
- removed `nim` class and moved functionality such as nim sum to `nimber`
- implemented `game::inverse()` for all game types

## Future: Smaller Step Versions 1.x to Prepare for Version 2

### Version 1.1 - More cleanup and Tools and Components for Database
- Code formatting tools and checks, "lint"-like
- `scale` S such as multiples of up, or up+star, or integers
    - binary search to find upper/lower bounds for a game G on scale S

### Version 1.2 - simplification rules for `game` and `sumgame`
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


