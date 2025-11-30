# Development notes
This document includes more detailed information than `README.md`, including design choices and tradeoffs, version history, and implementation details.

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
# Table of Contents

- [Search and Solving a Game](#search-and-solving-a-game)
- [More On Data Types](#more-on-data-types)
- [More On Extending the `game` Class](#more-on-extending-the-game-class)
- [Impartial Games](#impartial-games)
- [Global Options (`global_options.h`)](#global-options-global_optionsh)
- [Initialization (`mcgs_init.h`)](#initialization-mcgs_inith)
- [Random (`random.h`)](#random-randomh)
- [Hashing (`hashing.h`)](#hashing-hashingh)
- [Adding Hashing To Games](#adding-hashing-to-games)
- [Transposition Tables (`transposition.h`)](#transposition-tables-transpositionh)
- [Database (`database.h`, `global_database.h`)](#database-databaseh-global_databaseh)
- [Adding A Game To the Database](#adding-a-game-to-the-database)
- [Safe Arithmetic Functions (`safe_arithmetic.h`)](#safe-arithmetic-functions-safe_arithmetich)
- [RTTI - Run-time type information (`type_table.h`)](#rtti---run-time-type-information-type_tableh)
- [Sumgame Simplification (cgt_game_simplification.h)](#sumgame-simplification-cgt_game_simplificationh)
- [Bounds (`bounds.h`)](#bounds-boundsh)
- [Serialization (`iobuffer.h`, `serializer.h`, `dynamic_serializable.h`)](#serialization-iobufferh-serializerh-dynamic_serializableh)
- [Outstanding Issues](#outstanding-issues)
- [Design Choices and Remaining Uglinesses](#design-choices-and-remaining-uglinesses)
- [Misc Future Optimizations](#misc-future-optimizations)
- [Release Procedure](#release-procedure)
- [Versions](#versions)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# New Stuff (TODO move me)
## grid_hash (`grid_hash.h`)
The `grid_hash` class is used to compute local hashes for grid games which have
rotational and/or transpose symmetry. It owns and manages 8 `local_hash`
objects, one for each possible orientation of a grid. It computes the local
hashes for all specified orientations of a grid, and takes the minimum as the
final value.

For example, given the first Clobber board in the list below, its local hash
is computed along with those for the 7 boards below it, and the final hash
value is defined to be the minimum of all 8:
- "XO.|.X."
- ".X|XO|.."
- ".X.|.OX"
- "..|OX|X."
- "X.|OX|.."
- ".X.|XO."
- "..|XO|.X"
- ".OX|.X."

- Construct a `grid_hash` with a bit mask indicating which orientations of
  grids should be mapped to the same hash values.
    - Indices of bits in the mask are specified by the `grid_hash_orientation`
      enum.
    - The name of each enum value indicates a clockwise rotation in degrees,
      optionally followed by a transpose operation. i.e.
      `GRID_HASH_ORIENTATION_180T` denotes a 180 degree clockwise rotation
      followed by a transpose.
    - Common grid hash masks are provided by `grid_hash.h` as constants.
        - `GRID_HASH_ACTIVE_MASK_IDENTITY` indicates that a grid has no
          symmetry. It is equivalent to computing local hashes
          without using a `grid_hash`, and is used by no games.
        - `GRID_HASH_ACTIVE_MASK_ALL` indicates that all 8 orientations are
          equivalent. Used by Clobber, NoGo, and Amazons.
        - `GRID_HASH_ACTIVE_MASK_MIRRORS` indicates that boards obtained
          through vertical and/or horizontal mirroring are equivalent. Used by
          Domineering and Fission.

- There are two ways to compute the local hash of a grid game using
  `grid_hash`. Do one of the following two options and then call the
  `grid_hash::get_value()` method to get the final local hash value, then
  call the `local_hash::__set_value()` method with this hash value.
   1. Call the `grid_hash::init_from_board_and_type<T>()` template method.
       - Template parameter `T` denotes the type of element inside the grid's
         board. For most games, `T` is `int`.
       - Method arguments are the board contents (in a flattened row-major
         representation), board dimensions, and the `game_type_t` of the grid
         game.
   2. Manually reimplement option 1 if your game's board is split across
      several `vector`s. NoGo does this because each board location has a stone
      color and metadata indicating stones which are "immortal", and these
      are stored in separate `vector`s of the same size. Steps:
      1. Call the `grid_hash::reset` method with the grid dimensions of your
         game.
      2. Call the `grid_hash::toggle_type` method with the `game_type_t` of your
         game.
      3. Call the `grid_hash::toggle_value` method for each element in the grid,
         indicating the (0-indexed) row and column coordinates in the original
         grid.

- NOTE: grid hash masks must have the `GRID_HASH_ORIENTATION_0` bit set to 1 (a
  board must be equivalent to itself), AND the set of masked orientations must
  be "complete" in that every masked orientation must result in another masked
  orientation following any of the masked transformations. For example, a mask
  with only `0` and `90` is illegal, because `90` requires `180` and `270` to
  be set as well. There is currently no assertion to check for the latter of
  these two conditions.

## grid_mask (`grid_mask.h`)
A `grid_mask` provides a bit mask over grids, and a way to iterate over all
such masks for given fixed grid dimensions.

- Construct a `grid_mask` with a grid hash mask indicating symmetries to prune.

- The `set_shape()` method must be called with the grid dimensions of the
  masks you want to generate.

- A mask is skipped if its `grid_hash` value matches a previously generated
  mask. Use the `reset()` method to forget all previously generated masks.
  - Calling `set_shape(int_pair(3, 3))` and iterating over all 3x3 masks, then
    attempting the same process without first calling `reset()`, will produce no
    masks.

- The `get_mask()` method returns a const reference to the current mask, in a
  flattened row-major representation.

- Use `operator bool` to see if the current state is valid, and `operator++` to
  go to the next mask.
  - All masks with `N` `true` bits are generated before generating all masks
    with `N+1` `true` bits.

Example mask sequence when constructing with the grid hash mask
`GRID_HASH_ACTIVE_MASK_ALL` and calling `set_shape(int_pair(2, 2))`:
- `[0, 0, 0, 0]`
- `[1, 0, 0, 0]`
- skipped: `[0, 1, 0, 0]`
- skipped: `[0, 0, 1, 0]`
- skipped: `[0, 0, 0, 1]`
- `[1, 1, 0, 0]`
- skipped: `[1, 0, 1, 0]`
- `[1, 0, 0, 1]`
- skipped: `[0, 1, 1, 0]`
- skipped: `[0, 1, 0, 1]`
- skipped: `[0, 0, 1, 1]`
- `[1, 1, 1, 0]`
- skipped: `[1, 1, 0, 1]`
- skipped: `[1, 0, 1, 1]`
- skipped: `[0, 1, 1, 1]`
- `[1, 1, 1, 1]`

If `GRID_HASH_ACTIVE_MASK_IDENTITY` is used instead, the skipped masks will
be generated too.

## grid_generator (`grid_generator.h`)
The `i_grid_generator` interface class is similar to other generators in MCGS,
i.e. `move_generator`, and is used for generating all strips or grids up to some
maximum size. Use `operator bool` to see if the current state is valid,
and methods `gen_board()` and `get_shape()` to get the current grid contents
and dimensions (in a flattened row-major format), and use `operator++` to
advance to the next grid.

- `only_strips()` method indicates whether or not the generator will only
  produce boards having at most 1 row.
- The interface class defines `static` helper functions for incrementing grid
  dimensions. See comments in `grid_generator.h`.

- Two classes implement the `i_grid_generator` interface: `grid_generator`
  (`grid_generator.h`), and `sheep_grid_generator` (`sheep_grid_generator.h`).

`grid_generator` has two constructors with several arguments:
- Arguments common to both constructors:
    - `const int_pair& max_dims` dimensions of largest grid to generate. For
      non-strips, the order of dimensions does not matter, i.e. `{2, 3}`
      produces the same behavior as `{3, 2}`. Dimension increments may either
      swap the current dimensions, or increment one of them (unless
      `strips_only` is `true`, in which case the row dimension is always 1 after
      the initial 0x0 board).
    - `const std::vector<int>& tile_sequence` sequence of colors for each grid
      location.
    - `bool strips_only` indicates whether only strips should be generated, or
      both strips and grids.
- The 2nd constructor enables usage of a `grid_mask`, which serves two purposes:
  to generate grids in order of increasing/decreasing number of stones, and to
  prune (some) grids using symmetry.
    - `bool mask_active_bit` indicates the value of `grid_mask` bit which the
      `tile_sequence` applies to. For example, to generate Clobber games in
      order of increasing number of stones, this should be `true`, and to generate
      NoGo games in order of decreasing number of stones, this should be `false`.
    - `int mask_inactive_tile` the color applied to the `grid_mask` bit value
      opposite of `mask_active_bit`.
    - `unsigned int grid_hash_mask` the symmetry used by the `grid_mask`


Example output for `grid_generator` using first constructor:
```
// max_dims = {1, 2}
// tile_sequence = {EMPTY, BLACK, WHITE}
// only_strips = false
""
"."
"X"
"O"
".."
".X"
".O"
"X."
"XX"
"XO"
"O."
"OX"
"OO"
".|."
".|X"
".|O"
"X|."
"X|X"
"X|O"
"O|."
"O|X"
"O|O"
```

Example output for `grid_generator` using second constructor. Lines marked
as "pruned" would be pruned if `GRID_HASH_ACTIVE_MASK_ALL` were used instead:
```
// max_dims = {1, 2}
// tile_sequece = {BLACK, WHITE}
// mask_active_bit = true
// mask_inactive_tile = EMPTY
// strips_only = false
// grid_hash_mask = GRID_HASH_ACTIVE_MASK_IDENTITY
""
"."
"X"
"O"
".."
"X."
"O."
".X" // pruned
".O" // pruned
"XX"
"XO"
"OX"
"OO"
".|." // pruned
"X|." // pruned
"O|." // pruned
".|X" // pruned
".|O" // pruned
"X|X" // pruned
"X|O" // pruned
"O|X" // pruned
"O|O" // pruned
```

## config_map (`config_map.h`)
`config_map` is a helper class for parsing values from the database config
string. It's constructed with a substring of the database config string for
a specific game to be generated. The substring's contents are a series of
0 or more key/value pairs. Key and value are separated by a '=' character, and
each pair ends with a mandatory ';' character.

- `get_XYZ(const string& key)` methods return a `std::optional` which holds a
  value IFF the value string corresponding to the key can be converted to the
  requested value.
  - For example, if the `config_map` was constructed with a string
    "some_key = 4;", then the call `get_int("some_key")` would return a
    `std::optional<int>` containing the integer `4`.
  - If instead the string were "some_key = abc;" or "some_other_key = 4;",
    `get_int("some_key")` would return a `std::optional<int>` holding no value.

- Method `void check_unused_keys()` throws if some key/value pair had no
  conversion attempted. Used to catch typos in the database config string.


# Search and Solving a Game
- Two classes implement minimax game solving: `alternating_move_game` and `sumgame`
- `alternating_move_game` is used for solving a single game, without splitting it into a sum of subgames
    - `alternating_move_game::solve` is a basic boolean negamax search
    - Does not use a transposition table
- `sumgame` is used to store and solve a sum of games.
    - It is derived from `alternating_move_game`
        - Reimplements `solve` method
    - Boolean negamax search, with optimizations
        - Splits a subgame into more subgames after playing a move in it
        - Normalizes subgames
        - Simplifies "basic" CGT games
        - Uses a transposition table
        - Uses a database of subgame outcomes
    - `sumgame::_solve_with_timeout`
        - `private` method implements most of the search algorithm
        - Runs until it either completes, or times out
        - A timeout of 0 means infinite time
        - All public `solve` methods within `sumgame` (see below) are implemented in terms of this method
    - `sumgame::solve_with_timeout`
        - Currently spawns a thread which calls `_solve_with_timeout`, and the main thread blocks until completion or timeout.
        - This may interfere with some performance profiling tools, but other implementations based on checking a clock seem to be costly.
        - Calls `sumgame::_pre_solve_pass()` before search, which calls `game::split()` on
        each active subgame, and calls `game::normalize()` on all active subgames
        (whether or not they are the result of a split)
    - `sumgame::solve`
        - runs until completion, without timing out
    - `sumgame::solve_with_games`
        - Temporarily adds `game`s to the sum, calls `sumgame::solve`, then removes `game`s and returns the result.
        - Useful for playing a difference game

    - Search postpones playing moves on `integer_game` and `dyadic_rational`
    games until there are no more moves to play on other subgames
    - Among duplicate subgames (as determined by comparing local hashes of
    games), only one copy yields moves, and the others are skipped

## "Logically `const`" Interface for Solving Games
- In both `alternating_move_game` and `sumgame`, the public `solve` methods are declared as `const`.
- This means that while the game state may be modified during solve,
it must be restored before the end of `solve` in any case, including timeout or other failure modes.
- Several "assert_restore_" types are defined, to ensure that game states are restored:
    - `class assert_restore_game` (`game.h`)
        - Ensures local_hash, and move/undo_code stack sizes are restored
    - `class assert_restore_alternating_game` (`alternating_move_game.h`)
        - Ensures to_play, and game hash (local_hash of contained game, or 0 if no game is contained) are restored
        - If a game is contained within, creates and owns an `assert_restore_game` for that game
    - `class assert_restore_sumgame` (`sumgame.h`)
        - Derives from `assert_restore_alternating_game`
        - Ensures global_hash, number of subgames (active or inactive), and undo_code/play_record/change_record stack sizes are all restored

## Sumgame's Usage Of the Database
The database stores outcome classes for single subgames. `sumgame` uses this
data, from the global database object (`get_global_database()` in
`global_database.h`), during solving, to terminate search early in some cases.

- `sumgame::simplify_db()` is called at the start of `sumgame::_solve_with_timeout()`
    - This method looks up every active subgame's outcome class in the database
    - P positions are deactivated, and all other outcome classes are counted
    - The winner of the current game is known without further search when all
    subgames' outcome classes are known, and one of the following holds
    (after omitting P positions):

        - All outcome classes are `L`, or all outcome classes are `R`
        - Exactly one outcome class is `N`, and no outcome classes are "negative"
        for current player
            - For `BLACK` to play, this means there are no `R` outcome classes
            - For `WHITE` to play, this means there are no `L` outcome classes

## Solver Stats
During search, `sumgame::_solve_with_timeout()` records events using the global
instance of the `solver_stats` class (defined in `solver_stats.h`).

This includes:
- Node count (calls to `sumgame::_solve_with_timeout`)
- Transposition table lookup hit/miss count
- Database lookup hit/miss count
- Maximum search depth

TODO: These are currently only printed to csv output for the experiments in the paper,
and are discarded for other tests i.e. `./MCGS --run-tests`

# More On Data Types

## `cli_options` struct (`cli_options.h`)
- Holds values resulting from parsing command-line arguments
- Returned by function `parse_args`

## `sumgame_move` struct (`sumgame.h`)
- Represents a move made in a `sumgame`
- Contains a subgame index and the `move` made in the subgame
- The index is into the vector `sumgame::_subgames`
- The `move` is for the subgame stored there

## `game_type_t` integral type (`type_table.h`)
- An integer with a unique value for every game class
- Acessible by method or template function:
    - Method `game_type_t game::game_type() const`
    - Template function `game_type_t game_type<T>()` for non-abstract game type `T`
        - i.e. `game_type<clobber>()`
- Used to distinguish between game types, i.e. in a `vector<game*>`
- Value is assigned at run time, and depends on the order of `game_type()` calls
since the program's start

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
    - Destructor iteratively pops undo stack, calling undo functions until it sees the marker i.e:
    - `sumgame::undo_move()`
    - `sumgame::undo_simplify_basic()`
    - `sumgame::undo_simplify_db()`

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

## `strip` class (`strip.h`)
- Abstract class derived from `game`. Used for games played on a `1xN` strip
- Provides default implementations for:
    - `game::_init_hash()` (hash derived from only the board state)
    - `game::_normalize_impl()` and `game::_undo_normalize_impl()` (reverse/un-reverse board based on lexicographical ordering)
        - This assumes that reversing the board doesn't change the value of the game
    - `game::_order_impl()` (lexicographical ordering)

## `grid` class (`grid.h`)
- Abstract class derived from `game`. Used for games played on an `MxN` grid. Analogous to `strip`
- Row-major format. Coordinates and dimensions are `(row, column)` pairs
    - Represented as `int_pair` (typedef for `std::pair<int, int>`, defined in the header)
- Implements `game::_init_hash()` and `game::_order_impl()` analogously to `strip`
- Does not implement `game::_normalize_impl()` or `game::_undo_normalize_impl()`
    - Grid games should manage their own `grid_hash` objects to map equivalent
      (by symmetry) boards to the same local hash values.

## `grid_location` class (`grid_location.h`)
- Utility class for manipulating locations on a `grid`
    - Internally uses "coordinate" (`int_pair`) representation instead of "point" (`int`) representation.
        - "Point" representation corresponds to the 1D indices of the flattened 2D grid
        - "Coordinate" representation is a `(row, column)` pair. This is invariant to width changes, unlike "point" representation
        - Constructible with shape, and one of: coordinates (`int_pair`), a point (`int`), or no location (defaults to `(0, 0)`)
    - Has some ugly logic for accomodating `0x0` grids. Detailed below
        - TODO: perhaps this logic is confusing... Maybe remove some features until this goes away?
- May represent a valid or invalid state. Check with `valid()` method
- The state is valid IFF the current location is within the shape
- Shape `0x0` is legal during construction. Shapes with negative dimensions are illegal and will raise exceptions
    - Shape `0x0` is always invalid (but legal to construct)
    - Constructing with shape `0x0` always sets the location to `(0, 0)`, regardless of given coordinates or point
        - This is to be consistent with constructing from a point. Points can't be converted to coordinates if the shape is empty (this would require dividing by 0)

- From a valid state, an operation is legal IFF it results in a valid state
    - Except, `increment_position()` may result in an invalid state
    - This means that `set_shape({0, 0})` is illegal
- From an invalid state, the only legal operations are:
    - `valid()`, `is_empty()`, `get_shape()`, and any single mutation resulting in a valid state
- `increment_position()` is used to iterate over grid locations. May produce an invalid state
- `move()` moves the location in a direction. May fail and return `false`, leaving the grid_location unchanged
- `get_neighbor_coord()` and `get_neighbor_point()` produce a neighbor given some direction, leaving the `grid_location` unchanged. May fail and return `false`
- Many methods are implemented in terms of static class functions. Maybe confusing
as there are several similar looking functions?

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



# More On Extending the `game` Class
- In every game `x`'s implementation:
    - `x::play()` must immediately call `game::play()`
        - For impartial games, call `impartial_game::play()` instead
    - `x::undo_move()` must immediately call `last_move()` and then `game::undo_move()`
- Move generators are accessible only through the abstract game interface `create_move_generator`
    - Generators are dynamically allocated - wrap each use in a `std::unique_ptr`
    - An example is in `alternating_move_game::_solve`
    - A game-specific move generator is declared and used only in `x.cpp`, not in a header file
    - Moves returned by a `move_generator` must not use the color bit of the `move`
        - The exception to this is `impartial_game_wrapper`'s move generator
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
and is stored in the move stack. This is handled automatically by `game::play()`,
and affects the `move` returned by `game::last_move()`
- `undo_move` must respect and use the move player color information in the stack.

# Impartial Games
- Impartial games support added in version 1.2
- Main differences between `impartial_game` and `game`:
    1. `play(m)` does not take a color argument
        - Must still implement both `play(m)` and `play(m, to_play)`, possibly
        with one calling the other
        - Instead of calling `game::play()` at the start,
        call `impartial_game::play()`
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
    - Its move generator `ig_wrapper_move_generator` uses the color bit of
    moves it generates to encode the color of the move played on the wrapped
    game. Other move generators must not use this bit
    - It's illegal to wrap an already impartial game. An assert checks this
    - Assumes that the wrapped `game`'s `_split_impl()`, `_normalize_impl()`,
    and `_undo_normalize_impl()` methods all behave "nicely". They must not change
    the available moves, as this will change the nim value of the wrapper
        - i.e. `^*` must not split into `^` and `*`, because the combined game
        is in canonical form
        - i.e. `{1/4 | 2}` must not normalize to `1`

- `impartial_sumgame.h` defines functions for solving sums of impartial games.
Two functions do this: `search_impartial_sumgame` and
`search_impartial_sumgame_with_timeout`. The latter uses a timeout just like
`sumgame`'s `solve_with_timeout`
    - These functions use a persistent transposition table, just like `sumgame`,
    but both tables are independent from each other

# Global Options (`global_options.h`)
This file defines the `global_option` class, representing a global variable
which is part of MCGS's configuration.

Specific global options are also defined in this file, including those necessary to
reproduce experiments (i.e. random seeds, table sizes), but also more general
global options such as debug logging level. Many global options are used to
initialize core data structures of MCGS.

A `global_option` may optionally be included in the configuration summary
printed by `./MCGS --print-optimizations`.

To create a new global option, first declare it at the bottom of
`global_options.h`:
```
namespace global {
extern global_option<double> some_variable;
}
```
Then define it at the bottom of `global_options.cpp` (preferably using one of
the macros defined there):
```
namespace global {
// WILL be printed by config summary
INIT_GLOBAL_WITH_SUMMARY(some_variable, double, 4.5);

// or

// WILL NOT be printed by config summary
INIT_GLOBAL_WITHOUT_SUMMARY(some_variable, double, 4.5);
}
```

A `global_option` has a `_name` field, which may show in the config summary, and
is also used to define the value of `global_option::flag()`, a method giving
the flag which should be used to set the option from the command line.
The macros at the bottom of `global_options.cpp` initialize the `_name` field
to the name of the variable in the source code.

# Initialization (`mcgs_init.h`)
Executables based on the C++ source code must immediately call
`void mcgs_init_1()`, then (optionally) handle CLI options, and then call one of
`void mcgs_init_2()` or `void mcgs_init_2(const cli_options&)`. These functions
initialize core data structures and global variables. Global variables should
be initialized from one of these functions to avoid "Static Initialization Order
Fiasco" problems, as the initialization order of global variables in C++ is
undefined across translation units.

`mcgs_init_1` initializes lookup tables used for color/char conversions.

Things initialized by `mcgs_init_2` functions:
- `grid_hash` masks stored in grid games' `type_table_t` structs
- Serialization bookkeeping (for polymorphic types)
    - This is currently implemented but unused
- `random.h`'s random seed and global `random_generator`
- Global `random_table`s
- `sumgame`'s transposition table
- `impartial_sumgame.h`'s transposition table
- The global `database` is initialized, by loading `database.bin` or a specified
  database file (if exists)
- In the future, may assign `game_type_t`s to specific games, so that their
assignments are not dependent on input

# Random (`random.h`)
- Defines `random_generator` class
    - Constructor accepts a `uint64_t` seed. 0 is a valid seed with no
    special semantics
    - A global `random_generator` instance is accessible through `get_global_rng()`
        - `mcgs_init_all()` must be called first
        - Seeded by `global::random_seed`. If this is 0, the global
        `random_generator` is instead seeded with the current time by
        calling `ms_since_epoch()` (`utilities.h`)
    - Defines methods to get random numbers for different integral types
        - `get_uXX()`, `get_iXX()` where `XX` is one of `64`, `32`,
        `16`, `8`
        - `min` and `max` values are accepted as method arguments, with default
        values covering the whole range of the integral type
        - TODO should these exclude 0 by default?
- Method to access the underlying random number
generator, `std::mt19937_64& random_generator::get_rng()`

# Hashing (`hashing.h`)
Defines main data types for game hashing:
- `hash_t`
    - Typedef of `uint64_t`
    - Each game, or sum of games, produces a unique `hash_t` value
- `random_table` class
    - Table of random numbers used by Zobrist hashing
- `local_hash` class
    - Manages a `hash_t` for a single `game` object
- `global_hash` class
    - Manages a `hash_t` for a single `sumgame` object

## `random_table` Class
A `random_table` is constructed with two arguments: `n_positions`, and `seed`,
specifying how many positions (i.e. stones in a strip game) are
represented in the table, and the seed for the random numbers in the table. 0 is
an invalid seed.

A `random_table` is indexed via the `get_zobrist_val()` template method, by a pair
of integral values, (`position`, and `color`), returning a `hash_t`. `color` can
be any 1-16 byte (inclusive) integral value. If `position` is past the bounds
of the table, the table will grow and print a warning to stderr (only the first
time this happens), and an additional warning is printed to stderr after
completion of all tests. This resizing may affect validity of reported test
times, particularly for small tests, or when many resizes happen.

### Global `random_table`s
There are several global `random_table`s, initialized by `mcgs_init_all()`
(`mcgs_init.h`), and accessible via the `get_global_random_table(table_id)` function
(`hashing.h`). Each table is used for a different purpose, to avoid accidental
hash collisions.

Table IDs:
- `RANDOM_TABLE_DEFAULT`
    - Used for "state" of a game (i.e. stones in a strip, `int`s of a rational)
- `RANDOM_TABLE_TYPE`
    - Used for type of a game (its `game_type_t`)
- `RANDOM_TABLE_MODIFIER`
    - Used in computing `global_hash` values, to modify a game's `local_hash`
        based on its order in a `sumgame`
- `RANDOM_TABLE_PLAYER`
    - Used for color of current player to play

Throughout the rest of the `Hashing` section, table indexing is expressed by
the notation `TABLE_ID[position, color]`, but in the source code, table IDs are
enum values, and the actual tables are accessed by the `get_global_random_table(table_id)`
function.

### `random_table` Indexing and Hacks
All `random_table` objects represent all 1-16 byte `color` values for all
positions in the table, using a hack to generate corresponding `hash_t` values
by combining a smaller set of values stored in memory. Every position in a
`random_table` has a unique array of 256 random `hash_t` values, used to
generate the value returned by `get_zobrist_val()`.
Pseudocode below (using unsigned arithmetic):

```
// get_zobrist_val(position, color)

let "tables" be an array of arrays, where each sub-array belongs to one position and contains 256 random hash_t values

subtable = tables[position] // array of 256 hash_t for this position
result = 0
i = 0

do
    byte = (color >> (i * 8)) & 0xFF // logical right shift
    element = subtable[byte] // a hash_t
    element = rotate(element, 3 * i) // bitwise rotation
    result ^= element
    i = i + 1
while color has remaining non-zero bytes

return result
```

The actual "rotate" function used is `rotate_interleaved()` (`utilities.h`).
Bits masked by `0101...0101` are rotated left, and bits masked by `1010...1010`
are rotated right.

## `local_hash` Class
Manages the `hash_t` of a `game`.

Methods:
- `toggle_value<T>(size_t position, T color)`
    - XOR the current hash value with `RANDOM_TABLE_DEFAULT[position, color]`
- `toggle_type(game_type_t type)`
    - XOR the current hash value with `RANDOM_TABLE_TYPE[0, type]`
- `reset()`
    - Reset the hash to 0
- `get_value()`
    - Get the current `hash_t` value

## `global_hash` Class
Manages the `hash_t` of a `sumgame`.

### Global Hash Value Definition
`sumgame::get_global_hash()` computes the hash of a `sumgame`. The definition
of this value is described below.

To compute the `global_hash` value for a `sumgame` `S`:
1. Normalize each `game` of `S` by calling `game::normalize()`
2. Sort the `game`s of `S` according to their `get_local_hash()` values so that each `game` `g_i` has a subgame index `i`
3. For each `g_i`, get its `local_hash` value `h_i`
4. For each `h_i`, compute a modified hash `H_i := hmod(h_i, i)`, for some hash modifier function `hmod`
    - The actual `hmod` function is `H_i := RANDOM_TABLE_MODIFIER[i, h_i]`
    - Using a linear congruential generator instead doesn't seem to be significantly different in terms of performance
5. Given the player to play `p`, compute `P := RANDOM_TABLE_PLAYER[0, p]`
6. The `global_hash` value is the XOR of all `H_i`, and `P`

TODO: Currently `sumgame::get_global_hash()` sorts all of the `sumgame`'s games
every time it's called. `sumgame` could maintain an ordering of its games to
prevent this.
- NOTE: Something like this was tried, but didn't seem beneficial

### `global_hash` Methods
Methods:
- `add_subgame(size_t subgame_idx, game* g)`
    - Given `g_i`, get `h_i`, compute and store `H_i`, then XOR `H_i` into
        the current global hash value
- `remove_subgame(size_t subgame_idx, game* g)`
    - Given `g_i`, XOR the previously stored `H_i` out of the current global
        hash value
- `set_to_play(bw to_play)`
    - Given `p`, compute `P` and XOR it into the global hash value
    - If `p` was previously set, first XORs the previous `P` out of the global hash value
- `reset()`
    - Reset the global hash value to 0, and clear all stored `H_i` and `p`
- `get_value()`
    - Get the current `hash_t` value. Must first set `p`

# Adding Hashing To Games
Important:
1. A game's `play` method must call `game::play` at the START of the method
2. A game's `undo_move` method must call `game::undo_move` at the START of the method (after getting the last move from the stack)

This is because these base class methods do some record keeping around local hashes.

NOTE: For details on dealing with grid game symmetry, see TODO

## 1. Mandatory `_init_hash` Method
All games must implement `game::_init_hash(local_hash& hash)`. In this method,
assume that `hash` has been reset, and that the game's `game_type_t` has already been
baked into `hash`. Use `hash.toggle_value(position, color)`, and include the
entire game state in the hash.

Example:
```
void dyadic_rational::_init_hash(local_hash& hash)
{
    hash.toggle_value(0, _p);
    hash.toggle_value(1, _q);
}
```

Default implementations are provided by `strip` and `grid`.

## 2. Optional `_normalize_impl` and `_undo_normalize_impl` Methods
Games can optionally implement `game::_normalize_impl()` and
`game::_undo_normalize_impl()`, (but if one is implemented, the other should be
as well)

Default implementation in `strip` (omitting incremental hash update):
```
void strip::_normalize_impl()
{
    // Is mirrored board lexicographically less than the current board?
    relation rel = _compare_boards(_board, _board, true, false);
    bool do_mirror = (rel == REL_LESS);

    if (do_mirror)
        _mirror_self();

    // For undo
    _default_normalize_did_mirror.push_back(do_mirror);
}
```

`game` provides trivial default implementations which preserve the current state.

IMPORTANT: Your function must not modify the available moves, i.e. simplifying
`{1/4 | 2}` to be `1`. This results in incorrect values when wrapped by
`impartial_game_wrapper`.

## 3. Optional Incremental Hash Updates
Some of a game's methods may incrementally update the hash:
- `play`
- `undo_move`
- `_normalize_impl`
- `_undo_normalize_impl`

If these methods don't update the hash, the hash is automatically invalidated,
and will be recomputed using `_init_hash` the next time it's needed.

Incremental hash update in `nogo_1xn::play`:
```
void nogo_1xn::play(const move& m, bw to_play)
{
    // This must be called first
    game::play(m, to_play);

    const int to = m;

    // If false, the local_hash hasn't been initialized, or was previously invalidated
    if (_hash_updatable())
    {
        // This will fail an assert if the above condition is false
        local_hash& hash = _get_hash_ref();

        hash.toggle_value(to, EMPTY); // Remove old state
        hash.toggle_value(to, to_play); // Add new state

        // The hash is invalidated unless this is called
        _mark_hash_updated();
    }

    // Modify game state
    replace(to, to_play);
}
```

`game`'s default implementation of `_normalize_impl()`:
```
void game::_normalize_impl()
{
    // Do nothing, and keep the current hash
    if(_hash_updatable())
        _mark_hash_updated();
}
```

# Transposition Tables (`transposition.h`)
Defines types for transposition tables:
- `ttable<Entry>` class
    - A transposition table whose entries are of type `Entry`
- `ttable<Entry>::search_result` class
    - The value returned by querying the transposition table with a `hash_t`.
        Refers to a (possibly absent) table entry and its associated data.
        Used to access, modify, or insert the table entry (and its associated
        data) for this hash

A `ttable<Entry>` is constructed with two arguments:
- `size_t index_bits`
    - How many bits of a `hash_t` are used to index into the table. The
        number of entries in the table is `2^index_bits`
    - The remaining non-index bits are the "tag bits", and are stored alongside
        each entry
- `size_t entry_bools`
    - Quantity of extra `bool`s to associate with each entry, stored outside
        of the `Entry` in a tightly packed bit vector, and accessed through
        the `ttable<Entry>::search_result` type

`ttable<Entry>` methods:
- `ttable<Entry>::search_result search(hash_t hash)`
    - Returns a `search_result` corresponding to the queried hash
- "simpler" but slower methods not using `search_result`:
    - `void store(hash_t hash, const Entry& entry)`
        - Store the given entry using the hash
    - `std::optional<Entry> get(hash_t hash) const`
        - Query with hash. If not found, the returned value is absent

`ttable<Entry>::search_result` methods:
- Methods which are always valid:
    - `bool entry_valid() const`
        - `true` IFF the queried table entry is present in the table
    - `void init_entry()` and `void init_entry(const Entry&)`
        - Initialize the table entry and its data. May overwrite another table entry
    - `void set_entry(const Entry& entry)`
        - Assign the table entry to `entry`.
        - If the entry isn't valid, this is equivalent to `init_entry(entry)`
- Methods which are only valid if `entry_valid()` returns `true`:
    - `Entry& get_entry()`
        - Get reference to the `Entry` of the table entry
    - `bool get_bool(size_t bool_idx) const` and `void set_bool(size_t bool_idx, bool new_val)`
        - Get/set the value of a packed bool associated with the table entry

In C++, empty structs occupy at least 1 byte. If `Entry` is empty (according to
`std::is_empty_v<Entry>`), the `ttable<Entry>` will only store a single `Entry` object,
reusing it for every table entry. This can be used to save space, i.e. if a
table entry consists of a small number of `bool`s, the `Entry` struct can be
left empty, and those `bool`s can instead be stored in a packed bit vector by
the `ttable<Entry>` itself, by setting the `entry_bools` argument in the
constructor.

## `ttable` Example 1
```
#include "hashing.h"
#include "transposition.h"

...

clobber_1xn g("XOXO");
const hash_t hash  = g.get_local_hash();

struct ttable_entry
{
    bool win;
};

// 24 bit index, no extra bools associated with entry
ttable<ttable_entry> tt(24, 0);

ttable<ttable_entry>::search_result sr = tt.search(hash);

// At start of minimax search
if (sr.entry_valid())
    return sr.get_entry().win;

// Later in minimax search function, when result is known ("win" is some bool)
ttable_entry entry = {win};
sr.set_entry(entry);
```

## `ttable` Example 2
```
#include "hashing.h"
#include "transposition.h"

...

clobber_1xn g("XOXO");
const hash_t hash = g.get_local_hash();

struct empty_struct
{
};

// 24 bit index, every entry has 1 additional bool stored outside of it
ttable<empty_struct> tt(24, 1);

ttable<empty_struct>::search_result sr = tt.search(hash);

// Start of minimax search
if (sr.entry_valid())
    return sr.get_bool(0);

// Later in minimax search function, when result is known ("win" is some bool)
sr.init_entry();
sr.set_bool(0, win);
```

# Database (`database.h`, `global_database.h`)
`database.h` defines the `database` class, and two database entry structs. The
struct `db_entry_partizan` is used to store outcome classes for partizan games,
and the struct `db_entry_impartial` is used to store nim values for impartial
games. The database is not used by `MCGS_test` (CLI option `--no-use-db` is
implied).

- `set_partizan` and `set_impartial` methods take a game (single `game`) and
    entry, and store the entry in the database using the game's local hash value
- `get_partizan` and `get_impartial` methods take a game, and return a
    `std::optional` of the corresponding entry type. The value is empty (i.e.
    `returned_entry.has_value() == false`) if the entry is not found
- `save` and `load` methods save/load the entire database to/from a file
- A global instance of `database` is accessible through
    `database& get_global_database()` (`global_database.h`), after
    `mcgs_init_all()` completes.
- `database` has its own sumgame that it uses to solve outcome classes
- The data is stored in two separate "trees", one tree for partizan games, the
    other for impartial games
    - Each tree is two layers of `std::unordered_map`. The first indexed by
        game type (`game_type_t`), the second is indexed by local
        hash (`hash_t`), yielding an entry struct
    - Uses `type_mapper` class (`type_mapper.h`) to translate between run time allocated
        `game_type_t` values, and disk `game_type_t` values

        - This means the order of `game_type()` calls (more particularly, which
            game class has which value for its `game_type_t`) is allowed to be
            undefined. It doesn't matter whether `game_type<clobber>()` is
            `1` or `2` etc -- the database translates these using its own
            internal mapping saved in the database file

            - This is quick, simply indexing into a vector using the run time
                `game_type_t` to get the disk equivalent

## Database Generation
Several types are used for database generation:

- `grid_generator` (`grid_generator.h`)
    - See documentation in header file for more details
    - Interface class for iterating over string representations of grid and
        strip games
    - `grid_generator_base` and `grid_generator_masked` are abstract classes
        implementing some basic functionality
    - `grid_generator_clobber`, `grid_generator_nogo`, and
        `grid_generator_default` are non-abstract classes
    - `grid_generator_clobber` and `grid_generator_nogo` are used to iterate
        in order of increasing or decreasing number of stones
    - `grid_generator_default` starts from a board full of `EMPTY` points, and
        treats the board like a base 3 string, ending with all points being
        `WHITE`

The current non-abstract `grid_generators` take dimensions of a "max shape",
i.e. `RxC` for non-negative integers `R` (row count) and `C` (column count),
and generate strings for all grids having a shape less or equal.
- For max shape `1x1`, the order of shapes is `0x0`, `1x1`
- For `2x2`, it is `0x0`, `1x1`, `1x2`, `2x1`, `2x2`
- For `2x1`, `1x2` is omitted, as its width is greater than `2x1`'s
- Strip game strings are generated by choosing `R` to be 1, so that the
generated strings don't contain row separators (`SEP` `'|'`)

- `db_game_generator` (`db_game_generator.h`)
    - Interface class for iterating over games, defining the order of database
        entry generation. Passed to `database::generate_entries(db_game_generator&)`
    - `gridlike_db_game_generator<Game_T, Generator_T>`
        (`gridlike_db_game_generator.h`) is a non-abstract `db_game_generator`
        for grid and strip games

        - `Game_T` must be some non-abstract game derived from `strip` or `grid`
        - `Generator_T` must be some non-abstract `grid_generator` type
        - Two constructors: one takes `RxC` max shape, the other takes `C` max
            columns. The `RxC` constructor is only legal if `Game_T` is derived
            from `grid`, otherwise a compile time error will be raised
        - Generates all games in `Generator_T`'s ordering, which are legal.
            Legal games are those which can be constructed without throwing an
            exception, and, if method `bool Game_T::is_legal() const` exists,
            the method returns true.

Given a `db_game_generator`, `database::generate_entries` consumes games from
the generator, and for each generated game `g`:
1. `g.split()` is called
2. If `g` didn't split, `g.normalize()` is called
3. If `g` did split, then for each resulting subgame `sg`, `sg.normalize()` is called
4. Each subgame `sg` (either resulting from a split or not), is passed to the method
    `database::_generate_entry_single`. This searches for `sg` in the database, and
    if not found, generates its entry by solving the game for both players to find
    its outcome class

NOTE: It currently would not be sufficient to simply handle only games which
don't split, as `nogo`'s split method generates subgames which are not created
by the `grid_generator`s. The `nogo::_immortal` vector is not considered by the
current `grid_generator`s, so boards with meaningful immortal markers are
found by calling `split()`

# Adding A Game To the Database
1. In `init_database.cpp`, in the `register_types` function, use the
    `DATABASE_REGISTER_TYPE` macro to make the database aware of your game's
    `game_type_t` value.

    - IMPORTANT: write the game class name as it appears, with nothing extra,
        i.e. `clobber` and not `some_namespace::clobber`, because the game name text
        is used to identify `game_type_t`s on disk
2. In `init_database.cpp`, in the `fill_database` function, add a
    `db_game_generator` to the `generators` vector
    - The methods of the `db_game_generator` interface are analogous to those of
    `move_generator`s. You may be able to reuse
    `gridlike_db_game_generator<Game_T, Generator_T>` for your game, if it's a `grid` or `strip` game.
    - The `db_game_generator` should generate each game such that all immediate
    child positions in the search tree (those reachable by 1 move) will have
    been previously generated. This is not a hard requirement, but will speed up
    database generation
    - Implementing methods `game::_normalize_impl` and
    `game::_undo_normalize_impl` for your game may reduce the size of the
    database, and the time required to generate it
    - You can add impartial games to the database, but currently they will be
    treated like partizan games. Their entries will only be used by
    the minimax search of `sumgame`, and will contain outcome classes and not
    nim values

Recompile, delete `database.bin`, then re-run MCGS, i.e. `./MCGS ""` (note the
empty game string). This will re-generate the database. Now `sumgame` solve
methods will use your game's database entries.

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

# RTTI - Run-time type information (`type_table.h`)
Defines `i_type_table` interface, and `type_table_t` struct. The struct contains
run-time type information for any type derived from the interface, and the
interface provides access to the struct.

The struct belongs to the derived type itself, and can be accessed either
through a pointer derived from `i_type_table`, or using a template function:

```
game* g = new clobber("XO");
g->type_table();

// OR

type_table<clobber>();
```

Both examples give the same `type_table_t` -- the unique table corresponding to
`clobber` (both refer to the same memory location).

Also defines runtime-allocated type integers (`uint32_t`), included as fields
of the `type_table_t` struct:
- `game_type_t`, integer with unique value for each `game` class
- `dyn_serializable_id_t`, integer with unique value for each serializable
    polymorphic type which has been registered with the serialization system.
    Ignore this for now as it is unused (but implemented)
- And an `unsigned int` grid hash mask
- These members should typically not be accessed directly through the struct,
    but rather through methods/functions defined by other files

Allocation of each `type_table_t` is done in this file, but
allocation/assignment of these integral values is done elsewhere, i.e.
`game.cpp`, `dynamic_serializable.cpp`, and `init_grid_hash_mask.cpp`:

- `game.h`/`game.cpp`
    - Defines method `game_type_t game::game_type() const`
    - Defines template function `game_type_t game_type<T>()`
    - The `game_type_t`s are assigned at run-time, and a game class's exact
        number depends on the order of `game_type()` calls since the program
        was started
    - The value is stored in the game's `type_table_t`

- `grid_hash.h`
    - Defines template function `unsigned int grid_hash_mask<Game_T>()`
    - Value must be initialized in `init_grid_hash_mask.cpp`

NOTE: If any of these methods are called in a constructor that isn't the
most derived type, i.e. `strip` instead of `nogo_1xn`, then values may be
incorrect. The `-DTYPE_TABLE_DEBUG` compilation flag checks for this error.

Implemented using built-in C++ RTTI (`std::type_info` and `std::type_index`) to
distinguish between types, i.e. using an unordered map indexed by
`std::type_index`.

Restrictions on RTTI methods/functions:
- `type_table()`
    - Method provided by class `i_type_table`
    - For template function, `T` must inherit from `i_type_table` and be non-abstract
- `game_type()`
    - Method provided by class `game`
    - For template function, `T` must inherit from `game` and be non-abstract
- `grid_hash_mask()`
    - For both method and template function, the value must have been initialized
    - For template function, `T` must inherit from `game` and be non-abstract

`game.h` defines template `T* cast_game<T*>(game*)` acting as a
`reinterpret_cast`, but uses `assert`s to verify that the game is not
`nullptr`, is active, and is of type `T` (using its `game_type_t`)

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

# Serialization (`iobuffer.h`, `serializer.h`, `dynamic_serializable.h`)
The serialization code detailed here is used by the database. This section
mostly describes implementation details not currently important for users.

Serialization of polymorphic types is implemented but unused.

## I/O Abstraction Class
Classes `ibuffer` and `obuffer` (`iobuffer.h`) interact with files.

- `ibuffer` is a wrapper of `std::ifstream`, `obuffer` is a wrapper
    of `std::ofstream`
- Constructors accept file names
    - Files are in binary format
    - `obuffer` truncates the file upon opening it (like deleting and opening
        a new file)
- Define methods for reading/writing fixed-width integers
    - i.e. `uint8_t ibuffer::read_u8()`
    - i.e. `void obuffer::write_i32(const int32_t&)`
    - TODO: How to handle floating point? Assume IEEE?
- A step toward enforcing machine-independent binary files
    - Avoids endianness problems. The read/write methods enforce a fixed byte
        order on disk
    - TODO: However, non fixed width integer types are still a problem...
    - In C++ it is not possible to distinguish between "C" integer types and
        their fixed width equivalents, i.e. `int` and `int32_t`. Maybe write a
        clang-tidy check?
    - Possible solution: encode integer widths into the file format. Before
        reading an int from a file, first read the expected width. If the width
        doesn't match the width of the `read` method, throw.

        - Run time safety but no compile time safety
    - Currently expose `T ibuffer::__read<T>()`
        and `void obuffer::__write<T>(const T&)` (integral types `T`)
        in the public interface. Prefixed by `__` because they're only
        a (temporary?) hack that shouldn't be called directly

## Non-polymorphic Type Serialization
Template struct `serializer<T, Enable = void>` defines the interface for
serialization of both non-polymorphic and polymorphic types. This subsection
talks about non-polymorphic types.

- For a type `T`, `serializer<T>` should define two static functions:
    - `inline static void save(obuffer&, const T&)`
    - `inline static T load(ibuffer& is)`
    - These functions should recursively use the `serializer` template where
        necessary. See example implementation for `vector` below
- The default-valued `Enable` template argument is there to allow conditional
    template specialization using SFINAE (substitution failure is not an
    error), and can usually be ignored

    - This is used to define the template for all integer types
- Instantiating the template for a type that doesn't define it will trigger
    a static assert
- `serializer.h` defines the template for several standard library types:
    - all integer types
    - `std::string`
    - `std::vector<T>`
    - `std::shared_ptr<T>` and `std::unique_ptr<T>`
    - `std::pair<T1, T2>`
    - `std::unordered_map<T1, T2>`
        - `unordered_map` actually has more template arguments. TODO:
            implement the rest?

Example usage:
```
// Loading
vector<int32_t> vec = serializer<vector<int32_t>>::load(some_ibuffer);

// Saving
serializer<vector<int32_t>>::save(some_obuffer, some_vec);

// Also works for integral types:
serializer<int32_t>::save(some_obuffer, some_i32);

// Avoid using non fixed width types! This creates non-portable code:
serializer<int>::save(some_obuffer, some_int);
```

Example implementation for `vector`:
```
template <class T>
struct serializer<std::vector<T>>
{
    inline static void save(obuffer& os, const std::vector<T>& val)
    {
        const size_t size = val.size();
        os.write_u64(size);

        for (size_t i = 0; i < size; i++)
            serializer<T>::save(os, val[i]); // recursive use of template
    }

    inline static std::vector<T> load(ibuffer& is)
    {
        std::vector<T> vec;

        const uint64_t size = is.read_u64();
        vec.reserve(size);

        for (uint64_t i = 0; i < size; i++)
            vec.emplace_back(serializer<T>::load(is)); // recursive use here too

        return vec;
    }
};
```

This structure allows serialization of complicated types, i.e.
`serializer<vector<pair<int32_t, int16_t>>>::save(...)`

## Polymorphic Type Serialization (Unused, and may change)
This code is implemented but unused. This section can (and should?) be
ignored for now.

To make your polymorphic type `T` serializable:
1. Inherit from interface class `dyn_serializable` (`dynamic_serializable.h`)
2. Define:
    - Method `void T::save_impl(obuffer&) const`
    - Function `static dyn_serializable* T::load_impl(ibuffer&)`
3. Call `register_dyn_serializable<T>()` in `init_serialization.cpp`
    - If not registered, your polymorphic type can't be saved/loaded. This
        will cause run time exceptions when saving/loading `T`
    - A compile time error is raised if a registered type doesn't implement
        these functions with the correct signatures

Polymorphic type save usage example:
```
game* some_game_ptr = new clobber("XO");
// All 3 valid:
serializer<dyn_serializable*>::save(some_obuffer, some_game_ptr);
serializer<game*>::save(some_obuffer, some_game_ptr);
serializer<clobber*>::save(some_obuffer, some_game_ptr);
```

Load usage example (remember to use keyword `delete` to clean up):
```
serializer<dyn_serializable*>::load(some_ibuffer);
serializer<game*>::load(some_ibuffer);
serializer<clobber*>::load(some_ibuffer);
```

A `serializer` template is defined in `dynamic_serializable.h` for all pointer
types derived from `dyn_serializable`. It calls your `save_impl` method and
`load_impl` function using some run time type information (see RTTI section).

Each registered polymorphic type has a unique `dyn_serializable_id_t` (a
unique run time allocated integer similar to a `game_type_t`). This specifies
an index into an array of function pointers (for `load_impl` functions). This
value is written/read from file when saving/loading polymorphic types.

TODO: Use `type_mapper` like with database, to translate between run time type
IDs and disk type IDs. This will allow type registration order to differ
between the program when it saves the file, and the program when it loads
the file (i.e. across different release versions of MCGS)


# Unused `game::_order_impl` Method
This section describes an (as of v1.4) unused method.

Games can optionally implement `game::_order_impl(const game* rhs)`. `rhs` always
has the same type as `this` -- the argument passed to `clobber_1xn::_order_impl` is always a `const clobber_1xn*`,
and must be casted from `const game*` in the method.

The returned value is a `relation` enum value (`cgt_basics.h`), and should have one of the
following values:
- `REL_LESS`
- `REL_EQUAL`
- `REL_GREATER`
- `REL_UNKNOWN`
    - This is returned by `game`'s default implementation

i.e. return `REL_LESS` if `this` is lexicographically less than `rhs`.

Example in `integer_game` (note the cast):
```
relation integer_game::_order_impl(const game* rhs) const
{
    const integer_game* other = reinterpret_cast<const integer_game*>(rhs);
    assert(dynamic_cast<const integer_game*>(rhs) == other);

    const int& val1 = value();
    const int& val2 = other->value();

    if (val1 != val2)
        return val1 < val2 ? REL_LESS : REL_GREATER;

    return REL_EQUAL;
}
```

You can simplify unit tests for ordering by using
`void order_test_impl(std::vector<game*>& games)` from `test/order_test_utilities.h`.

# Outstanding Issues
## Splitting Can Make Move Ordering Worse
Splitting into subgames creates move ordering problems in some cases.

Consider:
```
[clobber_1xn] XOXOXOXOXO.XOXOXOXOXO
[integer_game] 6
```

Suppose `6` is not of type `integer_game` or `dyadic_rational`, but is some
game with an integer/rational value (`sumgame` postpones moves on these 2 game
types). It is favorable to play in the clobber game. Without splitting, moves
will first be tried in the clobber game. However, with splitting, a move will
cause the clobber game to split into subgames, and those subgames will be
placed at the end of the sum, meaning moves will be played on the integer game
first. Without a transposition table, this increases run time from (5.49 ms
black, 17.45 ms white) to (3557.94 ms black, 9807.72 ms white), in version 1.2.

Maybe we can have a subgame sorting pass which occasionally runs?

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
    - Probably OK now? the `type_table_t` pointer is stored as a field of
    `i_type_table`, whenever the `type_table()` method is called for the
    first time on some object

# Release Procedure
1. Resolve relevant TODOs
    ```
    make find_todo
    ```
2. Fix clang-tidy errors
    - Check `tidy_result.txt` after each run. When complete, no errors should be produced by these commands
    1. ```
       make tidy_headers
       ```
    2. ```
       make tidy
       ```
3. Apply clang-format
    - Basic procedure: edit original source files, then (re-)render formatted files. Repeat until desired result is achieved (see clang-format section of `style.md` for more information)
    1. Option 1: Using `make format` target
        - Render all formatted files:
          ```
          make format
          ```
          - See `format_result.txt` for diff of original vs rendered files
          - NOTE: When formatted files are present, you cannot compile. `make format_delete` to delete all formatted files. You don't need to do this between re-renders
        - Alternatively, render a subset of formatted files:
          ```
          LINT_FILES="file1 file2..." make format
          ```
        - Apply changes (simple replacement using `mv` command):
          ```
          make format_replace
          ```
    2. Option 2: Using `format-chunk.py`
        - Check how many "chunks" (groups of 10 files) there are, then exit:
          ```
          python3 utils/format-chunk.py
          ```
        - Render a chunk (where `<i>` is a chunk number). This opens relevant files in NeoVim. You can edit the loop at the bottom of the script to use your editor, or omit the `vim` argument to skip opening an editor:
          ```
          python3 utils/format-chunk.py vim -<i>
          ```
        - Apply changes after each chunk gives desired result:
          ```
          make format_replace
          ```
    - When finished, `make format` should ideally leave behind no formatted files (files containing `___transformed` in their names)
4. Run unit tests with all debugging tools:
   ```
   make clean && make test DEBUG=1 ASAN=address
   ```
5. Run game search tests with different debugging tools
    - Check results after each run (view resulting `out.html` in your web browser):
      ```
      python3 create-table.py out.csv -o out.html
      ```
    1. Run default tests with all debugging tools (this will be very slow):
       ```
       make clean && make DEBUG=1 ASAN=address && ./MCGS --run-tests
       ```
       Check list of default implementation warnings at the end to see if some
       games are missing implementations of functions.
    2. Run larger test set with default compilation flags (also quite slow):
       ```
       make clean && make && ./MCGS --run-tests --test-dir input/main_tests
       ```
6. Update documentation. Prune outdated/resolved notes
    - `development-notes.md`
    - `style.md`
    - `todo.md`
    - `info.test`
    - `README.md`
    - To create/update table of contents:
      ```
      doctoc --github --title "# Table of Contents" --maxlevel 1 docs/development-notes.md
      ```
7. Prune relevant temp files (`src/temp`, `docs/temp`)
8. Create github release. Include notes about new features

# Versions
## Version 0 (Completed)
- `move` and `game` classes, `alternating_move_game`
- Nim: `nim` implementation done
- Utility class `strip` for 1xn boards
- Clobber on a strip: `clobber_1xn`
- Basic minimax implementation in `alternating_move_game::solve`
- Basic test cases in files, run automatically
- Nogo on a strip: `nogo_1xn` class
- Simple game classes: integer, dyadic rational, up-star, switch, nimber

## Version 1 (Completed)
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

## Version 1.1 (Completed)
### New Features
- `simplify_basic_cgt` solver optimization
    - simplify "basic" games of same type in a `sumgame`, e.g.
        - add up integers/rationals
        - add ups+stars
        - add nimbers
- New MCGS CLI flags
- Input language version: `1.0` --> `1.1`
    - C-like comment syntax
    - `dyadic_rational` and `switch_game` now expect fractions
    - Syntax for all games is documented in [input/info.test](input/info.test)
- More input files in `input` directory

### Major Code Additions
- Bounds search implementation
- RTTI implementation (`game_type_t`)
- `sumgame` improvements and helper classes
- New utility classes/functions
    - `fraction`
    - `safe_arithmetic.h`
- Various scripts in `utils` directory (i.e. to generate random input files)
- More cleanup and Tools and Components for Database
    - Code formatting tools and checks, "lint"-like
        - Applied `clang-format` and `clang-tidy` to all source files
    - `scale` S such as multiples of up, or up+star, or integers
        - binary search to find upper/lower bounds for a game G on scale S
        - simplify `game` G in `sumgame` S
            - compare with/without subgame split

## Version 1.2 (Completed)
### New Features
- Transposition tables speed up search
- Impartial games support
    - Impartial game solving algorithm
    - Input language version `1.1` --> `1.2`
        - Impartial game variants are automatically created for all games i.e. `[impartial clobber_1xn]`
        - New solve command for impartial sums: `{N 2}` computes nim-value, expects it to be `2`
- More games: `nogo`, `clobber`, `kayles`
- More `.test` files. Most have been verified by external solvers

### Major Code Additions
- `game` and `sumgame` hashing
    - Several hashing-related hooks for `game`s to implement
- `grid` class
    - `grid` helper classes (`grid_utils.h`)
- `impartial_game` class
- `mcgs_init_all()` initializes global data
- More code safety (clang-tidy checks, debugging checks)
- More scripts in the `utils` directory
    - CGSuite scripts
    - More random game generation scripts

## Future: Smaller Step Versions 1.x to Prepare for Version 2
- change read from string functions to directly create sumgame
    - this would require `sumgame` to be the sole owner of its games
    - OR, the `game_case` produced by `file_parser::parse_chunk` could own
    the games in a `vector<unique_ptr<game>>`
- compare G with a simpler game H, replace in S if equal
- compare G1+G2 with a simpler H, replace in S if equal

## Version 1.3 (Completed)
### New Features
- First database implementation (`database.h`, `global_database.h`)
    - Currently stores outcome classes for single (partizan) normalized subgames
    - `sumgame` uses outcome classes from the database to speed up search
    - `db_game_generator`s (`db_game_generator.h`) define the order of database
    entry generation for a game type
    - The database is loaded from the `database.bin` file on startup, if present
    - If not present, `database.bin` is created for all currently implemented
    `grid` and `strip` games
- Split method for `nogo`
    - From Y. Shan's PhD thesis (see `nogo.h`)
- Scripts for paper experiments (`utils/paper_experiments`)
    - Configurable test case generator
    - Multithreaded test runner runs several instances of MCGS and assigns test
    cases to available threads
    - Diagram generator
    - See `instructions.txt` in the `utils/paper_experiments` directory
- `commits.py` script for comparing performance across commits
- Input language version `1.2` --> `1.3`

### Major Code Additions
- Helper classes for adding games to the database
    - `grid_generator` (`grid_generator.h`) types provide a way to generate
    all string representations of legal strip and grid games, up to some max size
        - Generates in order of increasingly large dimensions
        - Ties can optionally be broken in order of increasing or decreasing
        number of stones
    - `gridlike_db_game_generator` (`gridlike_db_game_generator.h`) provides
    a template to easily create a `db_game_generator` for `strip` and `grid` games.
        - Simply specify the game class, generator class, and max board dimensions
- Serialization API (`serializer.h`)
    - Uses a recursive templating system to support saving and loading of
    complex types to/from disk, while keeping implementation simple
    - Premade implementations for several standard library types
    - Support for both non-polymorphic and polymorphic types
- `split()` and `normalize()` methods improved for some games
- `clobber` split is always enabled, no longer requiring an additional compilation flag

## After Version 1.3 (Future)
- Add more games (i.e. Amazons)
- Simple interactive player
- Improve database
    - Support querying sums
    - Impartial game support
    - Utilities
        - Tool to compare database files
    - Possibly dynamically load/unload database chunks
    - More data in entries
        - Link to "simpler" equal game
        - Dominated move list

## Version 2 (Future)
- general sum simplifications
    - find inverse pairs and deactivate

