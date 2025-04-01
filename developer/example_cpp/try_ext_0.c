#include "../cc🖉/cpp_ext_0.c"

int main(void){
  //--------------------------------------------------------------------------
  // Existence Checks (sanity anchor)
  //--------------------------------------------------------------------------

  SHOW( _FIRST(1) ); 
  SHOW( _FIRST() );  // returns nothing, as empty args are permitted
  SHOW( _SECOND(1,2) );
  // SHOW( _SECOND(1) ); // fails, as _SECOND needs at least two args
  SHOW( _SECOND(1,) ); // returns nothing, as empty args arg slots are permitted
  printf("\n");

  SHOW( RETURN_NOTHING() );
  printf("\n");

  SHOW( _OR() );
  SHOW( _OR(1) );
  SHOW( _OR(1,2,3) );
  printf("\n");

  #define T(x) 7
  SHOW( T(x) );

  SHOW( _NOT_ITEM() );
  SHOW( _NOT_ITEM(1) );
  SHOW( _NOT_ITEM(T(x)) );
  // SHOW( NOT_ITEM(1,2,3) ); // compiler error parameter arity mismatch
  printf("\n");

  SHOW( NOT() );
  SHOW( NOT(1) );
  SHOW( NOT(T(x)) );
  SHOW( NOT(1,2,3) );
  printf("\n");

  SHOW( BOOL() );
  SHOW( CAT2(_IF_ ,BOOL()) );
  printf("\n");

  SHOW( TO_1_OR_0() );
  SHOW( TO_1_OR_0(1) );
  SHOW( TO_1_OR_0(x) );
  SHOW( TO_1_OR_0(1.2.3) );
  printf("\n");

  //--------------------------------------------------------------------------
  // EXISTS
  //--------------------------------------------------------------------------

  SHOW( EXISTS() );            // false → ε
  SHOW( EXISTS(0) );           // true → 1
  SHOW( EXISTS(x,y,z) );       // true → 1
  printf("\n");

  //--------------------------------------------------------------------------
  // Logic Connectors
  //--------------------------------------------------------------------------

  SHOW( LEQ2( ,    ) );  // () implies () → 1
  SHOW( LEQ2( ,  1 ) );  // () implies 1 → 1
  SHOW( LEQ2( 1,    ) ); // 1  implies () → () (fail)
  SHOW( LEQ2( 1,  0 ) ); // 1  implies 0 → 0 (still true!)
  printf("\n");

  SHOW( XOR2( ,    ) );  // both false → false
  SHOW( XOR2( ,  0 ) );  // one true → true
  SHOW( XOR2( 0,  0 ) ); // both true → false
  printf("\n");

  SHOW( AND2( ,  0 ) );  // one false → false
  SHOW( AND2( 0,  1 ) ); // both true → true
  printf("\n");

  SHOW( OR2( ,    ) );   // both false → false
  SHOW( OR2( ,  0 ) );   // one true → true
  printf("\n");

  //--------------------------------------------------------------------------
  // Registered Equivalence
  //--------------------------------------------------------------------------

  #define EQ__APPLE__oo__APPLE
  #define EQ__PEAR__oo__PEAR
    
  SHOW( EQ(APPLE ,APPLE) );
  SHOW( EQ(APPLE ,PEAR) );
  SHOW( EQ(PEAR ,PEAR) );
  SHOW( EQ(,) );
  SHOW( EQ(,PEAR) );
  SHOW( EQ(PEAR ,) );
  printf("\n");

 //--------------------------------------------------------------------------
  // BOOLEAN
  //--------------------------------------------------------------------------

  SHOW( BOOLEAN() );           // false → 0
  SHOW( BOOLEAN(0) );          // true → 1
  SHOW( BOOLEAN(foo) );        // true → 1
  SHOW( BOOLEAN(1,2,3) );      // true → 1
  printf("\n");

  //--------------------------------------------------------------------------
  // Remainder of a list
  //--------------------------------------------------------------------------

  SHOW(REST());
  SHOW(REST(1));
  SHOW(REST(1,2));
  SHOW(REST(1,2,3));


}
  
