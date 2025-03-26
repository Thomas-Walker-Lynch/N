#include <stdio.h>

/*===========================================================================
  Force extra macro expansion (the EVAL trick)
  This chain of EVAL macros forces the preprocessor to perform many rescans,
  which is necessary to “unroll” recursive macros.
===========================================================================*/
#define EVAL(...)            EVAL8(__VA_ARGS__)
#define EVAL16(...)          EVAL8(EVAL8(__VA_ARGS__))
#define EVAL8(...)           EVAL4(EVAL4(__VA_ARGS__))
#define EVAL4(...)           EVAL2(EVAL2(__VA_ARGS__))
#define EVAL2(...)           EVAL1(EVAL1(__VA_ARGS__))
#define EVAL1(...)           __VA_ARGS__

/*===========================================================================
  Defer macros: these help “hide” recursive calls for additional expansion passes.
===========================================================================*/
#define EMPTY()
#define DEFER1(m)            m EMPTY()
#define DEFER2(m)            m EMPTY EMPTY()()
#define DEFER3(m)            m EMPTY EMPTY EMPTY()()()
#define DEFER4(m)            m EMPTY EMPTY EMPTY EMPTY()()()  /* as needed */

#define a(x) x + x
#define b 7 
#define cat(x,y) EVAL(DEFER1(a)(x) * y)

int main(void) {
    int result = cat(5, 7);
    printf("cat(5, 7) = %d\n", result);
    return 0;
}
