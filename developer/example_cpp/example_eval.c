#include <stdio.h>
#define STR(x) #x
#define SHOW(expr) printf("%s --> %s\n", #expr, STR(expr))

int main(void){
  printf("example_eval.c\n");

  #define NULL_FN()
  #define NEGATE(x) -x
  #define NOT_SO_FAST(x)  NEGATE NULL_FN() (x)
  SHOW(NOT_SO_FAST(5));

  #define BE(x) x
  SHOW(BE(NOT_SO_FAST(5)));
}

