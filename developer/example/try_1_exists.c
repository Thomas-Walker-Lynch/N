#include "cpp_ext_0.c"

#define F 1
#define O1 2
#define O2 3
#define B 4
#define A 5
#define R 6
#define D 7


// Patch for _NOT_EMPTY to succeed on 0
#define _REWRITE_TWION_0 _REWRITE_TWION

int main(void){
  printf("try_1_exists\n\n");

  // Constant macros
  int x COMMA y = 1;  // Tests that COMMA = ,
  printf("y? %d\n", y);
  printf("\n");

  // Token paste test
  int _cat_result = CAT2(CAT3(F ,O1 ,O2) ,CAT4(B ,A ,R ,D));
  printf("FOOBARD %d\n" ,_cat_result);
  printf("\n");

  // Selector macros
  int first = _FIRST(11 ,22 ,33);    // → 11
  int second = _SECOND(11 ,22 ,33);  // → 22
  printf("_FIRST(11 ,22 ,33) = %d\n", first);
  printf("_SECOND(11 ,22 ,33) = %d\n", second);
  printf("\n");

  // Existence detection
  int empty = NOT_EMPTY_ITEM();         // → 1 (rewrite hits)
  int empty_0 = NOT_EMPTY_ITEM(0);      // → 1 (rewrite hits)
  int empty_1 = NOT_EMPTY_ITEM(1);      // → 0 (no rewrite)
  int empty_f = NOT_EMPTY_ITEM(hello);  // → 0

  printf("NOT_EMPTY_ITEM() = %d\n", empty);
  printf("NOT_EMPTY_ITEM(0) = %d\n", empty_0);
  printf("NOT_EMPTY_ITEM(1) = %d\n", empty_1);
  printf("NOT_EMPTY_ITEM(hello) = %d\n", empty_f);
  printf("\n");

  // Not Existence detection
  empty = EMPTY_ITEM();         // → 1 (rewrite hits)
  empty_0 = EMPTY_ITEM(0);      // → 1 (rewrite hits)
  empty_1 = EMPTY_ITEM(1);      // → 0 (no rewrite)
  empty_f = EMPTY_ITEM(hello);  // → 0

  printf("EMPTY_ITEM() = %d\n", empty);
  printf("EMPTY_ITEM(0) = %d\n", empty_0);
  printf("EMPTY_ITEM(1) = %d\n", empty_1);
  printf("EMPTY_ITEM(hello) = %d\n", empty_f);
  printf("\n");


  //  int empty_10 = NOT_EMPTY_ITEM(10,11,12); // illegal call, try it anyway ..compilation error
  int empty_11 = NOT_EMPTY(10,11,12); // this is a legal call
  int empty_12 = NOT_EMPTY(); 

  // printf("NOT_EMPTY_ITEM(10,11,12) = %d\n", empty_10);
  printf("NOT_EMPTY(10,11,12) = %d\n", empty_11);
  printf("NOT_EMPTY() = %d\n", empty_12);
  printf("\n");

  //  int empty_10 = NOT_EMPTY_ITEM(10,11,12); // illegal call, try it anyway ..compilation error
  empty_11 = EMPTY(10,11,12); // this is a legal call
  empty_12 = EMPTY(); 

  // printf("EMPTY_ITEM(10,11,12) = %d\n", empty_10);
  printf("EMPTY(10,11,12) = %d\n", empty_11);
  printf("EMPTY() = %d\n", empty_12);
  printf("\n");


  return 0;
}

/*
  2025-03-27T12:38:04Z[developer]
  Thomas-developer@Stanley§/home/Thomas-masu/developer/N/developer/example§
  > gcc try_1_exists.c 

  2025-03-27T12:40:32Z[developer]
  Thomas-developer@Stanley§/home/Thomas-masu/developer/N/developer/example§
  > ./a.out
  _CAT(FOO ,BAR) → 12345

  _FIRST(11 ,22 ,33) = 11
  _SECOND(11 ,22 ,33) = 22

  NOT_EMPTY_ITEM() = 0
  NOT_EMPTY_ITEM(0) = 1
  NOT_EMPTY_ITEM(1) = 1
  NOT_EMPTY_ITEM(hello) = 1

  EMPTY_ITEM() = 1
  EMPTY_ITEM(0) = 0
  EMPTY_ITEM(1) = 0
  EMPTY_ITEM(hello) = 0

  NOT_EMPTY(10,11,12) = 1
  NOT_EMPTY() = 0

  EMPTY(10,11,12) = 0
  EMPTY() = 1
*/
