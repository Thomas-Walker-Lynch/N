#include <stdio.h>
#include "cpp_RT.c"


#define STR(x) #x
#define XSTR(x) STR(x)


// Patch for CAT to succeed
#define FOOBAR 12345

// Patch for _EXISTS to succeed on 0
#define _REWRITE_TWION_0 _REWRITE_TWION

int main(void){

  // Constant macros
  int x COMMA y = 1;  // Tests that COMMA = ,

  // Token paste test
  int _cat_result = FOOBAR; // _CAT(FOO ,BAR) → FOOBAR → 12345
  printf("_CAT(FOO ,BAR) → %d\n", _cat_result);
  printf("\n");

  // Selector macros
  int first = _FIRST(11 ,22 ,33);    // → 11
  int second = _SECOND(11 ,22 ,33);  // → 22
  printf("_FIRST(11 ,22 ,33) = %d\n", first);
  printf("_SECOND(11 ,22 ,33) = %d\n", second);
  printf("\n");

  // Existence detection
  int empty = EXISTS_ITEM();         // → 1 (rewrite hits)
  int empty_0 = EXISTS_ITEM(0);      // → 1 (rewrite hits)
  int empty_1 = EXISTS_ITEM(1);      // → 0 (no rewrite)
  int empty_f = EXISTS_ITEM(hello);  // → 0

  printf("EXISTS_ITEM() = %d\n", empty);
  printf("EXISTS_ITEM(0) = %d\n", empty_0);
  printf("EXISTS_ITEM(1) = %d\n", empty_1);
  printf("EXISTS_ITEM(hello) = %d\n", empty_f);
  printf("\n");

  // Not Existence detection
  empty = NOT_EXISTS_ITEM();         // → 1 (rewrite hits)
  empty_0 = NOT_EXISTS_ITEM(0);      // → 1 (rewrite hits)
  empty_1 = NOT_EXISTS_ITEM(1);      // → 0 (no rewrite)
  empty_f = NOT_EXISTS_ITEM(hello);  // → 0

  printf("NOT_EXISTS_ITEM() = %d\n", empty);
  printf("NOT_EXISTS_ITEM(0) = %d\n", empty_0);
  printf("NOT_EXISTS_ITEM(1) = %d\n", empty_1);
  printf("NOT_EXISTS_ITEM(hello) = %d\n", empty_f);
  printf("\n");


  //  int empty_10 = EXISTS_ITEM(10,11,12); // illegal call, try it anyway ..compilation error
  int empty_11 = EXISTS(10,11,12); // this is a legal call
  int empty_12 = EXISTS(); 

  // printf("EXISTS_ITEM(10,11,12) = %d\n", empty_10);
  printf("EXISTS(10,11,12) = %d\n", empty_11);
  printf("EXISTS() = %d\n", empty_12);
  printf("\n");

  //  int empty_10 = EXISTS_ITEM(10,11,12); // illegal call, try it anyway ..compilation error
  empty_11 = NOT_EXISTS(10,11,12); // this is a legal call
  empty_12 = NOT_EXISTS(); 

  // printf("NOT_EXISTS_ITEM(10,11,12) = %d\n", empty_10);
  printf("NOT_EXISTS(10,11,12) = %d\n", empty_11);
  printf("NOT_EXISTS() = %d\n", empty_12);
  printf("\n");


  return 0;
}
