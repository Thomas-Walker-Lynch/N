#include <stdio.h>
#define STR(x) #x
// no evaluation, and one pass of evaluation
#define SHOW(expr) printf("%s --> %s\n", #expr, STR(expr))

#define BE(...) __VA_ARGS__
#define EMPTY()

#include <stdio.h>
int main(void){
  printf("example_grow.c\n");
  printf("\n");

  // case 1
  /* 
    GROW(7) → 17 * GROW(7)
    GROW(GROW(5)) → 17 * GROW(17 * GROW(5))
  */
  #define GROW(x) 17 * GROW(x)  
  SHOW(GROW(7));
  SHOW(GROW(GROW(5)));
  printf("\n");

  // case 2
  // GROW2(11) --> 19 * GROW2 (11)
  #define GROW2(x) 19 * GROW2 EMPTY() (x)  
  SHOW(GROW2(11)) ;
  printf("\n");

  // case 3
  // GROW3(13) --> 119 * 19 * GROW2 (13)
  #define GROW3(x) BE(119 * GROW2 EMPTY() (x))
  SHOW(GROW3(13));
  printf("\n");
    
  // case 4
  /* 
    `BE` placed on the outside. The idea is that evaluation will return `123 * GROW (15)` and then this will be evaluated resulting in `123 * 123 * GROW(15)`.

    However, any time GROW4 literally spells out GROW4(...) inside its own expansion
    it will leave it literally and not expand it. 

    Though it is interesting that here that occurs at a higher level in the evaluation tree than the first ocurrance of the recursive call, but still it is in the tree.

    GROW4(15) --> 123 * GROW4 (15)
  */
  #define GROW4(x) BE(123 * GROW4 EMPTY() (x))
  SHOW(GROW4(15));
  printf("\n");

  // case 5
  /*
    Substitution of the function named followed by a trampoline works.

    The original expression is written in terms of a deferred CONDFEDERATE function
    instead of in terms of a recursive call to GROW5. cpp can suspect nothing.

    The result is an expression in terms of the unevaluated CONDFEDERATE function.

    The CONFEDERATE function is defined to return the token GROW5. Hence in a subsequent evaluation, and there must be a subsequent evaluation for this to work,  The CONFEDERATE function will run, return GROW5 which is next to its call parenthesis, so then the
    recursive call will run.

    BE(GROW5(21)) --> 541 * 541 * CONFEDERATE () (21)
    BE(BE(GROW5(57))) --> 541 * 541 * 541 * CONFEDERATE () (57)
  */
  #define GROW5(x) 541 * CONFEDERATE EMPTY() () (x)
  #define CONFEDERATE() GROW5
  SHOW(BE(GROW5(21)));
  SHOW(BE(BE(GROW5(57))));
  printf("\n");

  // case 6
  /*
    Once a recursively called function is marked, or 'colored' it can never be expanded, even when passed through to another variable and separately evaluation.
   */
  #define GROW6(x) 1029 * GROW6 EMPTY() (x)
  #define RESULT1 GROW6(51)
  SHOW(RESULT1);
  SHOW(BE(RESULT1));
  printf("\n");
  // RESULT1 --> 1029 * GROW6 (51)
  // BE(RESULT1) --> 1029 * GROW6 (51)

  // case 7
  #define RESULT2 GROW6(151)
  SHOW(RESULT2);
  SHOW(BE(RESULT2));
  printf("\n");
  // RESULT2 --> 1029 * GROW6 (151)
  // BE(RESULT2) --> 1029 * GROW6 (151)

}
/*
  2025-03-27T10:45:39Z[developer]
Thomas-developer@Stanley§/home/Thomas-masu/developer/N/developer/example§
> gcc example_grow.c

2025-03-27T11:43:19Z[developer]
Thomas-developer@Stanley§/home/Thomas-masu/developer/N/developer/example§
> ./a.out 
example_grow.c

GROW(7) --> 17 * GROW(7)
GROW(GROW(5)) --> 17 * GROW(17 * GROW(5))

GROW2(11) --> 19 * GROW2 (11)

GROW3(13) --> 119 * 19 * GROW2 (13)

GROW4(15) --> 123 * GROW4 (15)

BE(GROW5(21)) --> 541 * 541 * CONFEDERATE () (21)
BE(BE(GROW5(57))) --> 541 * 541 * 541 * CONFEDERATE () (57)

RESULT1 --> 1029 * GROW6 (51)
BE(RESULT1) --> 1029 * GROW6 (51)

RESULT2 --> 1029 * GROW6 (151)
BE(RESULT2) --> 1029 * GROW6 (151)

*/
