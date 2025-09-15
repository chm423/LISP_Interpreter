# LISP Interpreter Project - Sprints 1-3
### By: Cole Munn

## Overview
This project contains the beginning steps of a LISP interpreter with the ability to parse S-Expressions and call various functions on them.

The file structure is as follows:
```
	main.c
	README.md
	test_results.txt
```
with main.c including all of the code for the project. The main function is located at the very end of this C file.

To build the program, run the following commands:
```
	gcc main.c -o lisp
	./lisp
```

Currently, the main function includes a `runTests()` function which runs the tests laid out below, but there is also a commented-out input loop to further test S-Expression parsing and printing through standard input. Helper constructors were created for every atom type (e.g. `makeLong()`, `makeFloat()`, `makeSymbol()`, `makeString()`) and are a part of the `sexp()` constructor's implementation. These functions were primarily used in `runTests()`, except for when testing the `sexp()` constructor, which uses its proper constructor.
## Test Plan
The testing methodology for each sprint is as follows:
### Sprint 1: S-expression data structure
- nilp: test with empty list and single atom to verify nil status
- numberp: test with long, double, and symbol to verify number
- symbolp: test with symbol and string to verify if s-expression is symbol
- stringp: test with string and number to verify if s-expression contains enclosing quotes
- listp: test with nil, list, and symbol to verify list type
- sexpToBool: test with nil, cons cell, and long to verify true/false nature of s-expressions
- cons and cdr: test with dotted pairs to verify their correct display and parsing
- nested list: tested with nested list to verify correct display and parsing
- sexp constructor: tested with all atom types and list types (including variants) to verify correct parsing and converting of a string into each s-expression type
### Sprint 2: Predicate functions, cons
- car and cdr: test with full list, single atom, and nil to verify proper returning of list head/tail
- predicate functions (nilp, numberp, symbolp, stringp, listp) tested in sprint 1 to identify proper s-expression type
### Sprint 3: Number and logic Functions
- add, sub, mul: test with two longs and one long + one double
- div: test with two longs for whole number result, two longs for decimal result, and two longs for DivByZero result
- mod: test with two longs for non-zero result, two longs for zero result, and two longs for DivByZero result
- lt, gt, lte, gte: test with two long to return "t", "()", and variants with symbols to ensure only number type considered
- eq: test with two longs, one long and one float, symbols, strings, and lists to verify proper comparision of various types
- not: test with nil and symbol to verify proper inversion of boolean type

## Test Results
The results for the above tests are shown in `test_results.txt` in the project folder, displayed exactly as the terminal outputted them.

These results were made from a helper function `assertTest()` which printed the input, computed the result, and checked if the given result matched the expected one. 
Inputs shown in the results consist of inline S-Expressions declared within the assertTest function's argument

Passed tests display the result that was expected (which would match the given result), while failed tests provide the received result and what it expected instead.
## Documentation
Overall, the project appears fairly functional and performs as expected. The failed tests in listed in the results are to be expected as their correct implementation is not required for the project (e.g. equality functions between lists, longs and floats, etc.)
Some limitations of this program include:
- inability for input loop to parse s-expressions that span multiple lines
- no implementation of the quote function (as of yet)
- no implementation of lists for the `eq()` function
- `sexpToBool()` treats any non-nil as true
- arithmetic functions only consider 2 arguments
- many S-Expressions are not freed despite being dynamically allocated, causing memory leaks
- large lists will overflow buffer in `sexpToString()` (limit 256 chars)
- dotted pairs not read in properly (not required in assignment), '.' read as symbol
- sometimes, when evaluating true functions (e.g. nil?) the printed output will be blank instead of "t"