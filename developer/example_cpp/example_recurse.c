// from http://jhnet.co.uk/articles/cpp_magic

#include <stdio.h>

#define STR(...) #__VA_ARGS__
// no evaluation, and one pass of evalutation
#define SHOW(expr) printf("%s --> %s\n", #expr, STR(expr))

#define NULL_FN()

#define EVAL32(...) EVAL16(EVAL16(__VA_ARGS__))
#define EVAL16(...) EVAL8(EVAL8(__VA_ARGS__))
#define EVAL8(...) EVAL4(EVAL4(__VA_ARGS__))
#define EVAL4(...) EVAL2(EVAL2(__VA_ARGS__))
#define EVAL2(...) EVAL1(EVAL1(__VA_ARGS__))
#define EVAL1(...) __VA_ARGS__

#define DEFER1(m) m NULL_FN()

#define RECURSE() I am recursive, look: DEFER1(_RECURSE)()()
#define _RECURSE() RECURSE


int main(){

  printf(
    "No EVAL:\n"
    "    RECURSE()\n"
  );
  SHOW(RECURSE());
  printf("\n");

  printf(
    "With EVAL1:\n"
    "    EVAL1(RECURSE())\n"
  );
  SHOW(EVAL1(RECURSE()));
  printf("\n"); 

  printf(
    "With EVAL2:\n"
    "    EVAL2(RECURSE())\n"
  );
  SHOW(EVAL2(RECURSE()));
  printf("\n"); 

  printf(
    "With EVAL32:\n"
    "    EVAL32(RECURSE())\n"
  );
  SHOW(EVAL32(RECURSE()));
  printf("\n");
         
};
