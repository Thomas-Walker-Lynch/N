#include "cpp_ext_0.c"
#include "cpp_ext_1.c"

#define LIST int ,float ,char ,void ,long

// Define a predicate macro: returns 1 if argument is "char"
#define IS_char(x) _IS_char_##x
#define _IS_char_char 1

int main(void){

  //--------------------------------------------------------------------------
  // FIND — returns first matching element from the list
  //--------------------------------------------------------------------------

  // Use FIND to locate "char" in the list
  SHOW( FIND((LIST) ,IS_char) );  // → char

  // Try a predicate that won’t match
  #define IS_double(x) _IS_double_##x
  #define _IS_double_double 1
  SHOW( FIND((LIST) ,IS_double) ); // → (empty)

  // Show that FIND returns the first match
  #define IS_type(x) _IS_type_##x
  #define _IS_type_float 1
  #define _IS_type_void  1
  SHOW( FIND((LIST) ,IS_type) );   // → float (not void)

  return 0;
}
