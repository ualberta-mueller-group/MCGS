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
- `class assert_restore_game` in `alternating_move_game.h` is a stub for
checking that the state is restored
    - a naive first implementation just checks the length of the move stack
    - TODO it probably is broken for sumgame, since it uses a different stack
    - TODO this check will be made functional after hash codes are implemented

# Initialization (`mcgs_init.h`)
Executables based on the C++ source code must call one of the `mcgs_init_all()`
or `mcgs_init_all(const cli_options& opts)` functions after (optionally)
parsing command-line arguments. These functions initialize static data, i.e.
`sumgame`'s transposition table, and global `random_table`s.

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

## `cli_options` struct (`cli_options.h`)
Holds values resulting from parsing command-line arguments
- Returned by static method `cli_options::parse_args`
- Holds optimization options as global/static fields of `cli_options::optimize`

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
represented in the table, and the seed for the random numbers in the table. A
seed of 0 seeds the table with the current time (in ms) since the system
clock's epoch.

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
2. Sort the `game`s of `S` according to `game::order()` so that each `game` `g_i` has a subgame index `i`
3. For each `g_i`, get its `local_hash` value `h_i`
4. For each `h_i`, compute a modified hash `H_i := hmod(h_i, i)`, for some hash modifier function `hmod`
    - The actual `hmod` function is `H_i := RANDOM_TABLE_MODIFIER[i, h_i]`
    - Using a linear congruential generator instead doesn't seem to be significantly different in terms of performance
5. Given the player to play `p`, compute `P := RANDOM_TABLE_PLAYER[0, p]`
6. The `global_hash` value is the XOR of all `H_i`, and `P`

TODO: Currently `sumgame::get_global_hash()` sorts all of the `sumgame`'s games
every time it's called. `sumgame` could maintain an ordering of its games to
prevent this.

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
1. A game's `play` method should call `game::play` at the START of the method
2. A game's `undo_move` method should call `game::undo_move` at the START of the method (after getting the last move from the stack)

These base class methods do some record keeping around local hashes.

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

A default implementation is provided by `strip`.

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

## 4: Optional `_order_impl` Method
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


# Transposition Tables (`transposition.h`)
Defines types for transposition tables:
- `ttable<Entry>` class
    - A transposition table whose entries are of type `Entry`
- `ttable<Entry>::iterator` class
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
        the `ttable<Entry>::iterator` type

`ttable<Entry>` methods:
- `ttable<Entry>::iterator get_iterator(hash_t hash)`
    - Returns an entry iterator corresponding to the queried hash

`ttable<Entry>::iterator` methods:
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

clobber_1xn g("XOXO");
const hash_t hash  = g.get_local_hash();

struct ttable_entry
{
    bool win;
};

// 24 bit index, no extra bools associated with entry
ttable<ttable_entry> tt(24, 0);

ttable<ttable_entry>::iterator tt_it = tt.get_iterator(hash);

// At start of minimax search
if (tt_it.entry_valid())
    return tt_it.get_entry().win;

// Later in minimax search function, when result is known ("search_result" is some bool)
ttable_entry entry = {search_result};
tt_it.set_entry(entry);
```

## `ttable` Example 2
```
#include "hashing.h"
#include "transposition.h"

clobber_1xn g("XOXO");
const hash_t hash = g.get_local_hash();

struct empty_struct
{
};

// 24 bit index, every entry has 1 additional bool stored outside of it
ttable<empty_struct> tt(24, 1);

ttable<empty_struct>::iterator tt_it = tt.get_iterator(hash);

// Start of minimax search
if (tt_it.entry_valid())
    return tt_it.get_bool(0);

// Later in minimax search function, when result is known ("search_result" is some bool)
tt_it.init_entry();
tt_it.set_bool(0, search_result);
```

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

# This text can probably be deleted

## Regarding `game` "Hooks"
- A `game`'s `local_hash` is computed lazily by `game::get_local_hash()`, and then possibly incrementally updated afterward
- The hash is computed from scratch by abstract method `game::_init_hash(local_hash&) = 0`, implemented by each game
- Hashes can optionally be incrementally updated by a game's play(), undo_move(), normalize(), and undo_normalize() methods
    - These methods can check if the hash has been computed via `_hash_valid()`, then update the hash and call `_mark_hash_updated()`
    - But this requires `game` to do record keeping before these methods interact with the hash
        - play() must immediately call `game::play()`
        - undo_move() must immediately get the previous move from the stack, and call `game::undo_move()`
        - normalize() must immediately call `game::normalize()`
        - undo_normalize() must immediately call `game::undo_normalize()`
        - If any of these methods fail to call the corresponding base class method, search will be incorrect

- An alternative is to use "hooks", i.e. have `game::play()` be a non-virtual method which calls abstract method `game::_play_hook() = 0`
    - With this, it's impossible for the user to forget to call base class methods
    - But multiple levels of inheritance can make hooks confusing
        - With "c" for "color" and "nc" for "no color":
            - `void game::play_c()` and `virtual void game::_play_c_hook() = 0`
            - `void impartial_game::play_nc()` and `virtual void impartial_game::_play_nc_hook() = 0`
            - A 3rd abstract class, inherting from `impartial_game`, would make this worse
        - Harder to trace execution
            - Your game's play() method isn't called directly when you do `g.play(...)`
                - Can't trace execution just by looking at your play() method
                - Multiple methods called before the game's play() method
            - The code has more functions, making it harder to understand
            - See call graph example below

### Call graphs for `impartial_game`s, using hook solution
```
kayles k(...);
k.play_c(...);
```
game::play_c()
    impartial_game::_play_c_hook() final
        kayles::_play_nc_hook() override


```
kayles k(...);
k.play_nc(...);
```
impartial_game::play_nc()
    game::play_c()
        impartial_game::_play_c_hook() final
            kayles::_play_nc_hook() override


### Proposal
Have some methods be hooks, and others not. `game::play()` and
`game::undo_move()` should stay how they were before, but normalize and
undo_normalize can be hooks; these methods are unlikely to result in confusing
chains of virtual methods

## `game` Hooks Related to Hashing
`game` has several virtual methods related to hashing, some of which have
default implementations.

Non-virtual `game::order(const game*)` returns a `relation` enum value, indicating the
ordering of two games. Considering two games `g1` and `g2`, `g1 < g2` (for
ordering) when:
- `g1`'s `game_type_t` is less than `g2`'s
- If `g1` and `g2` are of the same type, the returned value is
    `g1._order_impl(g2)`
    - If `_order_impl` returns `REL_UNKNOWN`, the value returned by
        `game::order` is instead `REL_EQUAL`

Hooks without default implementations:
- `game::_init_hash(local_hash& hash)`
    - When called, `hash` has been reset, and has had its type set. The
        implementer must compute the full hash for their game.

Hooks with default implementations:
- `game::_order_impl(const game*)`
    - Only called on two games of the same type. Returns ordering of the games,
        as a `relation` enum value. Default returns `REL_UNKNOWN`
    - Implementer should cast the game pointer argument to their game's type,
        i.e. `clobber_1xn::_order_impl` should cast from `const game*` to
        `const clobber_1xn*`
- `game::_normalize_impl()`
    - Normalize a game. Default does nothing
- `game::_undo_normalize_impl()`
    - Undo normalization of a game. Default does nothing

`strip` provides default implementations:
- `strip::_init_hash(local_hash& hash)`
    - Computes hash of the board. Assumes that the board contains all of the
        game's state
- `strip::_normalize_impl()` and `strip::_undo_normalize_impl()`
    - Potentially reverses the board based on lexicographical order
- `strip::_order_impl(const game* rhs)`
    - Ordering based on lexicographical ordering of boards

### Incremental Hash Updates
`game`s can optionally incrementally update their hashes in the following methods:
- `play()`
- `undo_move()`
- `_normalize_impl()`
- `_undo_normalize_impl()`

To incrementally update a hash, follow these steps:
1. If `_hash_updatable()` returns `true`, the hash can be updated, otherwise it cannot
2. Get a reference to the `local_hash` through `_get_hash_ref()`
3. Remove relevant previous state from the hash, and add the new state, using `local_hash::toggle_value(position, color)` to do both
4. Call `_mark_hash_updated()`. If this function is not called, the hash will be discarded, to be recomputed later by `_init_hash()` as needed
    - Some games/methods opt to discard hashes, when updating them isn't worth the cost


