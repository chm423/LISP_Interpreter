#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <ctype.h>
#include <stdbool.h>

/* list types of atom */
typedef enum {
    ATOM_LONG, ATOM_DOUBLE, ATOM_SYMBOL, ATOM_STRING
} AtomType;

/* struct for atom: can be number | symbol | string */
typedef struct Atom {
    AtomType type;
    union { // allows storage of diff data types in one location
        long long_value;
        double double_value;
        char* symbol_value;
        char* string_value;
    } value;
} Atom;


/* struct for cons cell:
        building block of lists
        car (head): points to first s-expression
        cdr (tail): points to rest of list (list or nil)
*/
typedef struct ConsCell {
    struct SExp* car; // head ptr
    struct SExp* cdr; // tail ptr
} ConsCell;

/* enum list for s-expression types */
typedef enum {
    SEXP_ATOM, SEXP_LIST
} SExpType;

/* struct for s-expression: can be atom | list */
typedef struct SExp {
    SExpType type;
    union {
        Atom atom;
        ConsCell cons;
    } data;
} SExp;

// global nil object
SExp nil = { .type = SEXP_LIST, .data.cons = { .car = NULL, .cdr = NULL } };
// global truth object
SExp truth = {.type = SEXP_ATOM, .data.atom = {.type = ATOM_SYMBOL, .value.symbol_value = "t"}};

/* create new cons cell with supplied head and tail */
SExp *cons(SExp* car, SExp* cdr) {
    SExp* cell = malloc(sizeof(SExp)); 
    if (cell == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    cell->type = SEXP_LIST;
    cell->data.cons.car = car;
    cell->data.cons.cdr = cdr;
    return cell;
}

/* returns the car (head) of list */
SExp *car(SExp* list) {
    // if atom, return nil
    if (list->type != SEXP_LIST) {
        printf("Error: car called on Atom\n");
        return &nil;
    }
    if (list == &nil) {
        return &nil;
    }
    return list->data.cons.car;
}

/* returns the cdr (tail) of list */
SExp *cdr(SExp* list) {
    // if atom, return nil
    if (list->type != SEXP_LIST) {
        printf("Error: cdr called on Atom\n");
        return &nil;
    }
    if (list == &nil) {
        return &nil;
    }
    return list->data.cons.cdr;
}

/* constructor functions */
SExp* makeLong(long value) {
    SExp* atom = malloc(sizeof(SExp));
    atom->type = SEXP_ATOM;
    atom->data.atom.type = ATOM_LONG;
    atom->data.atom.value.long_value = value;
    return atom;
}
SExp* makeDouble(double value) {
    SExp* atom = malloc(sizeof(SExp));
    atom->type = SEXP_ATOM;
    atom->data.atom.type = ATOM_DOUBLE;
    atom->data.atom.value.double_value = value;
    return atom;
}
SExp* makeString(const char* value) {
    SExp* atom = malloc(sizeof(SExp));
    atom->type = SEXP_ATOM;
    atom->data.atom.type = ATOM_STRING;
    atom->data.atom.value.string_value = strdup(value);
    return atom;
}
SExp* makeSymbol(const char* value) {
    SExp* atom = malloc(sizeof(SExp));
    atom->type = SEXP_ATOM;
    atom->data.atom.type = ATOM_SYMBOL;
    atom->data.atom.value.symbol_value = strdup(value);
    return atom;
}

// skip spaces when reading input
void skipWhitespace(char** input) {
    while (**input && isspace(**input)) {
        (*input)++;
    }
}

SExp* parseAtom(char**input) {
    skipWhitespace(input); // skip leading spaces (if any)
    char* start = *input;

    // strings
    if (**input == '"') {
        (*input)++; // skip opening quote
        start = *input;

        while (**input && **input != '"') {
            (*input)++;
        }

        if (**input == '"') {
            int length = *input - start; 
            char *string = malloc(length + 1);
            strncpy(string, start, length);
            string[length] = '\0'; // append null-terminate
            (*input)++; // skip closing quote

            // construct new atom
            SExp* atom = makeString(string);
                // printf("[DEBUG] Parsed string: \"%s\"\n", string); // Debug message
            return atom;
        } else {
            printf("Error: Unterminated string\n");
            return &nil; 
        }
    }

    // numbers 
    if (isdigit(**input) || **input == '-' || **input == '.') {
        char* end;
        double value = strtod(*input, &end);
        if (end != *input && (*end == '\0' || isspace(*end) || *end == '(' || *end == ')')) { // valid number check
            *input = end;

            // double or long?
            if (value == (long)value) {
                // long
                SExp* atom = makeLong((long)value);
                    // printf("[DEBUG] Parsed long: %ld\n", (long)value); // Debug message
                return atom;
            }
            else {
                // double
                SExp* atom = makeDouble(value);
                    // printf("[DEBUG] Parsed double: %f\n", value); // Debug message
                return atom;
            }
        }
    }

    // symbols
    while (**input && !isspace(**input) && **input != '(' && **input != ')') {
        (*input)++; // increment until space or parentheses
    }

    int length = *input - start;
    char* symbol = malloc(length + 1);
    strncpy(symbol, start, length);
    symbol[length] = '\0'; // null-terminate

    // construct new atom
    SExp* atom = makeSymbol(symbol);
        // printf("[DEBUG] Parsed symbol: %s\n", symbol); // Debug message
    return atom;
}

SExp* readSExpHelper(char** input); // forward declaration for parseList

// parse list
SExp* parseList(char** input) {
    skipWhitespace(input);
    if (**input == ')') {
        (*input)++; 
        return &nil; // empty list
    }
    SExp* car = readSExpHelper(input); // parse head
    SExp* cdr = parseList(input); // parse tail
    return cons(car, cdr);
}

// recursive helper for readSExp
SExp* readSExpHelper(char** input) {
    skipWhitespace(input);

    if (**input == '(') {
        (*input)++;
        return parseList(input);
    }
    else if (**input == ')') {
        printf("Error: Unexpected ')'\n");
        return &nil;
    }
    else {
        return parseAtom(input);
    }
}

// read s-expression from string
SExp* sexp(const char* input) {
    char* inputCopy = strdup(input); // mutable copy
    char* cursor = inputCopy;
    SExp* result = readSExpHelper(&cursor);
    free(inputCopy);
    return result;
}

// print s-expression
void printSExp(SExp* sexp) {
    if (sexp->type == SEXP_ATOM) {
        // print based on atom type
        switch (sexp->data.atom.type) {
            case ATOM_LONG: 
                printf("%ld", sexp->data.atom.value.long_value);
                break;
            case ATOM_DOUBLE:
                printf("%f", sexp->data.atom.value.double_value);
                break;
            case ATOM_SYMBOL:
                printf("%s", sexp->data.atom.value.symbol_value);
                break;
            case ATOM_STRING:
                printf("\"%s\"", sexp->data.atom.value.string_value);
                break;
        }
    }
    else if (sexp->type == SEXP_LIST) {
        printf("(");

        // traverse list from car to cdr until nil
        SExp* current = sexp;
        while (current != &nil) {
            printSExp(current->data.cons.car); // recursive call to print

            // check if cdr is dotted pair
            if (current->data.cons.cdr != &nil && current->data.cons.cdr->type != SEXP_LIST) { 
                printf(" . "); 
                printSExp(current->data.cons.cdr); 
                break; 
            }

            current = current->data.cons.cdr;
            if (current != &nil) {
                printf(" ");
            }
        }
        printf(")");
    }
}

// helper function to convert to string (needed because of buffer string)
void sexpToStringHelper(SExp* s, char* buffer, size_t size) {
    // atom
    if (s->type == SEXP_ATOM) {
        switch (s->data.atom.type) {
            case ATOM_LONG:
                snprintf(buffer + strlen(buffer), size - strlen(buffer), "%ld", s->data.atom.value.long_value);
                break;
            case ATOM_DOUBLE:
                snprintf(buffer + strlen(buffer), size - strlen(buffer), "%f", s->data.atom.value.double_value);
                break;
            case ATOM_SYMBOL:
                snprintf(buffer + strlen(buffer), size - strlen(buffer), "%s", s->data.atom.value.symbol_value);
                break;
            case ATOM_STRING:
                snprintf(buffer + strlen(buffer), size - strlen(buffer), "\"%s\"", s->data.atom.value.string_value);
                break;
        }
    }
    // list
    else if (s->type == SEXP_LIST) {
        strncat(buffer, "(", size - strlen(buffer) - 1);
        SExp* current = s;
        while (current != &nil) {
            // recursive call to handle nested lists
            sexpToStringHelper(current->data.cons.car, buffer, size);

            // dotted pair
            if (current->data.cons.cdr != &nil && current->data.cons.cdr->type != SEXP_LIST) {
                strncat(buffer, " . ", size - strlen(buffer) - 1);
                sexpToStringHelper(current->data.cons.cdr, buffer, size);
                break;
            }

            current = current->data.cons.cdr;
            if (current != &nil) {
                strncat(buffer, " ", size - strlen(buffer) - 1);
            }
        }
        strncat(buffer, ")", size - strlen(buffer) - 1);
    }
}

// function to convert sexp to string (alternative to printSExp, similar implementation)
char* sexpToString(SExp* s) {
    static char buffer[256];
    buffer[0] = '\0';
    sexpToStringHelper(s, buffer, sizeof(buffer));
    return buffer;
}


/* predicate functions (sprint 2)*/

// check if s-expression is nil
SExp* nilp(SExp* sexp) {
    if (sexp == &nil) {
        return &truth;
    }
    else {
        return &nil;
    }

}
// check if s-expression is a symbol
SExp* symbolp (SExp* sexp) {
    if (sexp->type == SEXP_ATOM && sexp->data.atom.type == ATOM_SYMBOL) {
        return &truth;
    }
    else {
        return &nil;
    }
}
// check if s-expression is a number (long or double)
SExp* numberp (SExp* sexp) {
    if (sexp->type == SEXP_ATOM && (sexp->data.atom.type == ATOM_LONG || sexp->data.atom.type == ATOM_DOUBLE)) {
        return &truth;
    }
    else {
        return &nil;
    }
}
// check if s-expression is a string
SExp* stringp (SExp* sexp) {
    if (sexp->type == SEXP_ATOM && sexp->data.atom.type == ATOM_STRING) {
        return &truth;
    }
    else {
        return &nil;
    }
}
// check if s-expression is a list (cons cell or nil)
SExp* listp (SExp* sexp) {
    if (sexp->type == SEXP_LIST) {
        return &truth;
    }
    else {
        return &nil;
    }
}
// general boolean converter
bool sexpToBool (SExp* sexp) {
    return (sexp == &nil) ? false : true;
}

/* logic functions (sprint 3)*/
// helper to convert SExp to double (if applicable), pass to out
bool getNumber(SExp* sexp, double* out) {
    if (sexp->type == SEXP_ATOM) {
        if (sexp->data.atom.type == ATOM_LONG) {
            *out = (double)(sexp->data.atom.value.long_value);
            return true;
        }
        else if (sexp->data.atom.type == ATOM_DOUBLE) {
            *out = sexp->data.atom.value.double_value;
            return true;
        }
    }
    return false; // not a number
}

// add
SExp* add(SExp* a, SExp* b){
    double x,y;
    if (!getNumber(a, &x) || !getNumber(b, &y)) return sexp("NotANumber");
    double result = x + y;
    return (result == (long)result) ? makeLong((long)result) : makeDouble(result);
}
// subtract
SExp* sub(SExp* a, SExp* b){
    double x,y;
    if (!getNumber(a, &x) || !getNumber(b, &y)) return sexp("NotANumber");
    double result = x - y;
    return (result == (long)result) ? makeLong((long)result) : makeDouble(result);
}
// multiply
SExp* mul(SExp* a, SExp* b){
    double x,y;
    if (!getNumber(a, &x) || !getNumber(b, &y)) return sexp("NotANumber");
    double result = x * y;
    return (result == (long)result) ? makeLong((long)result) : makeDouble(result);
}
// divide (returns error if divide by 0)
SExp* divide(SExp* a, SExp* b){
    double x, y;
    if (!getNumber(a, &x) || !getNumber(b, &y)) return sexp("NotANumber");
    if (y == 0) return sexp("DivideByZero");
    double result = x / y;
    return (result == (long)result) ? makeLong((long)result) : makeDouble(result);
}
// modulo (only for long) (returns error if divide by 0)
SExp* mod(SExp* a, SExp* b){
    double x, y;
    if (!getNumber(a, &x) || !getNumber(b, &y)) return sexp("NotANumber");
    if ((long)y == 0) return sexp("DivideByZero");
    double result = (long)x % (long)y;
    return makeLong((long)result);
}
// less than
SExp* lt(SExp* a, SExp* b){
    double x, y;
    if (!getNumber(a, &x) || !getNumber(b, &y)) return sexp("NotANumber");
    return (x < y) ? &truth : &nil;
}
// greater than
SExp* gt(SExp* a, SExp* b){
    double x, y;
    if (!getNumber(a, &x) || !getNumber(b, &y)) return sexp("NotANumber");
    return (x > y) ? &truth : &nil;
}
// lte
SExp* lte(SExp* a, SExp* b){
    double x, y;
    if (!getNumber(a, &x) || !getNumber(b, &y)) return sexp("NotANumber");
    return (x <= y) ? &truth : &nil;
}
// gte
SExp* gte(SExp* a, SExp* b){
    double x, y;
    if (!getNumber(a, &x) || !getNumber(b, &y)) return sexp("NotANumber");
    return (x >= y) ? &truth : &nil;
}
// equality function: considers any atom type
SExp* eq(SExp* a, SExp* b){
    if (a->type != b->type) return sexp("TypeMismatch"); // different types
    if (a->type == SEXP_ATOM) {
        // both atoms, check atom type
        if (a->data.atom.type != b->data.atom.type) return sexp("TypeMismatch"); // different atom types
        switch (a->data.atom.type) {
            case ATOM_LONG:
                return (a->data.atom.value.long_value == b->data.atom.value.long_value) ? &truth : &nil;
            case ATOM_DOUBLE:
                return (a->data.atom.value.double_value == b->data.atom.value.double_value) ? &truth : &nil;
            case ATOM_SYMBOL:
                return (strcmp(a->data.atom.value.symbol_value, b->data.atom.value.symbol_value) == 0) ? &truth : &nil;
            case ATOM_STRING:
                return (strcmp(a->data.atom.value.string_value, b->data.atom.value.string_value) == 0) ? &truth : &nil;
        }
    }
    else if (a->type == SEXP_LIST) {
        return sexp("ListEquality");
    }
    return &nil; // fallback
}
// logical not: takes boolean atom and returns opposite
SExp* notf(SExp* a){
    return (a == &nil) ? &truth : &nil;
}

// testing functions
void assertTest(const char* testName, SExp* actual, const char* expected) {
    char* got = sexpToString(actual);
    if (strcmp(got, expected) == 0) {
        printf("PASSED: %s => %s\n", testName, got);
    } else {
        printf("FAILED: %s => got %s, expected %s\n", testName, got, expected);
    }
}
void runTests() {
    SExp* two = makeLong(2);
    SExp* three = makeLong(3);
    SExp* four = makeLong(4);
    SExp* five = makeLong(5);
    SExp* ten = makeLong(10);
    SExp* point5 = makeDouble(2.5);

    // Sprint 1
    printf("=== Sprint 1 Tests ===\n");

    // nil
    printf("--- nil check ---\n");
    assertTest("nilp( () )", nilp(&nil), "t");
    assertTest("nilp(5)", nilp(makeLong(5)), "()");

    // numbers
    printf("--- number check ---\n");
    assertTest("numberp(123)", numberp(makeLong(123)), "t");
    assertTest("numberp(3.14)", numberp(makeDouble(3.14)), "t");
    assertTest("numberp(x)", numberp(makeSymbol("x")), "()");

    // symbols
    printf("--- symbol check ---\n");
    assertTest("symbolp(x)", symbolp(makeSymbol("x")), "t");
    assertTest("symbolp(\"hi\")", symbolp(makeString("hi")), "()");

    // strings
    printf("--- string check ---\n");
    assertTest("stringp(\"hello\")", stringp(makeString("hello")), "t");
    assertTest("stringp(42)", stringp(makeLong(42)), "()");

    // lists
    printf("--- list check ---\n");
    assertTest("listp( () )", listp(&nil), "t");
    assertTest("listp(cons(1, ()))", listp(cons(makeLong(1), &nil)), "t");
    assertTest("listp(symbol)", listp(makeSymbol("y")), "()");

    // bools
    printf("--- bool check ---\n");
    assertTest("sexpToBool(())", sexpToBool(&nil) ? &truth : &nil, "()");
    assertTest("sexpToBool(cons(1, ()))", sexpToBool(cons(makeLong(1), &nil)) ? &truth : &nil, "t");
    assertTest("sexpToBool(5)", sexpToBool(makeLong(5)) ? &truth : &nil, "t");

    // cons, car, cdr
    printf("--- cons cells, car, cdr ---\n");
    SExp* lst = cons(makeSymbol("a"), cons(makeSymbol("b"), &nil));
    assertTest("cons(a,(b))", lst, "(a b)");
    assertTest("car((a b))", car(lst), "a");
    assertTest("cdr((a b))", cdr(lst), "(b)");

    // dotted pair
    printf("--- dotted pairs ---\n");
    SExp* dotted = cons(makeSymbol("x"), makeSymbol("y"));
    assertTest("cons(x,y)", dotted, "(x . y)");
    assertTest("cdr(a . b)", cdr(sexp("(a . b)")), "b");

    // nested list
    printf("--- nested list ---\n");
    SExp* nested = cons(makeSymbol("a"), cons(cons(makeLong(1), cons(makeLong(2), &nil)), &nil));
    assertTest("nested list (a (1 2))", nested, "(a (1 2))");

    // sexp constuctor
    printf("--- sexp constructor ---\n");
    assertTest("sexp(\"42\")", sexp("42"), "42");
    assertTest("sexp(\"3.14\")", sexp("3.14"), "3.140000");
    assertTest("sexp(\"hello\")", sexp("hello"), "hello");
    assertTest("sexp(\"\\\"hi\\\"\")", sexp("\"hi\""), "\"hi\"");
    assertTest("sexp(\"(a b c)\")", sexp("(a b c)"), "(a b c)");
    assertTest("sexp(\"(1 (2 3) 4)\")", sexp("(1 (2 3) 4)"), "(1 (2 3) 4)");
    assertTest("sexp(\"(a . b)\")", sexp("(a . b)"), "(a . b)");

    // Sprint 2
    printf("=== Sprint 2 Tests ===\n");
    // car
    printf("--- car ---\n");
    SExp* abList = cons(makeSymbol("a"), cons(makeSymbol("b"), &nil));
    assertTest("car((a b))", car(abList), "a");
    assertTest("car((5))", car(cons(makeLong(5), &nil)), "5");
    assertTest("car( () )", car(&nil), "()");

    // cdr
    printf("--- cdr ---\n");
    assertTest("cdr((a b))", cdr(abList), "(b)");
    assertTest("cdr((a))", cdr(cons(makeSymbol("a"), &nil)), "()");
    assertTest("cdr( () )", cdr(&nil), "()");

    // Sprint 3
    printf("=== Sprint 3 Tests ===\n");

    // arithmetic
    printf("--- arithmetic ---\n");
    assertTest("add(2,3)", add(two, three), "5");
    assertTest("add(2.5,3)", add(point5, three), "5.500000");

    assertTest("sub(10,4)", sub(ten, four), "6");
    assertTest("sub(10,2.5)", sub(ten, point5), "7.500000");

    assertTest("mul(3,4)", mul(three, four), "12");
    assertTest("mul(2.5,4)", mul(point5, four), "10");

    assertTest("divide(10,2)", divide(ten, two), "5");
    assertTest("divide(10,4)", divide(ten, four), "2.500000");
    assertTest("divide(10,0)", divide(ten, makeLong(0)), "DivideByZero");

    assertTest("mod(10,3)", mod(ten, three), "1");
    assertTest("mod(10,5)", mod(ten, five), "0");
    assertTest("mod(10,0)", mod(ten, makeLong(0)), "DivideByZero");

    // comparison
    printf("--- comparison ---\n");
    assertTest("lt(2,3)", lt(two, three), "t");
    assertTest("lt(3,2)", lt(three, two), "()");
    assertTest("lt(a,b)", lt(makeSymbol("a"), makeSymbol("b")), "NotANumber");
    assertTest("lt(2,a)", lt(two, makeSymbol("a")), "NotANumber");

    assertTest("gt(5,2)", gt(five, two), "t");
    assertTest("gt(2,5)", gt(two, five), "()");
    assertTest("gt(a,b)", gt(makeSymbol("a"), makeSymbol("b")), "NotANumber");
    assertTest("gt(2,b)", gt(two, makeSymbol("b")), "NotANumber");

    assertTest("lte(2,2)", lte(two, two), "t");
    assertTest("lte(3,2)", lte(three, two), "()");
    assertTest("lte(a,b)", lte(makeSymbol("a"), makeSymbol("b")), "NotANumber");
    assertTest("lte(2,b)", lte(two, makeSymbol("b")), "NotANumber");

    assertTest("gte(3,2)", gte(three, two), "t");
    assertTest("gte(2,3)", gte(two, three), "()");
    assertTest("gte(a,b)", gte(makeSymbol("a"), makeSymbol("b")), "NotANumber");
    assertTest("gte(2,b)", gte(two, makeSymbol("b")), "NotANumber");

    // equality
    printf("--- equality ---\n");
    assertTest("eq(2,2)", eq(two, makeLong(2)), "t");
    assertTest("eq(2,2.5)", eq(two, point5), "()");
    assertTest("eq(a,a)", eq(makeSymbol("a"), makeSymbol("a")), "t");
    assertTest("eq(a,b)", eq(makeSymbol("a"), makeSymbol("b")), "()");
    assertTest("eq(\"hi\",\"hi\")", eq(makeString("hi"), makeString("hi")), "t");

    SExp* list1 = cons(makeLong(1), cons(makeLong(2), cons(makeLong(3), &nil)));
    SExp* list2 = cons(makeLong(1), cons(makeLong(2), cons(makeLong(3), &nil)));
    assertTest("eq((1 2 3),(1 2 3))", eq(list1, list2), "t");

    // logical
    printf("--- logical ---\n");
    assertTest("not(nil)", notf(&nil), "t");
    assertTest("not(t)", notf(&truth), "()");
}




int main(){
    // runTests();
    // input loop, handle input from stdin

    char input[100];
    printf("Type additional s-expressions for testing, 'exit' to quit.\n");
    while (strcmp(input, "exit\0") != 0){
        if (fgets(input, sizeof(input), stdin) != NULL){
            // handle \n character
            char *pos;
            if ((pos = strchr(input, '\n')) != NULL) {
                *pos = '\0';
            }
            SExp* s = sexp(input);

            // printSExp(s);
            printf("%s\n", sexpToString(s));
            printf("isSymbol: %s\n", sexpToBool(symbolp(s)) ? "true" : "false");
            printf("isNumber: %s\n", sexpToBool(numberp(s)) ? "true" : "false");
            printf("isString: %s\n", sexpToBool(stringp(s)) ? "true" : "false");
            printf("isList: %s\n", sexpToBool(listp(s)) ? "true" : "false");
            printf("isNil: %s\n", sexpToBool(nilp(s)) ? "true" : "false");
        }
        else {
            printf("invalid input");
        }
    }
    return 0;
}