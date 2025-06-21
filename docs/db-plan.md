# Database Design and Planning
A more organized collection of thoughts about database details

Sections:
- Requirements/Motivations
- Hashing
    - Types of hashes
    - Use cases of hash types
    - "Change of base" trick for perfect hashes
- Data Structures
- Indexing
- Functions
- Serialization
    - Games
    - DB data
- Other implementation details
- Versions/steps
- Concerns, unresolved issues, and other notes

## Requirements/Motivations
Things to prioritize in the design of our databases.

1. Portability of DB files
    - DB files may take a long time to create, especially when we have lots of
        games, so we should be able to share them across machines
2. Speed of queries
    - Self explanatory. Queries shouldn't take too long
3. Allow both read AND write
    - May be optimized more for reads, but writes must be reasonably fast too
4. Safety
    - Metadata/header in DB files, sanity checks
        - `game_type_t` mapping, seeds, etc
    - Some way to validate DB data after generation
5. DB entry flexibility
    - Partizan games will have outcome classes, but impartial games will have
        nim values
    - Some games may use different hashing schemes
6. Modularity
    - We can't know if our solution is good unless we tweak it and compare
    - Some optimizations are very useful, but can only come much later. The
        system must be malleable enough to accomodate these
7. Dynamic loading/unloading
    - Good for very large problems, or when we support lot of games
    - Most searches only need a small section of the database loaded
        - Solving a clobber game shouldn't need the nogo data loaded (?)

## Hashing
### Types of hashes
1. Zobrist
    - Simple, general, fixed size
2. Perfect
    - No collisions, may be smaller, could double as game serialization
    1. Sparse (perfect hashes with gaps between hash values)
        - Potentially 1-2 operation lookup, with (large?) memory overhead
    2. Dense (perfect hashes with no gaps between hash values)
        - 1 operation lookup
    3. Run length encoding
        - Smaller hashes for games with repeating contents (i.e. checkers)

- "Gray codes" could improve locality of hashes?
    - TODO: needs elaboration
    - Unclear whether or not this is useful. The purpose of gray codes is to change
        only one code bit to get to the next decimal value. But, for two game
        boards treated as gray codes, if they differ by only one tile, then their
        corresponding decimal numbers may still be far apart
    - Maybe try some experiments with this?

- TODO: look into locality-sensitive hashing

### Use cases of hash types
Zobrist hashes:
- Average log2(N) steps to find item in list of N items
- Uniform randomly distributed
    - Therefore, splitting list of hashes into 2^M buckets means about
        1 + log2(N) - M steps to find items

Perfect hashes:
- While generally smaller, could still become larger than 64 bits if not careful
    - This could cause overflow problems for numerical operations
- Not uniform randomly distributed
    - Therefore, splitting into buckets may not be as simple or effective
- If a range of hashes is dense, then an item's location is known in 1 operation
- If the range is sparse, can still find item in 1-2 operations
    - One option: have empty entries for the missing hash values
    - OR, check some sort of "offset table" with offsets based on missing entries
    - Maybe space-inefficient
        - For N unused bits, only 1/2^N of the table is actually used
    - TODO: look into how unordered_map solves this (can still have O(1) lookup
        for sparse hash)

- Therefore, use zobrist hashes if there are discontinuities in the hash space,
    and use perfect hashes if there are none.
- We could use perfect hashes to verify that there are no collisions in the
    database (between entries using zobrist hashes)
    - OR, compare serialized game representations

### "Change of base" trick for perfect hashes
When tiles of a board have non power of 2 ranges, this creates sparse hashes
if not addresed.

i.e. tiles of `.`, `X`, and `O` only need "1.5" bits, but the natural solution
would be to allocate 2 full bits per tile. If we give each tile a full 2 bits,
this is extremely wasteful assuming we want 1 operation lookup;
for 3 tiles, there are (3^3 == 27) values, but 6 bits is (2^6 = 64) values, so
~58% of the space is unused!

Instead, consider the string of tiles to be a number whose digits may each be
of a different base, then convert the number to decimal. i.e. for a 3 tile
strip, suppose the tiles' ranges are 4, 2, and 3 respectively, and we have
the strip with values 3, 1, 2:

Digit base:  4  2  3
Digit:       3  1  2
Decimal conversion: `3*(2*3*1) + 1*(3*1) + 2*(1)` (each summand is a digit
    multiplied by the product of digit bases to its right)

If we have all boards in this range, then this process creates dense hashes.

NOTE: Must be careful to avoid overflow. There doesn't seem to be a way to
"flush" some of the digit sequence (i.e. some kind of modulus operation) and
then concatenate sequences of bits. Example: some base could be odd, then
resulting place value would modify binary digit for 1. But we can do this if all
bases are 2, so maybe there's something to this still?

## Data structures
- STL containers could be slow? May still be worth using for safety/debugging

Main types:
- Entry
    - Plain struct with serialization functions
- Index
    - Similar in usage to `std::unordered_map` and `std::map`?
    - Different types for different indexing strategies?
        - `class index_buckets` (hash buckets + binary search for zobrist hash)
        - `class index_array` (1 operation lookup for perfect hash)
    - Must have iterator and `size()`
    - Not serializable. Other data structures should use it, and serialize it
        - May be simpler to make it serializable? Especially with tree
            structure...
        - Deserialization should stop at some point; don't load all chunks
            - Solution: tree stores `chunk_handle`s instead of `chunk`s?
- One of: "Index Layer" OR "Index Tree"
    - Uses `index`
    - Serializable
    - TODO: How to preserve relationships between layers?
        - Maybe don't allow each layer to be independently serializable? Just
            load the entire index tree. Rename `index_layer` to `index_tree`
        - Serialize iterates over the entire tree of `index`es, writes into
            `iobuffer`
            - `iobuffer` contents:
                `tree_count = N, tree_data_1, tree_data_2, ... tree_data_N`
            - Recursive definition: each `tree_data` looks like this
- Chunk
    - May use `index` in its implementation
    - Serializable type

## Indexing
- Similar data should be in similar locations within the database
    - i.e. sums of clobber games should be together
    - To achieve this, give each sum a tuple of hashes
        - i.e. `(game_type_t sequence, strip length sequence, global hash)`
    - Each hash within a tuple could correspond to a separate index layer
    - OR, layers can be "flattened" together by combining hashes, and possibly
        increasing the number of hash buckets
- Should support both zobrist and perfect hashing (game dependent)
- Per-game hash tuples?

- Tricky optimization problem:
    - Add more layers? Flatten layers?
    - Increase/decrease number of hash buckets?
    - Increase/decrease number of hashes used in tuple?
- Adding layers decreases the probability of zobrist hash collisions

- Example: clobber_1xn sums:
    `(game_type_t sequence, strip length sequence, combined perfect hash)`
    - If querying only sums of like-type, game_type_t sequence could be just
        an int indicating how many games are in the sum
        - In the general case, we need the type sequence
    - Strip length sequence is necessary for perfect hashing of sums in this case

- Generalize "strip length sequence" to all games as
    `virtual vector<int> game::shape() const`?
    - Encode into DB as vector's length, followed by its values?
    - TODO: Check if this makes sense for all games, not just clobber

From prototyping:
- `class zobrist_index`/`class better_zobrist_index`
    - First attempt at map for database
    - Compare to `std::unordered_map`, and `robin_hood::unordered_map` (faster
        than `std`)
    - For search: comparable to `robin_hood`, but still slightly slower
        - `zobrist_index` faster than `better_zobrist_index`? (not sure why)
    - For insertion:
        - `better_zobrist_index`: much faster than `std`, but not as fast as
            `robin_hood`
    - Empirically: choose n_bits such that `1 <= average_bucket_size < 2`
        - `average_bucket_size := N / (1 << n_bits)`, where `N` is number of
            elements
        - This keeps memory overhead reasonable, and gives fast lookup
    - Linear probing for `bucket_size <= 8`, otherwise binary search
    - To search: use `n_bits` most significant bits of query hash to index bucket
        list. Then search bucket
    - Memory overhead (bucket_t is 16 bytes, element_t is 12 bytes)
        - 16 million elements, initial bucket size 4: 439.279 MB overhead
        - 16 million elements, initial bucket size 2: 250.905 MB overhead
        - 1 million elements, initial bucket size 4: 27.4444 MB overhead 
        - 1 million elements, initial bucket size 2: 15.6693 MB overhead 
    - NOTE: when deserializing the index from disk, the `bucket_t` shouldn't own
        its data -- it should point to some "flat" array, so that
        loading/unloading doesn't require a malloc/free for each bucket. This
        may require a "read only" mode. Or maybe this doesn't matter...

## Serialization
- Need to consider buffering for disk I/O
    - fread() vs read() vs fstream
    - Ideally (?):
        - Read entire buffer from disk
        - Write entire buffer to disk
        - Do operations on the complete buffer in memory
- Endianness is only a problem if we read/write raw arrays to disk
    - Problem is solved by encoding data into I/O buffers in some defined
        byte order
- New class `iobuffer`
    - inline functions to read/write integral types from/to the buffer
        - write_u32, read_s32
        - Solves endianness problem
    - Are existing std file I/O types sufficient for this?

### Game serialization
NOTE: `impartial_game_wrapper` games all have the same `game_type_t`, so for
    this section, suppose we've dealt with this somehow by making
    `impartial_game_wrapper`'s type simply be the negative of its wrapped game's
    (0 is already not a valid `game_type_t`).

- Games implement optional methods/functions:
    - Serialize method: `clobber_1xn::serialize(iobuffer&) const`
    - Deserialize function: `static clobber_1xn* deserialize(iobuffer&)`
    - Used by a "serializer" object, created for a specific game, and owned
        by the database code

- Interface `class i_serializer`
    - `virtual void serialize(iobuffer&, const game*) const = 0`
    - `virtual game* deserialize(iobuffer&) const = 0`

- Class `class serializer<T>: public i_serializer`
    - `T` is derived from `game`
    - Uses custom type traits to ensure `T` implements the necessary
        serialization functions/methods, giving nice error messages if
        it doesn't
        - `has_serialize_v<T>` and `has_deserialize_v<T>`
            - (expressions of type `static constexpr bool`)
            - Easy to implement these
    - Automatically implements `i_serializer` methods
        - Possibly writes `game_type_t` into iobuffer before calling `serialize`
    - Allow games to be registered for use in the database by some function
        somewhere. This creates a "global" serializer for that game type
    - Automatically handle impartial wrapper games too

- Serializer objects solve the problem of mapping a `game_type_t` read from
    disk, to serialization functions in memory somewhere
    - Also keeps game's serialize/deserialize methods optional, while still
        maintaining some compile time type checking
        - Includes enforcement of static deserialize function as part of the
            `game` DB interface (can't do this with normal class inheritance)
        - Though a game could inherit from some type `i_serializable<clobber>`
            and the check for the static function could happen then

```
// database.h
unordered_map<game_type_t, shared_ptr<i_serializer>> serializer_map;

template <class T>
shared_ptr<i_serializer> get_serializer<T>();
```

- `shared_ptr` in above code instead of `const i_serializer&` (avoid static
    data destructor order problems)

- TODO: handling impartial wrapper games is a bit janky and unclear...
    - What should the template parameters for these functions/types be?
    - `<Some_Game_Class, partizan_or_impartial_enum = GAME_PARTIZAN>` (??)

### DB data serialization
TODO: expand this after enumerating data structures, or write this in data
structures section instead...

## Other implementation details
- DB file needs a header for metadata
    - `random_table` seed
    - A few `random_table` values
    - Game class -> `game_type_t` mappings
        - mcgs_init() must assign `game_type_t` to each game
        - Use macro trick for this:
          ```
          #define INIT_TYPE(class_name) init_type<class_name>(std::string(#class_name))
          ```
    - We don't need to include the types for impartial wrapper games, they can
        be inferred

- Currently, `game_type_t` of all `impartial_game_wrapper` games is identical
    - Override `impartial_game_wrapper::game_type()` method: call wrapped game
        implementation, and return negative?

- Some games don't need database lookups, but we still know stuff about them
    - Use some kind of type info to inform the sumgame simplification process?
        - clobber_1xn: SIMPLIFY_COMBINE
            - Search the database for sums of these (only sums of like type?)
        - kayles: SIMPLIFY_SINGLE, SIMPLIFY_NO_DB
            - Replace single games, not sums
            - Don't use database (generate its entry dynamically)
        - integer_game: SIMPLIFY_SKIP
            - Entirely omit from (this) simplification process
        - All impartial games will use SIMPLIFY_SINGLE (replace with nimber)
            - Not all impartial games will use SIMPLIFY_NO_DB
                - Just octal games?
        - Instead of querying DB directly from sumgame, have some function to
            do this for us, which accounts for games like kayles not needing
            DB?

- With DB, sumgame's search gets many more steps, whose order matters
    - TODO: expand this and think about it more...
    1. Search DB, replace games
    2. Simplify basic CGT games
    3. Remove zero pairs
        - To maximally prune these, may need to repeat this step elsewhere in
            this sequence
    4. Apply static outcome class rules

## Versions/steps
1. iobuffer, game serialization
- TODO expand me

## Concerns, unresolved issues, and other notes
- If we only store normalized games, then either perfect hashes are all sparse,
    or data is duplicated (potentially 8 times with grids!)

- Could use a separate thread for I/O?

- Unloading chunks using a simple policy may not be ideal if they vary a lot in
    size

- It would be nice to have some way to validate the database after generating it
    - Even doing everything to minimize the probability of collisions, they
        could still happen (whether in the `ttable` during generation, or maybe
        between DB entries)
    - One possibility: use external solvers
    - Simpler/more practical solution: use MCGS itself
        - PROBLEM: the `random_table`s used to generate the DB files must be
            the same as the `random_table`s used when reading it. If the random
            seed is the cause of collisions, we would need some way to use
            two seeds at once (one to interact with the database, one for
            everything else)

- Would it be useful to allow games to link to games of different types? i.e.
    clobber linking to nogo

- Can integer_games really be excluded from DB queries by sumgame? Is there an
    instance of some `game + integer` that gives a useful DB query?

C++ integer problem:
- Types like `int` have variable width (depending on compiler/machine), but we
    want our DB files to be cross-platform compatible. This is a big problem,
    because if an `int` is 4 bytes, then it's impossible to distinguish between
    `int` and `int32_t`, but on another machine, `int` could be 2 or even 8
    bytes.
    - `static_assert(!std::is_same_v<int, int32_t>)` fails
    - Template specizaliation/function overloading can't distinguish between
        them
    - Compiler doesn't print a warning when an `int` is passed to a function
        taking an `int32_t` (-Wall, -Wpedantic, -Wextra)
    - It's not possible to use placeholder structs i.e. `struct i8`,
        `struct i32`, etc, enforce their usage in place of ints, and use a
        template to recursively replace the placeholders with the correct int
        types. For example, `unordered_map<int8_t, int8_t>` is not the
        same as `resolve_placeholders<unordered_map<i8, i8>>::type`,
        because the template containing the placeholder types is resolved
        to an actual type before `resolve_placeholders` can modify it
One solution:
    - Encode integer widths into the output file, preceding lists of integers,
        then catch mismatches as a runtime error
    - May require `iobuffer` operations to keep some kind of state
    - In the future, see if we can write a clang-tidy check to enforce not
        passing non-fixed-width integers to serialization functions?

- How to serialize floats/doubles in a portable way?
