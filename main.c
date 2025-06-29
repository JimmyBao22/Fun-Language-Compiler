#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>

#include "compiler.h"

int main() {

    // reads the fun program from stdin
    uint64_t inputLen = 10000;
    char* progOrig = (char*)(malloc(sizeof(char) * inputLen));
    char c;
    uint64_t index = 0;
    while ((c = getchar()) != EOF) {
        progOrig[index++] = c;
    }

    // preprocess to get rid of comments
    index = 0;
    char* prog = (char*)(malloc(sizeof(char) * inputLen));

    for (size_t i = 0; i < inputLen; i++) {
        if (progOrig[i] == '#') {
            // this line is a comment, skip it
            while (progOrig[i] != '\n' && i < inputLen) {
                i++;
            }
            i--;
        }
        else {
            prog[index++] = progOrig[i];
        }
    }

    // printf("%s\n", prog);

    puts("    .data");
    puts("format: .byte '%', 'l', 'u', 10, 0");
    puts("    .text");
    puts("    .global main");
    puts("    .extern printf");
    
    puts("main:");
    puts("    push %r12");
    puts("    push %r13");
    puts("    push %r14");
    puts("    push %r15");
    puts("    push %rbp");
    puts("    push %rbx");
    puts("    call ._.main");
    puts("    pop %r12");
    puts("    pop %r13");
    puts("    pop %r14");
    puts("    pop %r15");
    puts("    pop %rbp");
    puts("    pop %rbx");
    puts("    ret");

    puts("._.print:");
    puts("    push %rbp");
    puts("    mov %rsp, %rbp");
    puts("    and $0xFFFFFFFFFFFFFFF0, %rsp");
    puts("    xor %eax, %eax");
    puts("    mov 16(%rbp), %rsi");              // maybe change later
    puts("    lea format(%rip), %rdi");
    puts("    call printf");
    puts("    mov %rbp, %rsp");
    puts("    xor %eax, %eax");
    puts("    pop %rbp");
    puts("    ret");

    Compiler* compiler = compilerConstructor(prog);
    
    run(compiler);

    // deallocate space to reduce memory leaks
    // free(compiler);
    // free(prog);

    return 0;
}
