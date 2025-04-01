#include "../cc🖉/cpp_ext_0.c"
#include "../cc🖉/cpp_ext_1.c"

int main(void){

  //--------------------------------------------------------------------------
  // EVAL and DEFER sanity check (no visual output—verifies expansion only)
  //--------------------------------------------------------------------------

  #define PLUS_ONE(x) x + 1
  #define EXAMPLE_CHAIN(x) PLUS_ONE(PLUS_ONE(x))
  SHOW( EVAL(EXAMPLE_CHAIN(3)) ); // -> 3 + 1 + 1
  printf("\n");

  //--------------------------------------------------------------------------
  // FIND: returns first matching item from list
  //--------------------------------------------------------------------------

  #define EQ__A__oo__A
  #define EQ__B__oo__B
  #define EQ__C__oo__C
  #define EQ__D__oo__D

  #define EQ__X__oo__X
  #define EQ__Y__oo__Y
  #define EQ__Z__oo__Z

  #define IS_X(a) EQ(a ,X) // IS makes use of the EQ template for X

  SHOW( FIND(IS_X ,A ,B ,X ,Y ,Z) ); // → X
  SHOW( FIND(IS_X ,A ,B ,C ,D) );    // → ε
  printf("\n");

  //--------------------------------------------------------------------------
  // HAS_ITEM: returns 1 if item found
  //    HAS requires the EQ templates to be set for each case.
  //--------------------------------------------------------------------------

  SHOW( EQ(A ,B) );
  SHOW( EQ(B ,B) );
  printf("\n");

  SHOW( HAS_ITEM(C ,A ,B ,C ,D) );   // → 1
  SHOW( HAS_ITEM(Z ,A ,B ,C ,D) );   // → ε
  printf("\n");


  //--------------------------------------------------------------------------
  // WHILE: stops when predicate fails
  //--------------------------------------------------------------------------

  #define NON_Z(x) NOT(EQ(x ,Z)) // makes uses of the EQ template for Z

  SHOW( WHILE(NON_Z ,A ,B ,C ,Z ,D ,E) ); // → ε (stops at Z)
  SHOW( WHILE(NON_Z ,A ,B ,C) );          // → 1 (all pass)
  printf("\n");

  SHOW(EXISTS());
  SHOW(EXISTS(0));
  SHOW(EXISTS(1));
  SHOW(EXISTS(X));
  SHOW( WHILE(EXISTS ,1 ,X , ));
  SHOW( WHILE(EXISTS ,1 ,  ,X ,0));
  SHOW( WHILE(EXISTS ,1 ,X ,0));

  printf("\n");


  //--------------------------------------------------------------------------
  // LAST: returns final item of list
  //--------------------------------------------------------------------------

  SHOW( LAST(A ,B ,C ,D ,Z) );  // → Z
  SHOW( LAST(A ,B ,) );  // → 
  SHOW( LAST() );              // → ε
  printf("\n");

  //--------------------------------------------------------------------------
  // CAT: join items with a separator
  //--------------------------------------------------------------------------

  //  SHOW( CAT( ;, A ,B ,C ,D ,Z) );  // → Z
  SHOW( CAT(,A ,B ,C) );  // → 
  SHOW( CAT( COMMA, A ,B ,C ,D ,Z) );  // → Z
  SHOW( CAT(,A ,B ,) );  // → 
  SHOW( CAT(,1) );  
  SHOW( CAT(,) );              // → ε
  printf("\n");

  //--------------------------------------------------------------------------
  // AND: true if all items are true (non-empty)
  //--------------------------------------------------------------------------
#if 0
  SHOW( AND(1 ,X ,0) );        // → 1
  SHOW( AND(1 ,X ,) );       // → ε
  printf("\n");

  //--------------------------------------------------------------------------
  // OR: true if any item is true (non-empty)
  //--------------------------------------------------------------------------

  SHOW( OR( , , ,X) );         // → 1
  SHOW( OR( , , ,) );          // → ε
  printf("\n");
#endif
}
