#include <stdio.h>
#define STR(x) #x
// no evaluation, and one pass of evaluation
#define SHOW(expr) printf("%s --> %s\n", #expr, STR(expr))

#include "cpp_ext_0.c"

int main(void){
  //--------------------------------------------------------------------------
  // Existence Checks (sanity anchor)
  //--------------------------------------------------------------------------

  #define x0 
  SHOW(  EXISTS_ITEM(x0) );       // → 0 (assumed undefined)
  SHOW(  NOT_EXISTS_ITEM(x0) );   // → 1
  printf("\n");

  SHOW(  MATCH(x0) );
  SHOW(  NOT_MATCH(x0) );
  printf("\n");


  //--------------------------------------------------------------------------
  // Primitive Connectors
  //--------------------------------------------------------------------------

  SHOW( _NOT(0) );  // _RWR_NOT_0 is defined → MATCH → 1
  SHOW( _NOT(1) );  // no rule → MATCH fails → 0
  printf("\n");

  SHOW( _AND(0 ,0) );  // rule defined → MATCH → 1
  SHOW( _AND(1 ,0) );  // rule defined → MATCH → 1
  SHOW( _AND(0 ,1) );  // rule defined → MATCH → 1
  SHOW( _AND(1 ,1) );  // rule defined → MATCH → 1
  SHOW( _AND(1 ,x) );  // not legal args, but worked, rule missing → MATCH fails → 0
  printf("\n");

  SHOW( _OR(0 ,0) );   // rule defined → NOT_MATCH → 0
  SHOW( _OR(0 ,1) );   // rule missing → NOT_MATCH → 1
  SHOW( _OR(1 ,0) );   // rule missing → NOT_MATCH → 1
  SHOW( _OR(1 ,1) );   // rule missing → NOT_MATCH → 1
  printf("\n");



  //--------------------------------------------------------------------------
  // Equality
  //--------------------------------------------------------------------------

  SHOW( _EQ(0 ,0) );     // rule defined → MATCH → 1
  SHOW( _EQ(1 ,1) );     // rule defined → MATCH → 1
  SHOW( _EQ(0 ,1) );     // no rule → MATCH fails → 0
  SHOW( _EQ(x ,x) );     // no rule → MATCH fails → 0
  printf("\n");

  SHOW( _NOT_EQ(0 ,1) ); // rule missing → EXISTS → 1
  SHOW( _NOT_EQ(0 ,0) ); // rule exists → EXISTS → 0
  printf("\n");

  //--------------------------------------------------------------------------
  // Logical Connectors (BOOL + AND/OR/NOT)
  //--------------------------------------------------------------------------

  SHOW( _BOOL_2(0) );       // 0
  SHOW( _BOOL_2(1) );       // 1
  SHOW( _BOOL_2(2) );       //  1 because it exists
  SHOW( _BOOL_2(x0) );       // 0 because it does not exit (see the #define at the top)
  printf("\n");

  SHOW( BOOL(0) );       // _FIRST = 0, EXISTS_ITEM(_FIRST) = 0 → _AND(0 ,1) → 0
  SHOW( BOOL(1) );       // EXISTS_ITEM(1) = 0 → BOOL = 0
  SHOW( BOOL(10) );       // EXISTS_ITEM(1) = 0 → BOOL = 0
  SHOW( BOOL() );       // EXISTS_ITEM(1) = 0 → BOOL = 0
  SHOW( BOOL(x0) );       // EXISTS_ITEM(1) = 0 → BOOL = 0
  printf("\n");

  SHOW( NOT(0) );        // BOOL = 0 → NOT(0) = _NOT(0) = 1
  SHOW( NOT(1) );        // BOOL = 0 → NOT(0) = 1
  SHOW( NOT() );        // BOOL = 0 → NOT(0) = 1
  SHOW( NOT(10) );        // BOOL = 0 → NOT(0) = 1
  printf("\n");

  SHOW( AND(1 ,0) );     // 0
  SHOW( AND(1 ,1) );     // BOOL(1), BOOL(1) = 0,0 → AND(0 ,0) = _AND(0 ,0) = 1
  printf("\n");

  SHOW( OR(0 ,0) );      // BOOL(1), BOOL(0) = 0,0 → OR(0 ,0) = _OR(0 ,0) = 0
  SHOW( OR(1 ,0) );      // BOOL(1), BOOL(0) = 0,0 → OR(0 ,0) = _OR(0 ,0) = 0
  printf("\n");

  return 0;
}
/*
  2025-03-27T12:40:37Z[developer]
  Thomas-developer@Stanley§/home/Thomas-masu/developer/N/developer/example§
  > gcc try_2_connectors.c 

  2025-03-27T12:41:56Z[developer]
  Thomas-developer@Stanley§/home/Thomas-masu/developer/N/developer/example§
  > ./a.out
  EXISTS_ITEM(x0) --> 0
  NOT_EXISTS_ITEM(x0) --> 1

  MATCH(x0) --> MATCH()
  NOT_MATCH(x0) --> NOT_MATCH()

  _NOT(0) --> 1
  _NOT(1) --> 0

  _AND(0 ,0) --> 0
  _AND(1 ,0) --> 0
  _AND(0 ,1) --> 0
  _AND(1 ,1) --> 1
  _AND(1 ,x) --> 0

  _OR(0 ,0) --> 0
  _OR(0 ,1) --> 1
  _OR(1 ,0) --> 1
  _OR(1 ,1) --> 1

  _EQ(0 ,0) --> 1
  _EQ(1 ,1) --> 1
  _EQ(0 ,1) --> 0
  _EQ(x ,x) --> 0

  _NOT_EQ(0 ,1) --> 1
  _NOT_EQ(0 ,0) --> 0

  _BOOL_2(0) --> 0
  _BOOL_2(1) --> 1
  _BOOL_2(2) --> 1
  _BOOL_2(x0) --> 0

  BOOL(0) --> 0
  BOOL(1) --> 1
  BOOL(10) --> 1
  BOOL() --> 0
  BOOL(x0) --> 0

  NOT(0) --> 1
  NOT(1) --> 0
  NOT() --> 1
  NOT(10) --> 0

  AND(1 ,0) --> 0
  AND(1 ,1) --> 1

  OR(0 ,0) --> 0
  OR(1 ,0) --> 1


  2025-03-27T12:41:59Z[developer]
  Thomas-developer@Stanley§/home/Thomas-masu/developer/N/developer/example§
  >
*/
