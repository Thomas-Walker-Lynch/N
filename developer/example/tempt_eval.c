#include <stdio.h>
#include "macro_lib.c"

/* A very simple base macro that expands to an integer literal */
#define SIMPLE_MACRO1 549
#define SIMPLE_MACRO2 7919

/* Define a chain of macros that simply pass along the value.
   Each CALL_n() macro just calls the previous level.
*/
#define CALL1() SIMPLE_MACRO1
#define CALL2a() CALL1 EMPTY() () + 3
#define CALL2b() DEFER1(CALL1)() + 5

int main(void) {

  printf( "SIMPLE_MACRO1 = %d\n", SIMPLE_MACRO1 );
  printf( "CALL1()  = %d\n", EVAL(CALL1()) );
  printf( "CALL2a() = %d\n", EVAL(CALL2a()) );
  printf( "CALL2b() = %d\n", EVAL(CALL2b()) );

  return 0;
}
