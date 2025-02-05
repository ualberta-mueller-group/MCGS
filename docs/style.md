# Style guide

This document outlines the style used in C++ source files within this project, then explains configuration options 


- Code style summary
- Style tools
- Configuration options
- Other scripts






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



============================================================

Require "override" where applicable

No "using" keyword in headers


llvm-namespace-comment



identifiers: lower_case
macro: UPPER_CASE

constexpr: UPPER_CASE

private member (both field and method): prefix with "_"


methods first, THEN fields later
