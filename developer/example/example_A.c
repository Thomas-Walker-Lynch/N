// from http://jhnet.co.uk/articles/cpp_magic

#include <stdio.h>
#define STR(x) #x
#define SHOW(expr) printf("%s --> %s\n", #expr, STR(expr))

#define NULL_FN()
#define EVAL1(...) __VA_ARGS__

#define A(n) I like the number n


int main(){

  printf(
    "    I like the number 123\n"
    "    A (123)\n"
  );
  SHOW(A (123));
  SHOW(A NULL_FN() (123));
  printf("\n");

  printf(
    "    I like the number 123\n"
  );
  SHOW(EVAL1(A NULL_FN() (123)));
  printf("\n");


   /*
     ... The reason this works is that when CPP encounters a function-style macro, it recursively expands the macro's arguments before substituting the macro's body and expanding that. ...
   */
         
};
