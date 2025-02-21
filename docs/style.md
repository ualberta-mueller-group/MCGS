# Style Guide

This document gives details about coding style for C++ files in this project, and tools relating to style.

## Sections
- [Linter Problems](#linter-problems)
- [Code Style](#code-style)
- [Style Tooling](#style-tooling)
    - [clang-tidy Targets](#clang-tidy-targets)
    - [clang-format Targets](#clang-format-targets)
- [Modifying Tooling Configs](#modifying-tooling-configs)
    - [Tidy Config Testing Script](#tidy-config-testing-script)
    - [Format Config Choices](#format-config-choices)

# Linter Problems
Look into these more before doing the big cleanup...

## Clang Format
`test/utilities_test.cpp` on the `bounds` branch looks awful (because of `AlignAfterOpenBracket`)
`AlignTrailingComments` --> "Leave" (or "Never")
`BraceWrapping.AfterNamespace` --> false

# Code Style
This section describes the style used by C++ files in this project.

### Brace wrapping
Braces get their own lines:
```
if (true)
{
    do_something();
}
```

### Pointer/reference symbols
Pointer/reference symbols go on the left:
```
int* x;
int& y;
```

### Identifier naming
Identifiers should be lower case, with underscores separating words, with the following exceptions:
- Global/static/enum constants, macros, and constexpr variables should be upper case
- Local constants can be either lower or upper case

Additionally, private and protected member methods/fields should start with a `_`. Other identifiers should not.

Example:
```
int global1;
const int GLOBAL2 = 0;

#define MACRO1 5

enum some_enum
{
    SOME_ENUM_CONSTANT1,
};

class some_class
{
public:
    void method1(int param1);

    int field1;
    const int field2;

    static const int FIELD3;

private:
    void _method2();

    int _field4;
    const int _field5;

    static const int _FIELD6;
};

constexpr int SOME_INT = 0;

constexpr int some_constexpr_func()
{
    return 0;
}

int main()
{
    const int some_local_constant = 0;
    const int SOME_LOCAL_CONSTANT = 0;

    ...
}
```

### Structs/classes
Generally, use structs for plain data structures, and classes for objects with more complicated semantics (i.e. methods, private/protected specifiers, non-trivial constructors, etc.).

Within a struct/class, and within a given (possibly implicit) access specifier, methods should come before fields:
```
class some_class
{
    void _some_method1();
    void _some_method2();

    int _some_field1;
    int _some_field2;

public:
    void some_method3();

    int some_field3;
};
```

Additionally:
- Include 1 empty line above access specifiers (i.e. public/protected/private), unless the access specifier comes immediately after the opening brace
- Access specifiers aren't indented past the opening/closing brace level

### Override
Implementations of virtual methods should use either the `override` or `final` specifiers.

### Namespaces
End namespace blocks with comments like "namespace your_namespace_name":
```
namespace cgt
{
...
} // namespace cgt
```

### Implementations in headers
Function/method definitions should generally go in `.cpp` files and not `.h` files, unless the function/method is both short and declared `inline`, or is a template. Definitions possibly leading to [ODR violations](https://en.cppreference.com/w/cpp/language/definition) should be caught by the linter tools explained in following sections.

### "using" keyword
The `using` keyword should not appear in headers, i.e. `using namespace std;`, `using std::vector;`.

### Empty lines
Definition blocks (i.e. classes/structs, enums, functions/methods) should be separated by a single empty line:
```
enum some_enum
{
    SOME_VAL1,
    SOME_VAL2,
    ...
};

// This comment doesn't count as an empty line
void some_function()
{
}

class some_class
{
public:
    void some_method()
    {
        ...
    }

    void some_other_method()
    {
        ...
    }
};
```

### Spaces
Leave spaces around binary operators, but not unary operators:
```
int x = 4 + 5;
int y = *some_pointer;
++y;
int z = some_pointer[x];
```

Leave spaces between statements of a for loop's "head", unless the statement is empty:
```
for (int x = 0; x < 100; x++)
{
}

for (;;)
{
}
```

Leave a space after a C-style cast:
```
float x = (float) 5;
```

# Style Tooling
Two tools are used to help enforce style: `clang-tidy`, and `clang-format`. Clang-tidy does static analysis to catch problems such as incorrect identifier naming, whereas clang-format mainly deals with formatting of whitespace. These tools are used separately, from several makefile targets described in following subsections. These targets all operate on default sets of source files, and you can override these by defining the `LINT_FILES` variable in your shell environment:
```
LINT_FILES="src/some_file.cpp src/some_other_file.cpp" make format
```

Before opening a pull request, contributors should first run these tools and fix any problems found. Not all problems will be caught by these tools, including:
- Methods not coming before fields within an access specifier block
- Presence/absence of `_` prefix for static members of structs/classes
- Using classes instead of structs for "plain data" structures
- Using structs instead of classes for objects that aren't "plain data"

These tools should catch:
- Whitespace formatting problems
- Missing comments at the ends of `namespace` blocks
- Most identifier naming problems
- Missing `override` or `final` specifiers for virtual function implementations
- Functions in headers having more than 2 statements
- `using` keyword in headers
- Definitions in headers which could cause ODR violations

You should use the following makefile targets before opening a pull request:
- `tidy`
- `tidy_header_functions`
- `format`

## clang-tidy Targets
4 targets are used to invoke clang-tidy:

- tidy
    - Run clang-tidy on all source files, using release compilation flags
- tidy_release
    - Run clang-tidy on only release source files (i.e. `MCGS` target), using release compilation flags
- tidy_test
    - Run clang-tidy on only test source files (i.e. `MCGS_test` target), using test compilation flags
- tidy_header_functions
    - Special case which only checks for non-trivial functions in headers. Other targets do not check this
    - Run clang-tidy on all source files after excluding non `.h` files, using release compilation flags, and `.clang-tidy-headers`

The result will be printed to the screen, and also saved in `tidy_result.txt`. Many thousands of warnings will be found and suppressed within included system headers, and problems within project code will be shown as errors instead of warnings. If errors are not mentioned in the output, then no problems were found in project code. Errors must be fixed manually, as automatically applying suggested changes will break code, i.e. renaming a virtual method in a base class will not rename the overriden method in derived classes.

To disable clang-tidy for certain sections, see [Suppressing Undesired Diagnostics](https://clang.llvm.org/extra/clang-tidy/#suppressing-undesired-diagnostics). For example, to disable function length checks for template functions in headers:
```
template <class T>
// NOLINTNEXTLINE(readability-function-size)
void some_func(const T& some_t)
{
    ...
}
```

## clang-format Targets
3 targets are used for clang-format:

- format
    - Run clang-format on all source files, generating new files with suffixes like `___transformed.cpp` and `___transformed.h`, which are formatted versions of the original source files. Transformed files are omitted when identical to their original sources. Warnings will be printed for source/transformed pairs differing by more than just whitespace
- format_delete
    - Delete transformed files
- format_replace
    - Replace source files with their transformed versions, if they exist
 
A combined diff of all source/transformed pairs will be output to `format_result.txt`, and includes file names. For all of these targets, the paths specified within `LINT_FILES` can be either source files, or their transformed versions -- both will produce the same effect. 

To disable clang-format for certain sections, see [Disabling Formatting on a Piece of Code](https://clang.llvm.org/docs/ClangFormatStyleOptions.html#disabling-formatting-on-a-piece-of-code). Alternatively, for complex conditionals spanning multiple lines, include comments at the end of each line to prevent collapse onto a single line:
```
if (a //
    || (b && d) // these comments can be empty or non-empty
    || (c && a)) //
{
    ...
}
```

# Modifying Tooling Configs
The following subsections are only relevant to those seeking to modify the Clang tooling configs, `.clang-tidy` and `.clang-format`.

## Tidy Config Testing Script
The files in this subsection are assumed to be in `utils/tidy_config_test`, unless stated otherwise.

`run.sh` is used to test `.clang-tidy` (from the project's root directory) against an auto-generated sample input file. Through comments, `scripts/tidy_test_template.cpp` specifies which lines should produce clang-tidy errors, and which should not. This file is used to generate `temp/tidy_test.cpp` by renaming identifiers containing "check" or "CHECK" to some variant of "yeserror", "YESERROR", "noerror", and "NOERROR", based on capitalization, and adding a unique numeric suffix. These numeric suffixes will show up in `temp/errors.txt` with the text "FALSE POSITIVE" or "MISSING ERROR" to indicate that a line produced an error when it shouldn't have, or produced no error when it should have. See `temp/tidy_test.cpp` to make sense of these numbers. When `temp/errors.txt` is empty, and the tool ran successfully, then the `.clang-tidy` config in the root directory produces the expected result.

If `temp/errors.txt` has false positives, check the clang-tidy output in `temp/output.txt` to see what's causing them.

## Format Config Choices
The `.clang-format` file was generated by first dumping the `microsoft` preset:
```
clang-format --style=microsoft --dump-config > .clang-format
```
and then modifying several options, explained below.
<br><br>


(AccessModifierOffset: -2 --> -4) line 3  
"public", "private", "protected" not indented from brace level.
<br><br>


(AlignAfterOpenBracket: Align --> BlockIndent) line 4  
When items between brackets don't fit on the line, add line break:
```
someLongFunction(
    argument1, argument2
)
```
Other options seem to cause problems with initializer lists near the ends of function
call parameters on long lines (the list gets put on the next line but indented
too far to the right). Example:
```
test_strip<elephants>(".......X.....O..X.....O.......", {
                                                            ".......X.....O",
                                                            "X.....O.......",
                                                        });
```
<br>


(AllowShortFunctionsOnASingleLine: None --> Empty) line 74  
Empty functions have braces on the same line:
```
cli_options::~cli_options() {}
```
<br>


(BraceWrapping.AfterCaseLabel: false --> true) line 86  
Brace on own line for case block.
<br><br>


(BraceWrapping.AfterUnion: false --> true) line 95  
Brace on own line for union.
<br><br>


(BraceWrapping.BeforeLambdaBody: false --> true) line 98  
Brace on own line for lambda function.
<br><br>


(BreakTemplateDeclarations: MultiLine --> Yes) line 118  
`template <class T>` on its own line.
<br><br>


(EmptyLineBeforeAccessModifier: LogicalBlock --> Always) line 128  
For access modifiers (i.e. "public" etc) which aren't the first thing in a block, an empty line is
included before. "LogicalBlock" is insufficient as it treats comments as empty lines.
<br><br>


(IndentCaseLabels: false --> true) line 155  
Case label indented inside of switch braces.
<br><br>


(PointerAlignment: Right --> Left) line 202  
Pointers on left side.
<br><br>


(SeparateDefinitionBlocks: Leave --> Always) line 212  
Empty lines between definitions for classes, structs, enums, and functions.
<br><br>


(SortIncludes: CaseSensitive --> Never) line 215  
Don't change "#include" ordering.
<br><br>


(SpaceAfterCStyleCast: false --> true) line 218:
```
(float) 4
// instead of:
(float)4
```
<br>


(SpaceBeforeCpp11BracedList: false --> true) line 224:
```
new int[3] {1, 2, 3};
// instead of
new int[3]{1, 2, 3};
```
