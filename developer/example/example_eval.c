#include <stdio.h>
#define STR(x) #x
#define SHOW(expr) printf("%s --> %s\n", #expr, STR(expr))

int main(void){
  printf("example_eval.c\n");

  #define EMPTY()
  #define NEGATE(x) -x
  #define NOT_SO_FAST(x)  NEGATE EMPTY() (x)
  SHOW(NOT_SO_FAST(5));

  #define BE(x) x
  SHOW(BE(NOT_SO_FAST(5)))
}

/*
  2025-03-27T04:47:02Z[developer]
  Thomas-developer@Stanley§/home/Thomas-masu/developer/N/developer/example§
  > gcc example_eval.c

  2025-03-27T04:47:32Z[developer]
  Thomas-developer@Stanley§/home/Thomas-masu/developer/N/developer/example§
  > ./a.out
  example_eval.c
  NOT_SO_FAST(5) → NEGATE (5)
  BE(NOT_SO_FAST(5)) → -5

*/  
