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

struct SExp;

/* struct for cons cell:
        building block of lists
        car (head): points to first s-expression
        cdr (tail): points to rest of list (list or nil)
*/
typedef struct ConsCell {
    struct SExp* car; // head ptr
    struct SExp* cdr; // tail ptr
} ConsCell;



typedef struct Lambda {
    struct SExp* params; // parameter list
    struct SExp* body;   // function body
    struct Env* env;  // closure environment
} Lambda;

/* enum list for s-expression types */
typedef enum {
    SEXP_ATOM, SEXP_LIST, SEXP_LAMBDA
} SExpType;

/* struct for s-expression: can be atom | list | lambda */
typedef struct SExp {
    SExpType type;
    union {
        Atom atom;
        ConsCell cons;
        Lambda func;
    } data;
} SExp;

// global nil object
SExp nil = { .type = SEXP_LIST, .data.cons = { .car = NULL, .cdr = NULL } };
// global truth object
SExp truth = {.type = SEXP_ATOM, .data.atom = {.type = ATOM_SYMBOL, .value.symbol_value = "t"}};
// global environment: parallel lists of symbols and vals
typedef struct Env {
    SExp* symbols;
    SExp* values;
    struct Env* parent;
} Env;

Env* globalEnv = NULL;

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
        return makeSymbol("Error: cdr called on Atom");
    }
    if (list == &nil) {
        return &nil;
    }
    return list->data.cons.cdr;
}

// second element in list
SExp* cadr(SExp* x) {
    return car(cdr(x));
}
// third element
SExp* caddr(SExp* x) {
    return car(cdr(cdr(x)));
}
// fourth element
SExp* cadddr(SExp* x) {
    return car(cdr(cdr(cdr(x))));
}



// skip spaces when reading input
void skipWhitespace(char** input) {
    while (**input && isspace(**input)) {
        (*input)++;
    }
}
// remove comments from input string
void stripComment (char* line) {
    char* commentStart = strchr(line, ';'); // find first semicolon
    if (commentStart) {
        *commentStart = '\0'; // terminate string at comment start
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
            return makeSymbol("Error: Unterminated string"); 
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

    // handle quote shortcut '
    if (**input == '\'') {
        (*input)++; // skip quote
        SExp* quoted = readSExpHelper(input);
        return cons(makeSymbol("quote"), cons(quoted, &nil));
    }
    if (**input == '(') {
        (*input)++;
        return parseList(input);
    }
    else if (**input == ')') {
        return makeSymbol("Error: Unexpected ')'");
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
    if (!getNumber(a, &x) || !getNumber(b, &y)) return makeSymbol("Error: Operand not a number");
    double result = x + y;
    return (result == (long)result) ? makeLong((long)result) : makeDouble(result);
}
// subtract
SExp* sub(SExp* a, SExp* b){
    double x,y;
    if (!getNumber(a, &x) || !getNumber(b, &y)) return makeSymbol("Error: Operand not a number");
    double result = x - y;
    return (result == (long)result) ? makeLong((long)result) : makeDouble(result);
}
// multiply
SExp* mul(SExp* a, SExp* b){
    double x,y;
    if (!getNumber(a, &x) || !getNumber(b, &y)) return makeSymbol("Error: Operand not a number");
    double result = x * y;
    return (result == (long)result) ? makeLong((long)result) : makeDouble(result);
}
// divide (returns error if divide by 0)
SExp* divide(SExp* a, SExp* b){
    double x, y;
    if (!getNumber(a, &x) || !getNumber(b, &y)) return makeSymbol("Error: Operand not a number");
    if (y == 0) return makeSymbol("Error: Divide by zero");
    double result = x / y;
    return (result == (long)result) ? makeLong((long)result) : makeDouble(result);
}
// modulo (only for long) (returns error if divide by 0)
SExp* mod(SExp* a, SExp* b){
    double x, y;
    if (!getNumber(a, &x) || !getNumber(b, &y)) return makeSymbol("Error: Operand not a number");
    if ((long)y == 0) return makeSymbol("Error: Divide by zero");
    double result = (long)x % (long)y;
    return makeLong((long)result);
}
// less than
SExp* lt(SExp* a, SExp* b){
    double x, y;
    if (!getNumber(a, &x) || !getNumber(b, &y)) return makeSymbol("Error: Operand not a number");
    return (x < y) ? &truth : &nil;
}
// greater than
SExp* gt(SExp* a, SExp* b){
    double x, y;
    if (!getNumber(a, &x) || !getNumber(b, &y)) return makeSymbol("Error: Operand not a number");
    return (x > y) ? &truth : &nil;
}
// lte
SExp* lte(SExp* a, SExp* b){
    double x, y;
    if (!getNumber(a, &x) || !getNumber(b, &y)) return makeSymbol("Error: Operand not a number");
    return (x <= y) ? &truth : &nil;
}
// gte
SExp* gte(SExp* a, SExp* b){
    double x, y;
    if (!getNumber(a, &x) || !getNumber(b, &y)) return makeSymbol("Error: Operand not a number");
    return (x >= y) ? &truth : &nil;
}
// equality function: considers any atom type
SExp* eq(SExp* a, SExp* b){
    if (a->type != b->type) return makeSymbol("Error: Type mismatch"); // different types
    if (a->type == SEXP_ATOM) {
        // both atoms, check atom type

        // handle numeric equality (long and double)
        if ((a->data.atom.type == ATOM_LONG || a->data.atom.type == ATOM_DOUBLE) &&
            (b->data.atom.type == ATOM_LONG || b->data.atom.type == ATOM_DOUBLE)) {
            double x, y;
            getNumber(a, &x);
            getNumber(b, &y);
            return (x == y) ? &truth : &nil;
        }


        if (a->data.atom.type != b->data.atom.type) return makeSymbol("Error: Type mismatch"); // different atom types
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
        return makeSymbol("Error: eq called on lists");
    }
    return &nil; // fallback
}
// logical not: takes boolean atom and returns opposite
SExp* notf(SExp* a){
    return (a == &nil) ? &truth : &nil;
}

/* Sprint 5 functions */

// lookup: find value from symbol in environment
SExp* lookup(SExp* symbol, Env* env) {

    for (Env* e = env; e != NULL; e = e-> parent) { // iterate through environments if not found
        SExp* syms = e->symbols;
        SExp* vals = e->values;
        while (syms != &nil && vals != &nil) {
            if (strcmp(car(syms)->data.atom.value.symbol_value, symbol->data.atom.value.symbol_value) == 0) {
                return car(vals);
            }
            syms = cdr(syms);
            vals = cdr(vals);
        }
    }

    return symbol;
}

// adds new local env to chain of environments
Env* consEnv(SExp* params, SExp* args, Env* parent) {
    Env* e = malloc(sizeof(Env));
    e->symbols = params;
    e->values = args;
    e->parent = parent;
    return e;
}
// helper to get length of list (for argument matching)
int listLength (SExp* list) {
    int count = 0;
    while (list != &nil) {
        count++;
        list = cdr(list);
    }
    return count;
}
SExp* reverseList(SExp* list){
    SExp* result = &nil;
    while (list != &nil){
        result = cons(car(list), result);
        list = cdr(list);
    }
    return result;
}
Env* extendEnv (SExp* params, SExp* args, Env* parent) { 
    Env* newEnv = malloc(sizeof(Env));
    newEnv->symbols = &nil;
    newEnv->values = &nil;
    newEnv->parent = parent;

    while (params != &nil && args != &nil) {
        newEnv->symbols = cons(car(params), newEnv->symbols);
        newEnv->values = cons(car(args), newEnv->values);
        params = cdr(params);
        args = cdr(args);
    }

    // reverse so first pair is at head
    newEnv->symbols = reverseList(newEnv->symbols);
    newEnv->values = reverseList(newEnv->values);
    return newEnv;
}



// set: add symbol-value pair to environment, will overwrite existing through recency in environment
SExp* set(SExp* symbol, SExp* value, Env* env) {
    env->symbols = cons(symbol, env->symbols);
    env->values = cons(value, env->values);
    return value; // return stored value
}

SExp* eval (SExp* sexp, Env* env) {
    if (nilp(sexp) == &truth) return &nil; // nil returns nil

    // atoms
    if (sexp->type == SEXP_ATOM) {
        switch (sexp->data.atom.type) {
            case ATOM_LONG:
            case ATOM_DOUBLE:
            case ATOM_STRING:
                return sexp; // self-evaluating
            case ATOM_SYMBOL:
                return lookup(sexp, env); // lookup in environment
        }
    }

    // lists
    if (sexp->type == SEXP_LIST) {
        SExp* func = car(sexp);
        SExp* args = cdr(sexp);

        if (func->type == SEXP_ATOM && func->data.atom.type == ATOM_SYMBOL) {

            char* fname = func->data.atom.value.symbol_value;

            // handle special forms
            if (strcmp(fname, "quote") == 0) {
                return car(args); // return quoted expression   
            }
            if (strcmp(fname, "set") == 0) {
                SExp* var = car(args);
                SExp* val = eval(cadr(args), env);
                return set(var, val, env);
            }
            if (strcmp(fname, "define") == 0) {
                SExp* name = car(args);
                SExp* value = cadr(args);
                SExp* func = NULL;

                if (value->type == SEXP_LIST && car(value)->type == SEXP_ATOM && strcmp(car(value)->data.atom.value.symbol_value, "lambda") == 0) {
                    func = eval(value, env);
                }
                else {
                    SExp* params = cadr(args);
                    SExp* body = caddr(args);

                    func = malloc(sizeof(SExp));
                    func->type = SEXP_LAMBDA;
                    func->data.func.params = params;
                    func->data.func.body = body;
                    func->data.func.env = env;
                }

                set(name, func, env);

                return name;
            }
            if (strcmp(fname, "lambda") == 0) {
                SExp* params = car(args);
                SExp* body = cadr(args);

                SExp* func = malloc(sizeof(SExp));
                func->type = SEXP_LAMBDA;
                func->data.func.params = params;
                func->data.func.body = body;
                func->data.func.env = env;

                return func;
            }


            // lists
            if (strcmp(fname, "cons") == 0) {
                SExp* head = eval(car(args), env);
                SExp* tail = eval(cadr(args), env);
                return cons(head, tail);
            }
            if (strcmp(fname, "car") == 0) {
                return car(eval(car(args), env));
            }
            if (strcmp(fname, "cdr") == 0) {
                return cdr(eval(car(args), env));
            }

            // short-circuiting functions
            if (strcmp(fname, "and") == 0) {
                SExp* first = eval(car(args), env);
                if (first == &nil) return &nil;
                return eval(cadr(args), env);
            }
            if (strcmp(fname, "or") == 0) {
                SExp* first = eval(car(args), env);
                if (first != &nil) return &truth;
                return eval(cadr(args), env);
            }
            // conditionals
            if (strcmp(fname, "if") == 0) {
                SExp* test = eval(car(args), env);
                if (test != &nil) {
                    return eval(cadr(args), env); // true branch
                } else {
                    return eval(caddr(args), env); // false branch
                }
            }
            if (strcmp(fname, "cond") == 0){
                SExp* clause = args;
                while (clause != &nil) {
                    SExp* pair = car(clause); // should be a pair of test and result
                    SExp* test = car(pair);
                    SExp* result = cadr(pair);
                    if (eval(test, env) != &nil) {
                        return eval(result, env); // return result of first true clause
                    }
                    clause = cdr(clause); // else go to next pair
                }
                return makeSymbol("Error: No selected branch"); // no clause matched
            }

            // other built-in functions
            if (strcmp(fname, "add") == 0) {
                return add(eval(car(args), env), eval(cadr(args), env));
            }
            if (strcmp(fname, "sub") == 0) {
                return sub(eval(car(args), env), eval(cadr(args), env));
            }
            if (strcmp(fname, "mul") == 0) {
                return mul(eval(car(args), env), eval(cadr(args), env));
            }
            if (strcmp(fname, "div") == 0) {
                return divide(eval(car(args), env), eval(cadr(args), env));
            }
            if (strcmp(fname, "mod") == 0) {
                return mod(eval(car(args), env), eval(cadr(args), env));
            }
            if (strcmp(fname, "lt") == 0) {
                return lt(eval(car(args), env), eval(cadr(args), env));
            }
            if (strcmp(fname, "gt") == 0) {
                return gt(eval(car(args), env), eval(cadr(args), env));
            }
            if (strcmp(fname, "lte") == 0) {
                return lte(eval(car(args), env), eval(cadr(args), env));
            }
            if (strcmp(fname, "gte") == 0) {
                return gte(eval(car(args), env), eval(cadr(args), env));
            }
            if (strcmp(fname, "eq") == 0) {
                return eq(eval(car(args), env), eval(cadr(args), env));
            }
            if (strcmp(fname, "not") == 0) {
                return notf(eval(car(args), env));
            }
            if (strcmp(fname, "nil?") == 0) {
                return nilp(eval(car(args), env));
            }
            if (strcmp(fname, "symbol?") == 0) {
                return symbolp(eval(car(args), env));
            }
            if (strcmp(fname, "number?") == 0) {
                return numberp(eval(car(args), env));
            }
            if (strcmp(fname, "string?") == 0) {
                return stringp(eval(car(args), env));
            }
            if (strcmp(fname, "list?") == 0) {
                return listp(eval(car(args), env));
            }
        }
        // check for user-defined function
        SExp* op = eval(func, env);

        if (op->type == SEXP_LAMBDA) {
            // check arg count
            int expected = listLength(op->data.func.params);
            int given = listLength(args);

            if (expected != given) {
                return makeSymbol("Error: Argument count mismatch");
            }

            // eval args
            SExp* evaluatedArgs = &nil;
            SExp* formalParams = op->data.func.params;
            SExp* actuals = args;

            while (actuals != &nil) {
                evaluatedArgs = cons(eval(car(actuals), env), evaluatedArgs);
                actuals = cdr(actuals);
            }
            evaluatedArgs = reverseList(evaluatedArgs);

            // extend enviro
            Env* newEnv = extendEnv(formalParams, evaluatedArgs, op->data.func.env);

            // eval body in new env
            return eval (op->data.func.body, newEnv);

        }
        if (op->type != SEXP_LAMBDA) return sexp;
    }
    return makeSymbol("EvalError"); // fallback
}

// testing functions
SExp* evalString(const char* input) {
    SExp* expr = sexp(input);   // parse string into an S-expression
    if (!expr) {
        printf("ParseError: %s\n", input);
        return &nil;             // return nil on parse failure
    }
    SExp* result = eval(expr, globalEnv);   // evaluate the S-expression
    return result;
}
void assertTest(FILE *file, const char* testName, SExp* actual, const char* expected) {
    char* got = sexpToString(actual);
    if (strcmp(got, expected) == 0) {
        fprintf(file, "PASSED: %s => %s\n", testName, got);
    } else {
        fprintf(file, "FAILED: %s => got %s, expected %s\n", testName, got, expected);
    }
}

void runTests(const char* fileName) {
    FILE *file = fopen(fileName, "w");
    if (!file) {
        perror("Failed to open file");
        return;
    }

    if (!globalEnv) {
        globalEnv = malloc(sizeof(Env));
        globalEnv->symbols = &nil;
        globalEnv->values = &nil;
        globalEnv->parent = NULL;
    }

    SExp* two = makeLong(2);
    SExp* three = makeLong(3);
    SExp* four = makeLong(4);
    SExp* five = makeLong(5);
    SExp* ten = makeLong(10);
    SExp* point5 = makeDouble(2.5);

    // Sprint 1
    fprintf(file, "=== Sprint 1 Tests ===\n");

    // nil
    fprintf(file, "--- nil check ---\n");
    assertTest(file, "(nil? ())", nilp(&nil), "t");
    assertTest(file, "(nil? 5)", nilp(makeLong(5)), "()");
    assertTest(file, "(nil? (nilp()))", nilp(nilp(&nil) ? &truth : &nil), "()");

    // numbers
    fprintf(file, "--- number check ---\n");
    assertTest(file, "(number? 123)", numberp(sexp("123")), "t");
    assertTest(file, "(number? 3.14)", numberp(sexp("3.14")), "t");
    assertTest(file, "(number? x)", numberp(sexp("x")), "()");
    assertTest(file, "(number? 9223372036854775807)", numberp(sexp("9223372036854775807")), "t");
    assertTest(file, "(number? -42)", numberp(makeLong(-42)), "t");

    // symbols
    fprintf(file, "--- symbol check ---\n");
    assertTest(file, "(symbol? x)", symbolp(sexp("x")), "t");
    assertTest(file, "(symbol? \"hi\")", symbolp(sexp("\"hi\"")), "()");
    assertTest(file, "(symbol? \"\")", symbolp(sexp("\"\"")), "()");
    assertTest(file, "(symbol? @#$)", symbolp(sexp("@#$")), "t");

    // strings
    fprintf(file, "--- string check ---\n");
    assertTest(file, "(string? \"hello\")", stringp(sexp("\"hello\"")), "t");
    assertTest(file, "(string? 42)", stringp(sexp("42")), "()");
    assertTest(file, "(string? \"\")", stringp(sexp("\"\"")), "t");
    assertTest(file, "(string? \"\\n\")", stringp(sexp("\"\\n\"")), "t");

    // lists
    fprintf(file, "--- list check ---\n");
    assertTest(file, "(list? ())", listp(&nil), "t");
    assertTest(file, "(list? (cons 1 ()))", listp(cons(makeLong(1), &nil)), "t");
    assertTest(file, "(list? y)", listp(makeSymbol("y")), "()");
    assertTest(file, "(list? (()))", listp(cons(&nil, &nil)), "t");

    // bools
    fprintf(file, "--- bool check ---\n");
    assertTest(file, "(sexpToBool ())", sexpToBool(&nil) ? &truth : &nil, "()");
    assertTest(file, "(sexpToBool (cons 1 ()))", sexpToBool(cons(makeLong(1), &nil)) ? &truth : &nil, "t");
    assertTest(file, "(sexpToBool 5)", sexpToBool(five) ? &truth : &nil, "t");

    // cons, car, cdr
    fprintf(file, "--- cons cells, car, cdr ---\n");
    SExp* lst = cons(makeSymbol("a"), cons(makeSymbol("b"), &nil));
    assertTest(file, "(cons a (b))", lst, "(a b)");
    assertTest(file, "(car (a b))", car(lst), "a");
    assertTest(file, "(cdr (a b))", cdr(lst), "(b)");
        assertTest(file, "(car ((1 2) 3))", car(cons(cons(makeLong(1), cons(two, &nil)), cons(three, &nil))), "(1 2)");

    // dotted pair
    fprintf(file, "--- dotted pairs ---\n");
    SExp* dotted = cons(makeSymbol("x"), makeSymbol("y"));
    assertTest(file, "(cons x y)", dotted, "(x . y)");
    assertTest(file, "(cdr (a . b))", cdr(sexp("(a . b)")), "b");

    // nested list
    fprintf(file, "--- nested list ---\n");
    SExp* nested = cons(makeSymbol("a"), cons(cons(makeLong(1), cons(makeLong(2), &nil)), &nil));
    assertTest(file, "(a (1 2))", nested, "(a (1 2))");

    // sexp constructor
    fprintf(file, "--- sexp constructor ---\n");
    assertTest(file, "(sexp \"42\")", sexp("42"), "42");
    assertTest(file, "(sexp \"3.14\")", sexp("3.14"), "3.140000");
    assertTest(file, "(sexp \"hello\")", sexp("hello"), "hello");
    assertTest(file, "(sexp \"\\\"hi\\\"\")", sexp("\"hi\""), "\"hi\"");
    assertTest(file, "(sexp \"(a b c)\")", sexp("(a b c)"), "(a b c)");
    assertTest(file, "(sexp \"(1 (2 3) 4)\")", sexp("(1 (2 3) 4)"), "(1 (2 3) 4)");
    assertTest(file, "(sexp \"(a . b)\")", sexp("(a . b)"), "(a . b)");

    // Sprint 2
    fprintf(file, "=== Sprint 2 Tests ===\n");
    // car
    fprintf(file, "--- car ---\n");
    SExp* abList = cons(makeSymbol("a"), cons(makeSymbol("b"), &nil));
    assertTest(file, "(car (a b))", car(abList), "a");
    assertTest(file, "(car (5))", car(cons(makeLong(5), &nil)), "5");
    assertTest(file, "(car ())", car(&nil), "()");

    // cdr
    fprintf(file, "--- cdr ---\n");
    assertTest(file, "(cdr (a b))", cdr(abList), "(b)");
    assertTest(file, "(cdr (a))", cdr(cons(makeSymbol("a"), &nil)), "()");
    assertTest(file, "(cdr ())", cdr(&nil), "()");

    // Sprint 3
    fprintf(file, "=== Sprint 3 Tests ===\n");
    fprintf(file, "--- arithmetic ---\n");
    assertTest(file, "(add 2 3)", add(two, three), "5");
    assertTest(file, "(add 2.5 3)", add(point5, three), "5.500000");
    assertTest(file, "(add 0.1 0.2)", add(makeDouble(0.1), makeDouble(0.2)), "0.300000");
    assertTest(file, "(sub 2 5)", sub(two, five), "-3");
    assertTest(file, "(add 1 \"hi\")", add(makeLong(1), makeString("hi")), "Error: Operand not a number");

    assertTest(file, "(mul 3 4)", mul(three, four), "12");
    assertTest(file, "(mul 2.5 4)", mul(point5, four), "10");

    assertTest(file, "(divide 10 2)", divide(ten, two), "5");
    assertTest(file, "(divide 10 4)", divide(ten, four), "2.500000");
    assertTest(file, "(divide 10 0)", divide(ten, makeLong(0)), "Error: Divide by zero");

    assertTest(file, "(mod 10 3)", mod(ten, three), "1");
    assertTest(file, "(mod 10 5)", mod(ten, five), "0");
    assertTest(file, "(mod 10 0)", mod(ten, makeLong(0)), "Error: Divide by zero");

    fprintf(file, "--- comparison ---\n");
    assertTest(file, "(lt 2 3)", lt(two, three), "t");
    assertTest(file, "(lt 3 2)", lt(three, two), "()");
    assertTest(file, "(lt a b)", lt(makeSymbol("a"), makeSymbol("b")), "Error: Operand not a number");
    assertTest(file, "(lt 2 a)", lt(two, makeSymbol("a")), "Error: Operand not a number");
    assertTest(file, "(gt 5 2)", gt(five, two), "t");
    assertTest(file, "(gt 2 5)", gt(two, five), "()");
    assertTest(file, "(gt a b)", gt(makeSymbol("a"), makeSymbol("b")), "Error: Operand not a number");
    assertTest(file, "(gt 2 b)", gt(two, makeSymbol("b")), "Error: Operand not a number");
    assertTest(file, "(lte 2 2)", lte(two, two), "t");
    assertTest(file, "(lte 3 2)", lte(three, two), "()");
    assertTest(file, "(lte a b)", lte(makeSymbol("a"), makeSymbol("b")), "Error: Operand not a number");
    assertTest(file, "(lte 2 b)", lte(two, makeSymbol("b")), "Error: Operand not a number");
    assertTest(file, "(gte 3 2)", gte(three, two), "t");
    assertTest(file, "(gte 2 3)", gte(two, three), "()");
    assertTest(file, "(gte a b)", gte(makeSymbol("a"), makeSymbol("b")), "Error: Operand not a number");
    assertTest(file, "(gte 2 b)", gte(two, makeSymbol("b")), "Error: Operand not a number");

    fprintf(file, "--- equality ---\n");
    assertTest(file, "(eq 2 2)", eq(two, makeLong(2)), "t");
    assertTest(file, "(eq 2 2.5)", eq(two, point5), "()");
    assertTest(file, "(eq a a)", eq(makeSymbol("a"), makeSymbol("a")), "t");
    assertTest(file, "(eq a b)", eq(makeSymbol("a"), makeSymbol("b")), "()");
    assertTest(file, "(eq \"hi\" \"hi\")", eq(makeString("hi"), makeString("hi")), "t");

    SExp* list1 = cons(makeLong(1), cons(makeLong(2), cons(makeLong(3), &nil)));
    SExp* list2 = cons(makeLong(1), cons(makeLong(2), cons(makeLong(3), &nil)));
    assertTest(file, "(eq (1 2 3) (1 2 3))", eq(list1, list2), "Error: eq called on lists");

    fprintf(file, "--- logical ---\n");
    assertTest(file, "(not ())", notf(&nil), "t");
    assertTest(file, "(not t)", notf(&truth), "()");

    // Sprint 5: set, lookup, arithmetic
    fprintf(file, "=== Sprint 5 Tests ===\n");
    assertTest(file, "(set x 42)", evalString("(set x 42)"), "42");
    assertTest(file, "x", evalString("x"), "42");
    assertTest(file, "(set y \"hello\")", evalString("(set y \"hello\")"), "\"hello\"");
    assertTest(file, "y", evalString("y"), "\"hello\"");
    assertTest(file, "(set x 100)", evalString("(set x 100)"), "100");
    assertTest(file, "x", evalString("x"), "100");
    assertTest(file, "z", evalString("z"), "z");
    assertTest(file, "(set x (add 1 2))", evalString("(set x (add 1 2))"), "3");
    assertTest(file, "(add x 4)", evalString("(add x 4)"), "7");
    assertTest(file, "(set x \"new\")", evalString("(set x \"new\")"), "\"new\"");
    assertTest(file, "x", evalString("x"), "\"new\"");
    assertTest(file, "(set y ())", evalString("(set y ())"), "()");
    assertTest(file, "y", evalString("y"), "()");

    assertTest(file, "(set x (add (mul 2 3) (sub 10 4)))", evalString("(set x (add (mul 2 3) (sub 10 4)))"), "12");
    assertTest(file, "x", evalString("x"), "12");
    assertTest(file, "(set x (cons 1 (cons 2 ())))", evalString("(set x (cons 1 (cons 2 ())))"), "(1 2)");
    assertTest(file, "(car x)", evalString("(car x)"), "1");
    assertTest(file, "(cdr x)", evalString("(cdr x)"), "(2)");
    assertTest(file, "(set x (cons y ()))", evalString("(set x (cons y ()))"), "(())");
    assertTest(file, "(set x '(add 2 3))", evalString("(set x '(add 2 3))"), "(add 2 3)");
    assertTest(file, "x", evalString("x"), "5");

    // Sprint 6: logical short-circuit, if, cond
    fprintf(file, "=== Sprint 6 Tests ===\n");
    fprintf(file, "--- short circuiting functions ---\n");
    assertTest(file, "(and (() 't))", evalString("(and () 't)"), "()");
    assertTest(file, "(and ('t 5))", evalString("(and 't 5)"), "5");
    assertTest(file, "(and ('t 't ()))", evalString("(and 't 't ())"), "()");
    assertTest(file, "(or ('t fail))", evalString("(or 't fail)"), "t");
    assertTest(file, "(or (() 123))", evalString("(or () 123)"), "123");
    assertTest(file, "(or (() ()))", evalString("(or () ())"), "()");
    assertTest(file, "(or (() () 't))", evalString("(or () () 't)"), "t");
    fprintf(file, "--- conditionals ---\n");
    assertTest(file, "(if 't 1 2)", evalString("(if 't 1 2)"), "1");
    assertTest(file, "(if () 1 2)", evalString("(if () 1 2)"), "2");
    assertTest(file, "(if 't yes no)", evalString("(if 't yes no)"), "yes");
    assertTest(file, "(cond ((gt 3 2) \"greater\") ((lt 3 2) \"less\"))", evalString("(cond ((gt 3 2) \"greater\") ((lt 3 2) \"less\"))"), "\"greater\"");
    assertTest(file, "(cond (() \"first\") ('t \"fallback\"))", evalString("(cond (() \"first\") ('t \"fallback\"))"), "\"fallback\"");
    assertTest(file, "(cond (() 1) (() 2))", evalString("(cond (() 1) (() 2))"), "Error: No selected branch");
    assertTest(file, "(cond ((and () skip) 1) ((or 't noskip) 2))", evalString("(cond ((and () skip) 1) ((or 't noskip) 2))"), "2");
    assertTest(file, "(and (if () 't ()) 't)", evalString("(and (if () 't ()) 't)"), "()");
    assertTest(file, "(cond ((and 't ()) none) ((or () 't) matched))", evalString("(cond ((and 't ()) none) ((or () 't) matched))"), "matched");
    assertTest(file, "(cond ('t \"should print\") (() \"should not print\"))", evalString("(cond ('t \"should print\") (() \"should not print\"))"), "\"should print\"");

    fprintf(file, "=== Sprint 7 Tests ===\n");

    fprintf(file, "--- simple function ---\n");
    assertTest(file, "(define square (x) (mul x x))", evalString("(define square (x) (mul x x))"), "square");
    assertTest(file, "(square 5)", evalString("(square 5)"), "25");
    assertTest(file, "(square \"a\")", evalString("(square \"a\")"), "Error: Operand not a number");

    fprintf(file, "--- multiple arguments ---\n");
    assertTest(file, "(define addTwo (a b) (add a b))", evalString("(define addTwo (a b) (add a b))"), "addTwo");
    assertTest(file, "(addTwo 3 4)", evalString("(addTwo 3 4)"), "7");
    assertTest(file, "(addTwo 3)", evalString("(addTwo 3)"), "Error: Argument count mismatch"); 
    assertTest(file, "(addTwo 3 4 5)", evalString("(addTwo 3 4 5)"), "Error: Argument count mismatch");

    fprintf(file, "--- nested calls ---\n");
    assertTest(file, "(define sumSquare (x y) (add (square x) (square y)))", evalString("(define sumSquare (x y) (add (square x) (square y)))"), "sumSquare");
    assertTest(file, "(sumSquare 2 3)", evalString("(sumSquare 2 3)"), "13");

    fprintf(file, "--- factorial function ---\n");
    assertTest(file, "(define fact (n) (if (lte n 1) 1 (mul n (fact (sub n 1)))))", evalString("(define fact (n) (if (lte n 1) 1 (mul n (fact (sub n 1)))))"), "fact");
    assertTest(file, "(fact 5)", evalString("(fact 5)"), "120");
    assertTest(file, "(fact 0)", evalString("(fact 0)"), "1");

    fprintf(file, "=== Sprint 8 Tests: Lambda Functions ===\n");

    fprintf(file, "--- lambda call ---\n");
    assertTest(file, "((lambda (x) (add x 1)) 5)", evalString("((lambda (x) (add x 1)) 5)"), "6");
    assertTest(file, "((lambda () 42))", evalString("((lambda () 42))"), "42");


    fprintf(file, "--- assign to variable ---\n");
    assertTest(file, "(define inc (lambda (x) (add x 1)))", evalString("(define inc (lambda (x) (add x 1)))"), "inc");
    assertTest(file, "(inc 10)", evalString("(inc 10)"), "11");

    fprintf(file, "--- multiple arguments ---\n");
    assertTest(file, "((lambda (a b) (mul a b)) 3 4)", evalString("((lambda (a b) (mul a b)) 3 4)"), "12");
    assertTest(file, "((lambda (x y) (add x y)) 3)", evalString("((lambda (x y) (add x y)) 3)"), "Error: Argument count mismatch");
    assertTest(file, "((lambda (x) (add x y)) 3)", evalString("((lambda (x) (add x y)) 3)"), "Error: Operand not a number"); // y is undefined
    assertTest(file, "((lambda (x) (div x 0)) 5)", evalString("((lambda (x) (div x 0)) 5)"), "Error: Divide by zero"); 

    fprintf(file, "--- nested lambda ---\n");
    assertTest(file, "((lambda (f x) (f x)) (lambda (y) (mul y 2)) 5)", evalString("((lambda (f x) (f x)) (lambda (y) (mul y 2)) 5)"), "10");
    
    fclose(file);
}


// helper to read expression from file/stdin, handles multi-line input
char* readExpression(FILE *in) {
    static char buffer[1024];
    buffer[0] = '\0';

    int parens = 0;
    char line[256];
    while (fgets(line, sizeof(line), in)) {
        stripComment(line);
        strncat (buffer, line, sizeof(buffer) - strlen(buffer) - 1);

        // count parentheses
        for (int i = 0; line[i] != '\0'; i++) {
            if (line[i] == '(') parens++;
            else if (line[i] == ')') parens--;
        }

        if (parens <= 0 && buffer[0] != '\0') {
            return buffer;
        }
    }
    return NULL; // EOF
}

// read file and eval each expression
void readFile(const char* filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        return;
    }

    if (!globalEnv) {
        globalEnv = malloc(sizeof(Env));
        globalEnv->symbols = &nil;
        globalEnv->values = &nil;
        globalEnv->parent = NULL;
    }

    char* expr;
    while ((expr = readExpression(file)) != NULL) {
        SExp* sexpInput = sexp(expr);
        SExp* result = eval(sexpInput, globalEnv);
        printf("%s\n", sexpToString(result));
    }
    fclose(file);
}

// read from stdin and eval each expression
void repl() {
    printf("Type 'exit' to quit.\n");

    if (!globalEnv) {
        globalEnv = malloc(sizeof(Env));
        globalEnv->symbols = &nil;
        globalEnv->values = &nil;
        globalEnv->parent = NULL;
    }

    char* expr;
    while (1) {
        printf(">"); // main prompt

        expr = readExpression(stdin); // read full expression, even multi-line
        if (!expr) break; // EOF

        // check for exit command
        if (strcmp(expr, "exit\n") == 0) break;

        // parse and evaluate
        SExp* sexpInput = sexp(expr);
        SExp* result = eval(sexpInput, globalEnv);

        // print result
        printf("%s\n", sexpToString(result));
    }
}



int main(int argc, char* argv[]){
    if (argc == 2) { // file input or test mode
        if (strcmp(argv[1], "-test") == 0) {
            runTests("test_results.txt");
        }
        else {
            readFile(argv[1]);
        }
    }
    else {          // standard input
        repl();
    }
    return 0;
}