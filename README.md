# LISP Interpreter Project
### By: Cole Munn

## Overview
This project contains a basic LISP interpreter with the ability to parse s-expressions and call various functions on them. Functionality includes arithmetic, comparison, and logic functions as well as the implementation of user-defined/lambda functions.

The file structure is as follows:
```
	main.c - full program
	README.md - this file
	test_results.txt - output location for testing with -test
	insertionSort.lisp - test input
	quickSort.lisp - test input
	mergeSort.lisp - test input
```
with main.c including all of the code for the project. The main function is located at the very end of this C file.
The three sorting files included were supplied by the instructor and were adjusted to work with this interpreter; they were used for additional testing and verification of interpreter functionality.

To build the program, run the following commands:
```
	gcc main.c -o lisp
	./lisp <args>
```
The program can be run in **3** modes, depending on the arguments provided (if any):
1. If a .lisp file is provided (i.e. `./lisp insertionSort.lisp`), the program will attempt to open and use the given file for input. 
2. If instead `-test` is used as the argument, the program will run a series of hardcoded tests to determine program functionality from each sprint, all of which is written to `test_results.txt`. 
3. Otherwise, if no argument is presented, the program will automatically use a REPL loop from standard input.



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
### Sprint 3: Number and logic functions
- add, sub, mul: test with two longs and one long + one double
- div: test with two longs for whole number result, two longs for decimal result, and two longs for DivByZero result
- mod: test with two longs for non-zero result, two longs for zero result, and two longs for DivByZero result
- lt, gt, lte, gte: test with two long to return "t", "()", and variants with symbols to ensure only number type considered
- eq: test with two longs, one long and one float, symbols, strings, and lists to verify proper comparison of various types
- not: test with nil and symbol to verify proper inversion of boolean type

### Sprint 4: 
- (submission sprints 1-3, no tests)

### Sprint 5: Eval function
- set: test setting variables to numbers, strings, lists, and expressions
- lookup: test retrieving previously set variables and ensure undefined variables return themselves
- arithmetic with variables: test (add x 3) after setting x to a number
- nested expressions: test (add (mul 2 3) (sub 10 4)) for correct evaluation order
- quoting: test setting variable to a quoted function to verify quoted s-expressions are not evaluated
- cons and list operations: test setting to manually built lists to ensure car/cdr work after assignment
### Sprint 6: Short-circuiting and conditionals
- and, or: test with combinations of t, (), etc to verify evaluation
- if: test both true and false branches with numbers, strings
- cond: test multiple clauses, including fallback clause 't, empty clause, and short-circuited clauses
### Sprint 7: User defined functions
- simple single-argument function: define, call with correct type, call with wrong type
- multiple arguments: define two-argument function, call with correct number, too few, and too many
- nested calls: function calling another user-defined function to verify local environment handling
- recursive functions: define factorial to test correct recursive implementation
- error handling: call undefined function, call with wrong argument type, call with wrong number of arguments
### Sprint 8: Lambda functions
- lambda calling: verify a simple function returns desired value
- assigning lambda to variable: test using define on a lambda to ensure proper behavior after calling function
- multiple arguments: testing lambda calling with more than one argument to ensure all are considered
- nested calls: calling a lambda within a lambda to verify proper solving order
- error handling: considering mismatching argument numbers and types

## Test Results
The results for the above tests are shown in `test_results.txt` in the project folder, displayed exactly as the interpreter outputted them.

These results were made from a helper function `assertTest()` which printed the input, computed the result, and checked if the given result matched the expected one. 
Inputs shown in the results consist of inline S-Expressions declared within the assertTest function's argument

Passed tests display the result that was expected (which would match the given result), while failed tests provide the received result and what it expected instead.
## Documentation
### Things to consider when utilizing this interpreter:
- in standard input, if no closing parentheses are provided for a statement, the interpreter will continue to add onto the previous statement until the statement is properly closed
- in file input, if an s-expression is not properly closed, the interpreter will ignore it and all following s-expressions
- when utilizing `cond`, the interpreter requires each branch to be a list of two s-expressions; for example:
```
	(cond exp
		(e1 e2)
		(e3 e4)
		('t e5)
	)
```
- conditional functions (such as `eq`, `lt`, `gt`) will return an error symbol if there is a type mismatch, meaning false positives could be returned in user-defined functions if incorrect input is supplied
- you cannot utilize `define` to override pre-defined functions (they will be set, but cannot be retrieved)
	- the same goes for atoms like numbers (e.g. `(set 11 x)` will always return 11)
- 

### Some limitations of this program include:
- no implementation of lists for the `eq()` function*
- setting a symbol to a quoted s-expression will not evaluate the quoted s-expression on further calling of the symbol
- arithmetic functions (`add`, `sub`, `mul`, etc.) and logic functions (`and`, `or`, `lt`, `gt`, etc.) will only consider the first two arguments*; everything following these two will be discarded
	- if there is only one argument, the interpreter will assume the second is nil, returning a "not a number" error
- dotted pairs not read in properly*, '.' read as symbol
- `and`, `or` assume the user only inputs 2 s-expressions following the declaration, meaning any additional inputs won't be considered
- higher order functions, such as `map` or `apply`, are not implemented in this interpreter*

\* instructor specified functionality not required for assignment