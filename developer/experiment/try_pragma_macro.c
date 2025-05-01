// experiment/test_pragma_macro.c

#include <stdio.h>

// -- STEP 1: Should compile even if pragma is ignored
#pragma message("Compiling test_pragma_macro.c")

#define A AMACRO

#pragma macro(A)

// Uncomment to test extension behavior:
// Expected behavior: defines MY_MACRO to have body "42"
// Expected fallback: compiler error or ignored pragma
// #pragma name(MY_MACRO, 42)

#ifndef MY_MACRO
#define MY_MACRO -1
#endif

int main(void) {
    printf("MY_MACRO expands to: %d\n", MY_MACRO);

    #ifdef AMACRO
      printf("AMACRO DEFINED!");
    #endif
    return 0;
}

/*
nvoking a non-existing pragma is ignored without the -Wall

2025-05-01T13:29:48Z[developer]
Thomas-developer@StanleyPark§/home/Thomas/subu_data/developer/N/developer/experiment§
> gcc -Wall try_pragma_macro.c 
try_pragma_macro.c:6:9: note: ‘#pragma message: Compiling test_pragma_macro.c’
    6 | #pragma message("Compiling test_pragma_macro.c")
      |         ^~~~~~~
try_pragma_macro.c:11: warning: ignoring ‘#pragma macro ’ [-Wunknown-pragmas]
   11 | #pragma macro(A)
      | 

*/
