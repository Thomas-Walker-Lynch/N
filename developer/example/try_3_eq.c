#include "cpp_ext_0.c"

int main(void){

  //---------------------------------------------------------------------------
  // EQ Tests — identity and mismatch
  //---------------------------------------------------------------------------

  SHOW(EQ(0 ,0));      // Expected → 1
  SHOW(EQ(1 ,1));      // Expected → 1
  SHOW(EQ(0 ,1));      // Expected → 0
  SHOW(EQ(1 ,0));      // Expected → 0

  SHOW(EQ(x ,x));      // Expected → 0 (no _RWREQ__x__oo__x defined)
  SHOW(EQ(x ,y));      // Expected → 0

  //---------------------------------------------------------------------------
  // NOTEQ Tests — inverse behavior
  //---------------------------------------------------------------------------

  SHOW(NOT_EQ(0 ,0));  // Expected → 0 (because EQ → 1, so EXISTS → 0)
  SHOW(NOT_EQ(1 ,1));  // Expected → 0
  SHOW(NOT_EQ(0 ,1));  // Expected → 1
  SHOW(NOT_EQ(1 ,0));  // Expected → 1

  SHOW(NOT_EQ(x ,x));  // Expected → 1 (EQ not defined → NOTEQ = 1)
  SHOW(NOT_EQ(x ,y));  // Expected → 1

  //---------------------------------------------------------------------------
  // Mixed cases with macros
  //---------------------------------------------------------------------------

  #define XSYM 0
  #define YSYM 1

  SHOW(EQ(XSYM ,XSYM));      // Expected → 1 (0,0)
  SHOW(EQ(XSYM ,YSYM));      // Expected → 0 (0,1)
  SHOW(NOT_EQ(XSYM ,YSYM));  // Expected → 1

  SHOW(EQ(XSYM ,0));         // Expected → 1
  SHOW(EQ(XSYM ,1));         // Expected → 0

  SHOW(NOT_EQ(YSYM ,0));     // Expected → 1
  SHOW(NOT_EQ(YSYM ,1));     // Expected → 0

  //---------------------------------------------------------------------------
  // Edge case: undefined symbol
  //---------------------------------------------------------------------------

  SHOW(EQ(z ,z));        // Expected → 0
  SHOW(NOT_EQ(z ,z));    // Expected → 1

  printf("\n");
  return 0;
}
