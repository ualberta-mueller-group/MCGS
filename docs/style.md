# Style guide

This document gives details about coding style for C++ files in this project, and tools relating to style.

## Sections
- [Code Style](#code-style)
- [Style Tooling](#style-tooling)
- [Other Scripts](#other-scripts)
- [Configuration Options](#configuration-options)


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
- Global/static/enum constants, macros, and constexprs should be upper case
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
public:
    void some_method1();
    void some_method2();

    int some_field1;
    int some_field2;

private:
    void _some_method3();

    int _some_field3;
};
```

Additionally:
- Include 1 empty line above access specifiers (i.e. public/protected/private), unless the access specifier comes immediately after the opening brace
- Access specifiers aren't indented past the opening/closing brace level

### Override
Implementations of virtual methods should use either the `override` or `final` keywords. The linter tools explained in following sections should catch missing occurrences of these.

### Namespaces
End namespace blocks with a comment like "namespace your_namespace_name":
```
namespace cgt
{
...
} // namespace cgt
```

### Implementations in headers
Function/method definitions should generally go in `.cpp` files and not `.h` files, unless the function/method is both short and declared `inline`. Definitions possibly leading to [ODR violations](https://en.cppreference.com/w/cpp/language/definition) should be caught by the linter tools explained in following sections.

### "using" keyword
The `using` keyword should not appear in headers. This should be caught by our linter tools, (i.e. `using namespace std;`, `using std::vector;`).

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
TODO clang-format, clang-tidy

# Other Scripts
TODO tidy_test_template.cpp

# Configuration Options
This section explains some configuration options chosen 



(AccessModifierOffset: -2 --> -4) #3
"public", "private", "protected" not indented from brace level


(AlignAfterOpenBracket: Align --> BlockIndent) #4
When items between brackets don't fit on the line, add line break
i.e.:
    someLongFunction(
        argument1, argument2
    )
Other options seem to cause problems with initializer lists near the ends of function
call parameters on long lines (the list gets put on the next line but indented
too far to the right). Example:
test_strip<elephants>(".......X.....O..X.....O.......", {
                                                            ".......X.....O",
                                                            "X.....O.......",
                                                        });


(AllowShortFunctionsOnASingleLine: None --> Empty) #74
Empty functions have braces on the same line i.e.:
    cli_options::~cli_options() {}


(BraceWrapping.AfterCaseLabel: false --> true) #86
Brace on own line for case block


(BraceWrapping.AfterUnion: false --> true) #95
Brace on own line for union


(BraceWrapping.BeforeLambdaBody: false --> true) #98
Brace on own line for lambda function


(BreakTemplateDeclarations: MultiLine --> Yes) #118
"template <class T>" on its own line


(EmptyLineBeforeAccessModifier: LogicalBlock --> Always) #128
For access modifiers (i.e. "public" etc) other than the first, an empty line is
included before. "LogicalBlock" is insufficient as it treats comments as empty lines


(IndentCaseLabels: false --> true) #155
Case label indented inside of switch braces


(PointerAlignment: Right --> Left) #202
Pointers on left side


(SeparateDefinitionBlocks: Leave --> Always) #212
Empty lines between definitions for classes, structs, enums, and functions


(SortIncludes: CaseSensitive --> Never) #215
Don't change "#include" ordering


(SpaceAfterCStyleCast: false --> true) #218
i.e. "(float) 4" instead of "(float)4"


(SpaceBeforeCpp11BracedList: false --> true) #224
i.e. "new int[3] {1, 2, 3};" instead of "new int[3]{1, 2, 3};"
