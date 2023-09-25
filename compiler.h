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
#include "constant folding.h"

// The plan is to honor as many C operators as possible with
// the same precedence and associativity
// e<n> implements operators with precedence 'n' (smaller is higher)

void expression(Compiler* compiler, bool effects);

// () [] . -> ...
void e1(Compiler* compiler, bool effects) {
    optionalSlice id = consumeIdentifier(compiler);
    if (id.exists) {
        if (consume(compiler, "(")) {

            // this is a function call
            uint64_t numParams = 0;
            while (!consume(compiler, ")")) {
                expression(compiler, effects);
                consume(compiler, ",");
                numParams++;
            }
            printf("    call ._.");
            printSlice(id.item);
            printf("\n");
            for (size_t i = 0; i < numParams; i++) {
                puts("    pop %r15");            // pop the parameters that were just pushed onto the stack
            }
            puts("    push %rax");
        }
        else {
            // get the correct value from its offset stored in the map
            int64_t offset = mapGet(compiler -> symbolTable, id.item);
            printf("    push %ld(%%rbp)\n", offset);
        }

        return;
    }
        
    optionalInt val = consumeLiteral(compiler);
    if (val.exists) {
        printf("    mov $%lu, %%rdi\n", val.item);
        puts("    push %rdi");
        return;
    }

    if (consume(compiler, "(")) {
        expression(compiler, effects);
        consume(compiler, ")");
        return;
    }

    fail(compiler);
}

// ++ -- unary+ unary- ... (Right)
void e2(Compiler* compiler, bool effects) {
    bool neg = false;
    bool encounteredNeg = false;

    // logical not
    while (true) {
        if (consume(compiler, "!")) {
            encounteredNeg = true;
            neg = !neg;
        }
        else {
            e1(compiler, effects);

            if (neg) {
                puts("    pop %rdi");
                puts("    cmp $0, %rdi");
                puts("    mov $0, %edi");
                puts("    sete %dil");
                puts("    push %rdi");
            }
            else if (encounteredNeg) {
                puts("    pop %rdi");
                puts("    cmp $0, %rdi");
                puts("    mov $0, %edi");
                puts("    setne %dil");
                puts("    push %rdi");
            }

            return;
        }
    }
}

// * / % (Left)
void e3(Compiler* compiler, bool effects) {
    e2(compiler, effects);

    while (true) {
        if (consume(compiler, "*")) {
            e2(compiler, effects);
            puts("    pop %rsi");
            puts("    pop %rdi");
            puts("    imul %rsi, %rdi");
            puts("    push %rdi");
        }
        else if (consume(compiler, "/")) {
            e2(compiler, effects);
            puts("    pop %rsi");
            puts("    pop %rax");
            puts("    xor %edx, %edx");
            puts("    div %rsi");
            puts("    push %rax");
        }
        else if (consume(compiler, "%")) {
            e2(compiler, effects);
            puts("    pop %rsi");
            puts("    pop %rax");
            puts("    xor %edx, %edx");
            puts("    div %rsi");
            puts("    push %rdx");
        }
        else {
            return;
        }
    }
}

// (Left) + -
void e4(Compiler* compiler, bool effects) {
    e3(compiler, effects);

    while (true) {
        if (consume(compiler, "+")) {
            e3(compiler, effects);
            puts("    pop %rsi");
            puts("    pop %rdi");
            puts("    add %rsi, %rdi");
            puts("    push %rdi");
        }
        else if (consume(compiler, "-")) {
            e3(compiler, effects);
            puts("    pop %rsi");
            puts("    pop %rdi");
            puts("    sub %rsi, %rdi");
            puts("    push %rdi");
        }
        else {
            return;
        }
    }
}

// << >>
void e5(Compiler* compiler, bool effects) {
    e4(compiler, effects);
}

// < <= > >=
void e6(Compiler* compiler, bool effects) {
    e5(compiler, effects);

    while (true) {
        if (consume(compiler, "<=")) {
            e5(compiler, effects);
            // v = (v <= u) ? 1 : 0;
            puts("    pop %rsi");
            puts("    pop %rdi");
            puts("    cmp %rsi, %rdi");
            puts("    mov $0, %edi");
            puts("    setbe %dil");
            puts("    push %rdi");
        }
        else if (consume(compiler, ">=")) {
            e5(compiler, effects);
            // v = (v >= u) ? 1 : 0;
            puts("    pop %rsi");
            puts("    pop %rdi");
            puts("    cmp %rsi, %rdi");
            puts("    mov $0, %edi");
            puts("    setae %dil");
            puts("    push %rdi");
        }
        else if (consume(compiler, "<")) {
            e5(compiler, effects);
            // v = (v < u) ? 1 : 0;
            puts("    pop %rsi");
            puts("    pop %rdi");
            puts("    cmp %rsi, %rdi");
            puts("    mov $0, %edi");
            puts("    setb %dil");
            puts("    push %rdi");
        }
        else if (consume(compiler, ">")) {
            e5(compiler, effects);
            // v = (v > u) ? 1 : 0;
            puts("    pop %rsi");
            puts("    pop %rdi");
            puts("    cmp %rsi, %rdi");
            puts("    mov $0, %edi");
            puts("    seta %dil");
            puts("    push %rdi");
        }
        else {
            return;
        }
    }
}

// == !=
void e7(Compiler* compiler, bool effects) {
    e6(compiler, effects);

    while (true) {
        if (consume(compiler, "==")) {
            e6(compiler, effects);
            // v = (v == u) ? 1 : 0;
            puts("    pop %rsi");
            puts("    pop %rdi");
            puts("    cmp %rsi, %rdi");
            puts("    mov $0, %edi");
            puts("    sete %dil");
            puts("    push %rdi");
        }
        else if (consume(compiler, "!=")) {
            e6(compiler, effects);
            // v = (v != u) ? 1 : 0;
            puts("    pop %rsi");
            puts("    pop %rdi");
            puts("    cmp %rsi, %rdi");
            puts("    mov $0, %edi");
            puts("    setne %dil");
            puts("    push %rdi");
        }
        else {
            return;
        }
    }
}

// (left) &
void e8(Compiler* compiler, bool effects) {
    e7(compiler, effects);
}

// ^
void e9(Compiler* compiler, bool effects) {
    e8(compiler, effects);
}

// |
void e10(Compiler* compiler, bool effects) {
    e9(compiler, effects);
}

// &&
void e11(Compiler* compiler, bool effects) {
    e10(compiler, effects);

    while (true) {
        if (consume(compiler, "&&")) {
            e10(compiler, effects);
            puts("    pop %rsi");
            puts("    pop %rdi");
            puts("    test %rsi, %rsi");
            puts("    setnz %sil");
            puts("    test %rdi, %rdi");
            puts("    setnz %dil");
            puts("    and %rsi, %rdi");
            puts("    and $1, %rdi");
            puts("    push %rdi");
        }
        else {
            return;
        }
    }
}

// ||
void e12(Compiler* compiler, bool effects) {
    e11(compiler, effects);
    
    while (true) {
        if (consume(compiler, "||")) {
            e11(compiler, effects);
            puts("    pop %rsi");
            puts("    pop %rdi");
            puts("    test %rsi, %rsi");
            puts("    setnz %sil");
            puts("    test %rdi, %rdi");
            puts("    setnz %dil");
            puts("    or %rsi, %rdi");
            puts("    and $1, %rdi");
            puts("    push %rdi");
        }
        else {
            return;
        }
    }
}

// (right with special treatment for middle expression) ?:
void e13(Compiler* compiler, bool effects) {
    e12(compiler, effects);
}

// = += -= ...
void e14(Compiler* compiler, bool effects) {
    e13(compiler, effects);
}

// ,
void e15(Compiler* compiler, bool effects) {
    e14(compiler, effects);
}

void expression(Compiler* compiler, bool effects) {
    optionalInt ret = checkExpression(compiler, effects);
    if (ret.exists) {
        // is a literal expression, just push expression found through constant folding
        printf("    mov $%lu, %%rdi\n", ret.item);
        puts("    push %rdi");
    }
    else {
        e15(compiler, effects);
    }
}

bool statement(Compiler* compiler, bool effects, optionalSlice currentFunction) {
    // printf("START\n%s\nEND\n\n", (compiler -> current));

    optionalSlice id = consumeIdentifier(compiler);

    if (!id.exists) {
        return false;
    }

    if (sliceEqualString(id.item, "print")) {
        consume(compiler, "(");
        expression(compiler, effects);
        consume(compiler, ")");
        puts("    call ._.print");
        puts("    pop %r15");
        return true;
    }

    if (sliceEqualString(id.item, "return")) {
        char* beforePointer = compiler -> current;
        optionalSlice id = consumeIdentifier(compiler);

        // Note: tail recursion optimization only works on calls of the form return function(...)
        if (currentFunction.exists && id.exists && sliceEqualSlice(id.item, currentFunction.item)) {

            uint64_t numParams = 1;
            char* beforePointer2 = compiler -> current;

            bool canUseTailRecursion = true;
            
            if (!consume(compiler, "(")) {
                canUseTailRecursion = false;
            }

            // find the number of parameters in this function to malloc the parameter array
            while (compiler -> current[0] != '\n') {
                if (compiler -> current[0] == ')') {
                    compiler -> current++;
                    while (compiler -> current[0] != '\n') {
                        if (compiler -> current[0] != ' ') {
                            canUseTailRecursion = false;
                        }
                        compiler -> current++;
                    }
                    break;
                }
                
                if (compiler -> current[0] == ',') {
                    numParams++;
                }
                compiler -> current++;
            }

            if (canUseTailRecursion) {

                compiler -> current = beforePointer2;

                int64_t offset = (numParams * 8) + 8;

                // consume parameters
                while (!consume(compiler, ")")) {
                    expression(compiler, effects);
                    puts("    pop %rdi");
                    printf("    mov %%rdi, %lu(%%rbp)\n", offset);
                    offset -= 8;
                    consume(compiler, ",");
                }

                puts("    mov %rbp, %rsp");
                puts("    pop %rbp");
                printf("    jmp ._.");
                printSlice(id.item);
                printf("\n");
                return true;
            }
        }

        compiler -> current = beforePointer;
        expression(compiler, effects);
        puts("    pop %rax");
        puts("    mov %rbp, %rsp");
        puts("    pop %rbp");
        puts("    ret");

        return true;
    }

    if (sliceEqualString(id.item, "if")) {
        // if ... 
        consumeOrFail(compiler, "(");
        expression(compiler, effects);
        consumeOrFail(compiler, ")");

        compiler -> countIf++;
        uint64_t currentIfCounter = compiler -> countIf;
        puts("    pop %rdi");
        puts("    test %rdi, %rdi");

        // jumps to label if not true (skip over if statement)
        printf("    jz ._.skipIf%lu\n", currentIfCounter);

        // go through the if statement
        uint64_t countBrackets = 1;
        consumeOrFail(compiler, "{");

        while (countBrackets > 0) {
            if (consume(compiler, "{")) {
                countBrackets++;
                continue;
            }
            if (consume(compiler, "}")) {
                countBrackets--;
                continue;
            }

            statement(compiler, effects, currentFunction);
        }


        printf("    jmp ._.endIf%lu\n", currentIfCounter);
        printf("._.skipIf%lu:\n", currentIfCounter);

        // check if there is an else statement
        char* prevPointer = compiler -> current;
        optionalSlice checkElse = consumeIdentifier(compiler);
        if (checkElse.exists && sliceEqualString(checkElse.item, "else")) {
            countBrackets = 1;
            consumeOrFail(compiler, "{");

            while (!consume(compiler, "}")) {
                if (consume(compiler, "{")) {
                    countBrackets++;
                    continue;
                }
                if (consume(compiler, "}")) {
                    countBrackets--;
                    continue;
                }

                statement(compiler, effects, currentFunction);
            }
        }
        else {
            compiler -> current = prevPointer;
        }

        printf("._.endIf%lu:\n", currentIfCounter);

        return true;
    }

    if (sliceEqualString(id.item, "while")) {
        // while ... 

        compiler -> countWhile++;
        uint64_t currentWhileCounter = compiler -> countWhile;

        printf("._.startWhile%lu:\n", currentWhileCounter);

        consumeOrFail(compiler, "(");
        expression(compiler, effects);
        consumeOrFail(compiler, ")");

        puts("    pop %rdi");
        puts("    test %rdi, %rdi");

        // jumps to label if not true (skip over while statement)
        printf("    jz ._.skipWhile%lu\n", currentWhileCounter);

        // go through the while statement
        consumeOrFail(compiler, "{");
        uint64_t countBrackets = 1;

        while (countBrackets > 0) {
            if (consume(compiler, "{")) {
                countBrackets++;
                continue;
            }
            if (consume(compiler, "}")) {
                countBrackets--;
                continue;
            }

            statement(compiler, effects, currentFunction);
        }

        printf("    jmp ._.startWhile%lu\n", currentWhileCounter);
        printf("._.skipWhile%lu:\n", currentWhileCounter);

        return true;
    }

    if (sliceEqualString(id.item, "else")) {
        // error, cannot have else without a preceding if statement
        fail(compiler);
    }

    if (sliceEqualString(id.item, "fun")) {
        // fun ... 
        optionalSlice functionName = consumeIdentifier(compiler);
        if (!functionName.exists) {
            fail(compiler);
        }
        compiler -> symbolTable = mapCreate();

        int64_t numParams = 0;

        consume(compiler, "(");
        char* beforePointer = compiler -> current;

        // find the number of parameters in this function to malloc the parameter array
        while (!consume(compiler, ")")) {
            optionalSlice parameterName = consumeIdentifier(compiler);
            if (!parameterName.exists) {
                fail(compiler);
            }
            numParams++;
            consume(compiler, ",");
        }

        compiler -> current = beforePointer;

        int64_t offset = (numParams * 8) + 8;

        // consume parameters
        while (!consume(compiler, ")")) {
            optionalSlice parameterName = consumeIdentifier(compiler);
            if (!parameterName.exists) {
                fail(compiler);
            }
            mapInsert(compiler -> symbolTable, parameterName.item, offset);
            offset -= 8;
            consume(compiler, ",");
        }

        consumeOrFail(compiler, "{");
        uint64_t countBrackets = 1;

        beforePointer = compiler -> current;
        
        // points to the last place where there was a newline character
        char* newlinePointer = compiler -> current;
        offset = -8;

        while (countBrackets > 0) {
            if (compiler -> current[0] == '}') {
                countBrackets--;
                compiler -> current++;
                continue;
            }
            if (compiler -> current[0] == '{') {
                countBrackets++;
                compiler -> current++;
                continue;
            }
            if (compiler -> current[0] == '\n') {
                newlinePointer = compiler -> current;
                compiler -> current++;
                continue;
            }

            // parse until you hit an equals, since every variable declaration has to be in the form:
                // variable = ... 
                // and every variable declaration is on a new line
            if (compiler->current[0] != '=') {
                compiler -> current++;
                continue;
            }

            char* currentPointer = compiler -> current + 1;

            compiler -> current = newlinePointer;

            optionalSlice parameterName = consumeIdentifier(compiler);
            if (!parameterName.exists) {
                fail(compiler);
            }
            
            if (!mapContains(compiler -> symbolTable, parameterName.item)) {
                mapInsert(compiler -> symbolTable, parameterName.item, offset);
                offset -= 8;
            }
            
            compiler -> current = currentPointer;
        }

        // parse through second time, this time actually writing the assembly code 
        compiler -> current = beforePointer;

        printf("._.");
        printSlice(functionName.item);
        printf(":\n");
        puts("    push %rbp");
        puts("    mov %rsp, %rbp");
        printf("    sub $%ld, %%rsp\n", -1*(offset+8));

        countBrackets = 1;
        while (countBrackets > 0) {
            if (consume(compiler, "}")) {
                countBrackets--;
                continue;
            }
            if (consume(compiler, "{")) {
                countBrackets++;
                continue;
            }
            statement(compiler, effects, functionName);
        }

        puts("    mov %rbp, %rsp");
        puts("    pop %rbp");
        puts("    xor %eax, %eax");         // default return value is 0
        puts("    ret");

        freeMap(compiler -> symbolTable);

        return true;
    }

    if (consume(compiler, "=")) {
        expression(compiler, effects);

        puts("    pop %rdi");
        printf("    mov %%rdi, %ld(%%rbp)\n", mapGet(compiler -> symbolTable, id.item));

        return true;
    }
    else {
        // can have a stand-alone function call without doing (var) = (function call)
        if (consume(compiler, "(")) {

            // this is a function call
            uint64_t numParams = 0;
            while (!consume(compiler, ")")) {
                expression(compiler, effects);
                consume(compiler, ",");
                numParams++;
            }
            printf("    call ._.");
            printSlice(id.item);
            printf("\n");
            for (size_t i = 0; i < numParams; i++) {
                puts("    pop %r15");            // pop the parameters that were just pushed onto the stack
            }
        }

        return true;
    }

    return false;
}

void statements(Compiler* compiler, bool effects) {
    optionalSlice slice = { false, sliceConstructorLen(0, 0) };
    while (statement(compiler, effects, slice));
}

void run(Compiler* compiler) {
    statements(compiler, true);
    endOrFail(compiler);
}

Compiler* compilerConstructor(char* prog) {
    Compiler* compiler = (Compiler*) (malloc(sizeof(Compiler)));
    compiler -> program = prog;
    compiler -> current = prog;
    compiler -> countIf = 0;
    compiler -> countWhile = 0;
    
    return compiler;
}
