#include "cpp_ext_0.c"
#include "cpp_ext_1.c"

#define LIST int ,float ,char ,void 

// register equality
#define EQ__int__oo__int
#define EQ__float__oo__float
#define EQ__char__oo__char
#define EQ__void__oo__void

// make predicates
#define IS_int(x)   EQ(x ,int)
#define IS_float(x) EQ(x ,float)
#define IS_double(x)  EQ(x ,double)
#define IS_char(x)  EQ(x ,char)
#define IS_void(x)  EQ(x ,void)

int main(void){

#define BE(...) __VA_ARGS__

  SHOW(IS_char(int));
  SHOW(IS_char(char));
  printf("\n");

  SHOW( FIND(IS_char ,) );
  SHOW( FIND(IS_char ,char) );  // → char
  SHOW( FIND(IS_char ,int) );  // → 
  printf("\n");

  SHOW( FIND(IS_char ,int ,void) );  // → 
  SHOW( FIND(IS_void ,int ,void) );  // → 
  printf("\n");

  SHOW( FIND(IS_int ,LIST) ); 
  SHOW( FIND(IS_float ,LIST) ); 
  SHOW( FIND(IS_double ,LIST) ); 
  SHOW( FIND(IS_char ,LIST) ); 
  SHOW( FIND(IS_void ,LIST) ); 
  SHOW( FIND(IS_big ,LIST) ); 
  printf("\n");


  //#endif

  return 0;
}
