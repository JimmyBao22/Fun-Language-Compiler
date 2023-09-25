#pragma once

// libc includes (available in both C and C++)
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// Implementation includes
#include "mapc.h"

// optional -> allows one to check if a slice/int was returned/exists
#define optional(type) struct { bool exists; type item; }

typedef optional(Slice) optionalSlice;
typedef optional(uint64_t) optionalInt;

typedef struct Compiler {
    char* program;
    char* current;
    uint64_t countIf;
    uint64_t countWhile;
    UnorderedMap* symbolTable;          // maps variables to offsets
} Compiler;

void fail(Compiler* compiler) {
    printf("failed at offset %ld\n", (size_t)(compiler -> current - compiler -> program));
    printf("%s\n", compiler -> current);
    exit(1);
}

void endOrFail(Compiler* compiler) {
    while (isspace(*(compiler -> current))) {
        compiler -> current++;
    }
    if (*(compiler -> current) != 0) {
        fail(compiler);
    }
}

// skips past all white space
void skip(Compiler* compiler) {
    while (isspace(*(compiler -> current))) {
        compiler -> current++;
    }
}

// consumes to check if the current line matches str
bool consume(Compiler* compiler, char const *str) {
    skip(compiler);

    size_t i = 0;
    while (true) {
      char const expected = str[i];
      char const found = compiler -> current[i];
      if (expected == 0) {
        // survived to the end of the expected string 
        compiler -> current += i;
        return true;
      }
      if (expected != found) {
        return false;
      }
      // assertion: found != 0
      i++;
    }
}

void consumeOrFail(Compiler* compiler, char const *str) {
    if (!consume(compiler, str)) {
        fail(compiler);
    }
}

// consume a variable 
optionalSlice consumeIdentifier(Compiler* compiler) {
    skip(compiler);

    if (isalpha(*(compiler -> current))) {
        char const *start = compiler -> current;

        do {
            compiler -> current++;
        } while(isalnum(*(compiler -> current)));

        optionalSlice slice = { true, sliceConstructorLen(start, (size_t)(compiler -> current - start)) };
        return slice;
    }
    else {
        optionalSlice slice = { false, sliceConstructorLen(0, 0) };
        return slice;
    }
}

// consume a number
optionalInt consumeLiteral(Compiler* compiler) {
    skip(compiler);

    if (isdigit(*(compiler -> current))) {
        uint64_t v = 0;

        do {
            v = 10 * v + ((*(compiler -> current)) - '0');
            compiler -> current++;
        } while (isdigit(*(compiler -> current)));

        optionalInt opInt = { true, v };
        return opInt;
    }
    else {
        optionalInt opInt = { false, 0 };
        return opInt;
    }
}

// consume past a loop/if statement/function declaration if we want to skip it
void consumePast(Compiler* compiler) {
    int count = 1;
    while (count > 0) {
        if (consume(compiler, "{")) {
            count++;
        }
        else if (consume(compiler, "}")) {
            count--;
            if (count == 0) {
                break;
            }
        }
        else {
            compiler -> current++;
        }
    }
}

// The plan is to honor as many C operators as possible with
// the same precedence and associativity
// e<n> implements operators with precedence 'n' (smaller is higher)

uint64_t expressionCF(Compiler* compiler, bool effects);

// () [] . -> ...
uint64_t e1CF(Compiler* compiler, bool effects) {
    optionalInt val = consumeLiteral(compiler);
    if (val.exists) {
        return val.item;
    }

    if (consume(compiler, "(")) {
        uint64_t v = expressionCF(compiler, effects);
        consume(compiler, ")");
        return v;
    }

    fail(compiler);
    return 0;
}

// ++ -- unary+ unary- ... (Right)
uint64_t e2CF(Compiler* compiler, bool effects) {
    bool neg = false;
    bool encounteredNeg = false;

    // logical not
    while (true) {
        if (consume(compiler, "!")) {
            encounteredNeg = true;
            neg = !neg;
        }
        else {
            uint64_t v = e1CF(compiler, effects);
            if (neg) {
                v = v > 0 ? 0 : 1;
            }
            if (encounteredNeg && v > 1) {
                v = 1;
            }
            return v;
        }
    }
}

// * / % (Left)
uint64_t e3CF(Compiler* compiler, bool effects) {
    uint64_t v = e2CF(compiler, effects);

    while (true) {
        if (consume(compiler, "*")) {
            v = v * e2CF(compiler, effects);
        }
        else if (consume(compiler, "/")) {
            uint64_t right = e2CF(compiler, effects);
            v = (right == 0) ? 0 : v / right;
        }
        else if (consume(compiler, "%")) {
            uint64_t right = e2CF(compiler, effects);
            v = (right == 0) ? 0 : v % right;
        }
        else {
            return v;
        }
    }
}

// (Left) + -
uint64_t e4CF(Compiler* compiler, bool effects) {
    uint64_t v = e3CF(compiler, effects);

    while (true) {
        if (consume(compiler, "+")) {
            v = v + e3CF(compiler, effects);
        }
        else if (consume(compiler, "-")) {
            v = v - e3CF(compiler, effects);
        }
        else {
            return v;
        }
    }
}

// << >>
uint64_t e5CF(Compiler* compiler, bool effects) {
    return e4CF(compiler, effects);
}

// < <= > >=
uint64_t e6CF(Compiler* compiler, bool effects) {
    uint64_t v = e5CF(compiler, effects);

    while (true) {
        if (consume(compiler, "<=")) {
            uint64_t u = e5CF(compiler, effects);
            v = (v <= u) ? 1 : 0;
        }
        else if (consume(compiler, ">=")) {
            uint64_t u = e5CF(compiler, effects);
            v = (v >= u) ? 1 : 0;
        }
        else if (consume(compiler, "<")) {
            uint64_t u = e5CF(compiler, effects);
            v = (v < u) ? 1 : 0;
        }
        else if (consume(compiler, ">")) {
            uint64_t u = e5CF(compiler, effects);
            v = (v > u) ? 1 : 0;
        }
        else {
            return v;
        }
    }
}

// == !=
uint64_t e7CF(Compiler* compiler, bool effects) {
    uint64_t v = e6CF(compiler, effects);

    while (true) {
        if (consume(compiler, "==")) {
            uint64_t u = e6CF(compiler, effects);
            v = (v == u) ? 1 : 0;
        }
        else if (consume(compiler, "!=")) {
            uint64_t u = e6CF(compiler, effects);
            v = (v != u) ? 1 : 0;
        }
        else {
            return v;
        }
    }
}

// (left) &
uint64_t e8CF(Compiler* compiler, bool effects) {
    return e7CF(compiler, effects);
}

// ^
uint64_t e9CF(Compiler* compiler, bool effects) {
    return e8CF(compiler, effects);
}

// |
uint64_t e10CF(Compiler* compiler, bool effects) {
    return e9CF(compiler, effects);
}

// &&
uint64_t e11CF(Compiler* compiler, bool effects) {
    uint64_t v = e10CF(compiler, effects);

    while (true) {
        if (consume(compiler, "&&")) {
            v = e10CF(compiler, effects) && v;
        }
        else {
            return v;
        }
    }
}

// ||
uint64_t e12CF(Compiler* compiler, bool effects) {
    uint64_t v = e11CF(compiler, effects);
    
    while (true) {
        if (consume(compiler, "||")) {
            v = e11CF(compiler, effects) || v;
        }
        else {
            return v;
        }
    }
}

// (right with special treatment for middle expression) ?:
uint64_t e13CF(Compiler* compiler, bool effects) {
    return e12CF(compiler, effects);
}

// = += -= ...
uint64_t e14CF(Compiler* compiler, bool effects) {
    return e13CF(compiler, effects);
}

// ,
uint64_t e15CF(Compiler* compiler, bool effects) {
    return e14CF(compiler, effects);
}

uint64_t expressionCF(Compiler* compiler, bool effects) {
    return e15CF(compiler, effects);
}

// checks if constant folding is possible
optionalInt checkExpression(Compiler* compiler, bool effects) {
    char* beforePointer = compiler -> current;
    while (compiler -> current[0] != '\n') {
        if (isalpha(*(compiler -> current))) {
            optionalInt cur = { false, 0 };
            compiler -> current = beforePointer;
            return cur;
        }
        compiler -> current++;
    }

    // this is a numeric expression, so just do constant folding and return value of expression
    compiler -> current = beforePointer;
    uint64_t ret = expressionCF(compiler, effects);
    optionalInt cur = { true, ret };
    return cur;
}
