// try_macro_cat.c
#include <stdio.h>
#include "macro_cat.c"

// Define test macros
#define A Hello
#define B World
#define C 123
#define D _XYZ
#define EMPTY

// Define some variables to test concatenation into identifiers
int HelloWorld = 10;
int Hello123 = 20;
int Hello_XYZ = 30;
int World = 40;

int main(void) {

    // CAT(A,B) → HelloWorld
    printf("CAT(A,B): %d\n", CAT(A,B));

    // CAT(A,C) → Hello123
    printf("CAT(A,C): %d\n", CAT(A,C));

    // CAT(A,D) → Hello_XYZ
    printf("CAT(A,D): %d\n", CAT(A,D));

    // CAT(EMPTY,B) → World
    printf("CAT(EMPTY,B): %d\n", CAT(EMPTY,B));

    // CAT(A,EMPTY) → Hello (must define int Hello)
    // Uncomment after defining: int Hello = 50;
    // printf("CAT(A,EMPTY): %d\n", CAT(A,EMPTY));

    // Nested concatenation: CAT(CAT(A,EMPTY),B) → HelloWorld
    // Uncomment after defining int Hello = ...;
    // printf("CAT(CAT(A,EMPTY),B): %d\n", CAT(CAT(A,EMPTY),B));

    return 0;
}
