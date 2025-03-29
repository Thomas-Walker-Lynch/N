#include "../cc🖉/cpp_ext_0.c"
#include "../cc🖉/cpp_ext_1.c"

#define LIST Integer ,Float ,Char ,Void 

// register equality
#define EQ__Integer__oo__Integer
#define EQ__Float__oo__Float
#define EQ__Char__oo__Char
#define EQ__Void__oo__Void

// make predicates
#define IS_Integer(x) EQ(x ,Integer)
#define IS_Float(x)   EQ(x ,Float)
#define IS_Missing(x) EQ(x ,Missing)
#define IS_Char(x)    EQ(x ,Char)
#define IS_Void(x)    EQ(x ,Void)

int main(void){

  SHOW(IS_Char(Integer));
  SHOW(IS_Char(Char));
  printf("\n");

  SHOW( FIND(IS_Char ,) );
  SHOW( FIND(IS_Char ,Char) ); 
  SHOW( FIND(IS_Char ,Integer) ); 
  printf("\n");

  SHOW( FIND(IS_Char ,Integer ,Void) ); 
  SHOW( FIND(IS_Void ,Integer ,Void) ); 
  printf("\n");

  SHOW( FIND(IS_Integer ,LIST) ); 
  SHOW( FIND(IS_Float ,LIST) ); 
  SHOW( FIND(IS_Missing ,LIST) ); 
  SHOW( FIND(IS_Char ,LIST) ); 
  SHOW( FIND(IS_Void ,LIST) ); 
  SHOW( FIND(IS_big ,LIST) ); // literal 'IS_big' will be seen as true
  printf("\n");

  SHOW( FIND_ITEM(Char ,) );
  SHOW( FIND_ITEM(Char ,Char) );  
  SHOW( FIND_ITEM(Char ,Integer) );  
  printf("\n");

  SHOW( FIND_ITEM(Char ,Integer ,Void) ); 
  SHOW( FIND_ITEM(Void ,Integer ,Void) ); 
  printf("\n");

  SHOW( FIND_ITEM(Integer ,LIST) ); 
  SHOW( FIND_ITEM(Float ,LIST) ); 
  SHOW( FIND_ITEM(Missing ,LIST) ); 
  SHOW( FIND_ITEM(Char ,LIST) ); 
  SHOW( FIND_ITEM(Void ,LIST) ); 
  SHOW( FIND_ITEM(big ,LIST) ); 
  printf("\n");

  return 0;
}
