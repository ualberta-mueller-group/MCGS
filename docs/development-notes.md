# Development notes
This document includes more detailed information than `README.md`, including design choices and tradeoffs, version history, and implementation details.


# Search and Solving a Game
- Two classes implement game solving: `alternating_move_game` and `sumgame`
- `alternating_move_game` is used for solving a single game, without splitting it into a sum of subgames
    - `alternating_move_game::solve` is a basic boolean negamax search
- `sumgame` is used to store and solve a sum of games. 
    - It is derived from `alternating_move_game`
        - Reimplements `solve` method
    - Boolean negamax search, with optimizations
        - Splits a subgame into more subgames after playing a move in it
        - Simplifies "basic" CGT games
    - `sumgame::_solve_with_timeout`
        - `private` method implements most of the search algorithm
        - Runs until it either completes, or times out
        - A timeout of 0 means infinite time
        - All public `solve` methods within `sumgame` (see below) are implemented in terms of this method
        - In the future we could use a macro to check if the result of a recursive call timed out (wrap recursive solve calls with some `CHECK` macro which will return from the function if the result is invalid)
    - `sumgame::solve_with_timeout`
        - Currently spawns a thread which calls `_solve_with_timeout`, and the main thread blocks until completion or timeout.
        - This may interfere with some performance profiling tools, but other implementations based on checking a clock seem to be costly.
    - `sumgame::solve`
        - runs until completion, without timing out
    - `sumgame::solve_with_games`
        - Temporarily adds `games` to the sum, calls `sumgame::solve`, then removes `games` and returns the result.
        - Useful for playing a difference game

## "Logically `const`" Interface for Solving Games
- In both `alternating_move_game` and `sumgame`, the public `solve` methods are declared as `const`.
- This means that while the game state may be modified during solve,
it must be restored before the end of `solve` in any case, including timeout or other failure modes.
- `class assert_restore_alternating_game` in `alternating_move_game.h` is a stub for
checking that the state is restored
    - a naive first implementation just checks the length of the move stack
    - TODO it probably is broken for sumgame, since it uses a different stack
    - TODO this check will be made functional after hash codes are implemented

# Impartial Games
- Impartial games support added in version 1.2
- Main differences between `impartial_game` and `game`:
    1. `play(m)` does not take a color argument
    2. `move_generator` does not take a color argument
    3. Completely different solving algorithms:
        - Evaluate any impartial game to a nim value
        - Can evaluate a sum one subgame at a time, no need
          for "global search" of sum - just compute all nimbers
          and nim-add them
        - (implemented) brute force algorithm using mex rule
        - (not yet) Lemoine and Viennot
        - (not yet) Beling and Rogalski
    4. Since `impartial_game` is a subclass of `game`,
       such games can still be searched in minimax fashion 
       as part of a regular `sumgame` - see the tests 
       in `impartial_sumgame_test`
- New game `kayles` - a simple and solved impartial game
- Impartial game wrapper `impartial_game_wrapper`
    - Allows any (partizan) game to be played in an 
      impartial way
    - Both players can play the moves of both BLACK and WHITE
      in the underlying game
    - In minimax search of such a game, the player making
      the move in the impartial game may be different from the player
      making a move in the wrapped game
    - Test cases: impartial clobber, using the wrapper 
      for `clobber_1xn` and comparison with Dai and Chen's 
      results
- Impartial sum game, `impartial_sumgame` - solve a sum 
  of impartial games

# Safe Arithmetic Functions (`safe_arithmetic.h`)
This section uses the term "wrapping" to mean either underflow or overflow.

- Defines template functions to do arithmetic without wrapping
- Assumes underlying machine uses two's complement to represent integers (there's a `static_assert` to check this)
- All functions return a `bool`
    - When `true`, the operation was completed without wrapping
    - When `false`, the operation would have wrapped. No operands were changed
        - `false` is also returned on invalid arguments, i.e. negative bit shift amounts
- Functions accept different types
    - `int` denotes any integral type
    - `num` denotes any integral or floating point type
    - Both unsigned and signed types, unless stated otherwise

These functions test whether an operation would wrap, without doing the operation:
- `add_is_safe(const num x, const num y)`
    - `true` iff `x + y` won't wrap
- `subtract_is_safe(const num x, const num y)`
    - `true` iff `x - y` won't wrap
- `negate_is_safe(const signed int x)`
    - `true` iff `-x` won't wrap

These functions perform operations, and will only change the operands on success:
- `safe_add(num& x, const num y)`
    - `x := x + y`
- `safe_add_negatable(signed int& x, const signed int y)`
    - `x := x + y`, also fails if negating the resulting `x` would wrap (i.e. `-x`)
- `safe_subtract(num& x, const num y)`
    - `x := x - y`
- `safe_subtract_negatable(signed int& x, const signed int y)`
    - `x := x - y` also fails if negating the resulting `x` would wrap
- `safe_negate(signed int& x)`
    - `x := -x`
- `safe_mul2_shift(signed int& x, const int exponent)`
    - `x := x * 2^exponent` (implemented as left shift)
    - Negative values of `x` are allowed
    - also `false` when the operation would flip the sign
    - On success, the resulting `x` is also negatable without wrapping
- `safe_pow2_mod(signed int& x, const signed int pow2)`
    - `x := x % pow2` (implemented as bitwise `&`)
    - `false` if `pow2` is not a power of 2, or `pow2 <= 0`

## safe_int<T> (future work?)
This is not implemented, but described as a possible future task.
- The user of safe arithmetic functions must still be careful to avoid errors
- Ideally we would have a `safe_int<T>` type, for signed integral types
    - Has two private fields: `T _value` and `bool _is_valid`
    - Has range of `[T_MIN + 1, T_MAX]`, assuming `T_MIN + 1 == -T_MAX`
    - Operations causing overflow will invalidate the result
    - Operations using invalid operands give an invalid result
    - Reading the value of an invalid result throws an exception
        - Programmer has to check `is_valid()` method at the end of computation before reading value, to avoid an exception
        - Ensures correctness; if no exception was thrown, the program is correct
- Have a macro `CHECK_SAFE_INT(x, code)`
    - If `x` is not valid, run `code` (allow cleanup and return from function)

# RTTI - Run-time type information (`game_type.h`)
- Defines interface class `i_game_type`
    - Currently implements method `game_type_t game_type() const` ("concrete" non-virtual method)
    - Also implements template function `game_type_t game_type<T>()`
    - `game_type_t` is an unsigned integral type with a unique value for every class inheriting from `i_game_type`
    - Both the template and method versions give the same result for the same `T`
        - The value is determined at run-time, and is dependent on the order of `game_type()` calls
    - Uses built-in C++ RTTI (`std::type_info` and `std::type_index`) to look up value in a `std::unordered_map`
        - Template version is faster as it stores this value in a static variable after the first map lookup
    - `game_type_t` is only defined for "concrete" games (non-abstract classes derived from `game`).
        - `game_type<T>()` fails a static assert if `T` doesn't satisfy this condition
        - `game_type() const` already satisfies this as it's not possible to instantiate an abstract type
- `game.h` defines template `T* cast_game<T*>(game*)` acting as a `reinterpret_cast`, but uses
`assert`s to verify that the game is not `nullptr`, is active, and is of type `T` (using its `game_type_t`)

# More on data types

## `sumgame_move` struct (`sumgame.h`)
Represents a move made in a `sumgame`
- Contains a subgame index and the `move` made in the subgame
- The index is into the vector `sumgame::_subgames`
- The `move` is for the subgame stored there

## `play_record` struct (`sumgame.h`)
- Holds information about a `sumgame_move` played in a `sumgame`
    - the `sumgame_move` itself
    - whether or not the move resulted in a split
    - which subgames were created from the split

## `sumgame_impl::change_record` class (`sumgame_change_record.h`)
- Similar to `play_record`; used to track changes to `sumgame` (i.e. by sumgame simplification steps), to allow undoing of changes
- Holds 2 `vector<game*>`s: one for deactivated games, and one for added games
- Undo operation first reactivates games, then pops games from `sumgame` and deletes them

## `sumgame_map_view` class (`sumgame_map_view.h`)
- Sorts all `game` objects contained by a `sumgame` using their `game_type_t`, acting as a map from game type to `vector<game*>&`
- Has public methods to mutate underlying `sumgame` while keeping it synchronized with the map view
    - These mutations are stored in a `change_record`
    - i.e. `sumgame_map_view::deactivate_game(game*)`, `sumgame_map_view::add_game(game*)`
- Only games with `is_active() == true` are kept in the map
- The map view remains valid only while the programmer interacts with the underlying `sumgame` through `sumgame_map_view`'s interface

## `sumgame::undo_stack_unwinder` class (`sumgame_undo_stack_unwinder.h`)
- Private inner class of `sumgame`
- Created as local variable at the start of `sumgame::_solve_with_timeout()`
    - Constructor pushes a marker onto `sumgame`'s undo stack
    - Destructor iteratively pops undo stack, calling undo functions (i.e. `sumgame::undo_move()`, `sumgame::undo_simplify_basic()`) until it sees the marker

## `sumgame` class (`sumgame.h`)
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
- Has an "undo stack" consisting of multiple `vector`s
    - `vector<sumgame_undo_code>`
        - Stack of enum type indicating which undo functions need to be called before returning from the current minimax search step
        - Contains markers, `SUMGAME_UNDO_STACK_FRAME`, pushed at the start of each minimax search step
    - `vector<play_record>`
        - To undo moves played in a sumgame
    - `vector<sumgame_impl::change_record>`
        - To undo other mutations to the `sumgame` i.e. by game simplification steps
    - These are stored as `vector<T>` instead of `vector<T*>` with the aim of reducing CPU cache misses
        - This seems to be slightly faster, but we should re-verify this when significantly changing the undo stack

## `fraction` class (`fraction.h`)
- Simple and lightweight fraction type whose denominator must be a positive power of 2
    - Consists only of 2 private `int`s: `_top` and `_bottom`, making copying fast
        - These `int`s must be within the interval `[INT_MIN + 1, INT_MAX]`, ensuring values are negatable without overflow
        - This range is enforced by the constructor and arithmetic operations
    - Has comparison operators (`<`, `<=`, `==`, `!=`, `>=`, `>`, `fraction::get_relation()`)
        - These will never fail, but are significantly more expensive than `int` comparisons due to needing to make operands compatible (have the same denominator)
        - `fraction(1, 2) == fraction(2, 4)`
    - Has operations which will never fail
        - Negate
        - Simplify
        - Get (and optionally remove) integral part as an `int` (whose denominator is assumed to be 1)
    - Has safe operations which may fail and return false, if they would underflow or overflow (similar to `safe_arithmetic.h`)
        - On failure, `fraction` operands may be left more (or less) simplified, but will still be equivalent to before the operation (i.e. according to `==`)
        - `fraction::safe_add_fraction(fraction& x, fraction& y)` and `fraction::safe_substract_fraction(fraction& x, fraction& y)`
            - If safe, do `x := x + y` or `x := x - y`
        - Make `fraction`s compatible
        - Raise denominator
            - To target value
            - By number of left bit shifts
        - Multiply bottom by power of 2

`fraction`s are more likely than integers to encounter arithmetic overflow, as raising the denominator by a factor of 2 halves the magnitude of representable values. `fraction`s don't store separate integral and fractional components, as this would complicate implementation: `mul2_bottom()` (effectively a division by a power of 2) would also need to operate on the integral component, which may not be divisible by 2.

## `switch_game` class (`cgt_switch.h`)
- Represents a game `g`, `g = {X | Y}` where `X` and `Y` are fractions
    - `X` and `Y` are represented by the `fraction` class
- Has 4 "kinds" represented by an enum, accessible by `switch_game::kind()`
    - `SWITCH_KIND_PROPER`
        - A true switch (i.e. `X > Y`)
    - `SWITCH_KIND_PROPER_NORMALIZED`
        - A true switch, also normalized (i.e. `g = {A | -A}`, where `A` is a fraction and `A` > `-A`)
    - `SWITCH_KIND_RATIONAL`
        - A `switch_game` after a move has been played by either player
        - After a move has been played, the underlying kind is inaccessible, until moves are undone and the game becomes a switch again
    - `SWITCH_KIND_CONVERTIBLE_NUMBER`
        - A `switch_game` for which `X <= Y`
        - Here `g` is equal to a number -- a `dyadic_rational` and possibly a star (as an `up_star`)
- Kinds are computed during construction using `fraction::get_relation()`
    - TODO: perhaps we should evaluate this lazily?

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

# Bounds (`bounds.h`)
Defines functions and types used for finding lower and upper bounds of games.

- The `bound_scale` enum represents a scale of games on which bounds are found.
- `bound_t` is a signed integral type representing an index along a bound scale.
- The functions `get_scale_game()` and `get_inverse_scale_game()` return a new `game` object for a given scale and index along the scale.
- The `relation` enum, (`cgt_basics.h`), is used to represent how a game relates to its bounds
- The `bounds_options` struct specifies a scale and interval on which bounds should be searched for.

Some scales and their games at select indices are shown in the following table:
| Scale | -2 | -1 | 0 | 1 | 2 |
| --- | --- | --- | --- | --- | --- |
| `up_star` | vv* | v* | * | ^* | ^^* |
| up | vv | v | 0 | ^ | ^^ |
| `dyadic_rational` | -2/8 | -1/8 | 0 | 1/8 | 2/8 |

## `game_bounds` class
A pair of lower and upper bounds for a sumgame `S` is represented by the `game_bounds` class. Each bound within is represented by a `bound_t` indicating its location along a scale, a `bool` indicating whether it's valid or invalid, and a `relation` indicating its relation to `S`. The `relation` is interpreted with the bound game `G` being on the left hand side, i.e. `G REL_LESS_OR_EQUAL S`, or `G REL_GREATER S` to denote `G <= S` and `G > S` respectively.

For serializaton purposes, it may be better to encode data for each bound into one `bound_t` using bitmasks, instead of having a `bound_t`, `bool`, and `relation`.

## `find_bounds()` function
`find_bounds()` computes both bounds for a `sumgame`, or `vector<game*>`, or `game*`. It takes a `vector` of one or more `bounds_options` structs, and returns a `vector` containing one `game_bounds` object for each `bounds_options`. The actual return type is `vector<game_bounds_ptr>` (`vector<shared_ptr<game_bounds>>`)

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

# Sumgame Simplification (cgt_game_simplification.h)
`sumgame::simplify_basic()` simplifies the sumgame by summing together basic CGT games and simplifying switches, (and `sumgame::undo_simplify_basic()` undoes this). Each step handles a different game type. Run-time type information is used to distinguish game types. To ensure that new games produced by a previous step may be included in the next step, steps happen in the following order:

1. `nimber`
2. `switch_game`
3. `up_star`
4. `integer_game` and `dyadic_rational` together

Steps producing no useful simplification will not modify the `sumgame`. i.e. if the `sumgame` has only one non-zero `up_star` game, the game will be left alone, rather than duplicated with one inactive copy.

These steps are run at the start of `sumgame::_solve_with_timeout()`, when:
- At the root of minimax search
- A "basic" CGT game has been added
- After undoing a simplification
    - Calls to `simplify_basic()` which don't change the `sumgame` don't trigger an undo


## `nimber` Simplification
- All `nimber`s are summed together using `nimber::nim_sum()`
- This step only has an effect if the sum contains at least 2 nimbers, or contains 1 nimber with `value() <= 1`
- May add 1 `nimber`, or 1 star (as `up_star`), or nothing
- Overflow is not a concern, as nimber addition is an XOR operation

## `switch_game` Simplification
`switch_game`s are of the form `{X | Y}` for some rationals `X` and `Y`.

First, all `switch_game`s are separated based on their `switch_kind`. Only `SWITCH_KIND_PROPER` and `SWITCH_KIND_CONVERTIBLE_NUMBER` are used. `SWITCH_KIND_RATIONAL` and `SWITCH_KIND_PROPER_NORMALIZED` are left alone. There are two major cases (with some subcases), described in the next subsections.

Note that operations involve the `fraction` class, and when arithmetic overflow would have occurred during simplification of a given `switch_game` object, this particular `switch_game` object is skipped and left untouched.

### Proper switch
Proper switches are `switch_game`s where `X > Y`. These games are normalized to produce a sum `M + {A | -A}`, for fractions `M` and `A`, where `M` is the mean of `X` and `Y`, and `A > -A`. `M` is added to the `sumgame` as a `dyadic_rational`, and `{A | -A}` is added to the `sumgame` as a `switch_game`.

This step only happens if there is at least one `integer_game` or `dyadic_rational` in the sum. This is to avoid the problem of switch normalization increasing the number of moves that can be played at a search step, leading to worse performance than if basic CGT simplification was disabled.

### Convertible number
Switches representing numbers are `switch_game`s where `X <= Y`. Several subcases occur:

- If `X < 0` and `Y > 0`, the game is 0 (so it is just deactivated)
- If `X == Y`, the game is replaced with `X + *` (a `dyadic_rational` and `up_star`)
- Otherwise the game is replaced by the "simplest" `dyadic_rational` `U`, `X < U < Y`

The simplest such value is the unique rational `U = i/2^j` for some integers `i` and `j`, `j >= 0`, having minimal `j`, or if `j == 0`, having `i` with smallest absolute value. This value is found by iteratively increasing `j` and checking if some multiple of `1/2^j` occurs between `X` and `Y`, and if it does, the specific multiple representing `U` is then found.

## `integer_game` and `dyadic_rational` Simplification
All `integer_game`s are summed together, then all `dyadic_rational`s are summed together. The two resulting sums are then summed into a combined sum. Within each of these 3 summations, summands which would cause overflow are skipped.

For each sum, no useful work was done if the sum is the result of less than 2 games, and the sum is non-zero. Otherwise the sum is useful.
- If the combined sum is useful, it replaces all games used to produce the sum
- Otherwise, the integer and rational sums individually replace the games used to produce them (only when the sums are useful)

## `up_star` Simplification
`up_star`s are summed together similarly to `integer_game`s and `dyadic_rational`s, and the resulting sum replaces the games used to produce it, only when the sum is useful (as explained in the previous subsection).

# Misc Future Optimizations
- We should be able to compile with `-DNDEBUG` at some point
    - This causes compiler warnings because of unused variables
    - There's a clang-tidy check, `bugprone-assert-side-effect`, which can ensure
    that none of our asserts have side effects.
    - But, it should have the `CheckFunctionCalls` option enabled, which disallows
    calls to all non-const functions (including global utility functions)
    - Possible run-time errors should use exceptions, not asserts (i.e. constructing)
    an invalid game due to user input.
    - `THROW_ASSERT(condition)` and `THROW_ASSERT(condition, exception)` are macros
    defined by `throw_assert.h`, which throw exceptions, and can replace asserts
    where run-time errors may occur.
- `game_type()` could possibly be faster if we eliminated the `unordered_map` lookup

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
- This is a consequence of the design choice above: `move` must be an `int`
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

### Version 1.1 completed 
- More cleanup and Tools and Components for Database
    - Code formatting tools and checks, "lint"-like
    - `scale` S such as multiples of up, or up+star, or integers
        - binary search to find upper/lower bounds for a game G on scale S
        - simplify `game` G in `sumgame` S
            - compare with/without subgame split
- simplify games of same type in S, e.g.
    - add up integers/rationals
    - add ups+stars
    - add nimbers

## Future: Smaller Step Versions 1.x to Prepare for Version 2
- change read from string functions to directly create sumgame
    - this would require `sumgame` to be the sole owner of its games
    - OR, the `game_case` produced by `file_parser::parse_chunk` could own 
    the games in a `vector<unique_ptr<game>>`
- compare G with a simpler game H, replace in S if equal
- compare G1+G2 with a simpler H, replace in S if equal

### Version 2
- general sum simplifications
    - remove/deactivate 0
    - find inverse pairs and deactivate

