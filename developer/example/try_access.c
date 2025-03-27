#include <stdio.h>
#include "cpp_ext_0.c"

#define SHOW(x) printf(#x " = %s\n", x)

#define STRINGIFY(x) #x
#define EXPAND_STRING(x) STRINGIFY(x)

int main(void){
  //---------------------------------------------------------------------------
  // FIRST with pad fallback
  //---------------------------------------------------------------------------
  SHOW(EXPAND_STRING(FIRST(none)));             // → none
  SHOW(EXPAND_STRING(FIRST(pad ,x)));           // → x
  SHOW(EXPAND_STRING(FIRST(pad ,x ,y ,z)));     // → x

  //---------------------------------------------------------------------------
  // SECOND with pad fallback
  //---------------------------------------------------------------------------
  SHOW(EXPAND_STRING(SECOND(pad)));             // → pad
  SHOW(EXPAND_STRING(SECOND(pad ,a)));          // → pad
  SHOW(EXPAND_STRING(SECOND(pad ,a ,b)));       // → b
  SHOW(EXPAND_STRING(SECOND(pad ,a ,b ,c)));    // → b

  //---------------------------------------------------------------------------
  // THIRD with pad fallback
  //---------------------------------------------------------------------------
  SHOW(EXPAND_STRING(THIRD(pad)));              // → pad
  SHOW(EXPAND_STRING(THIRD(pad ,a)));           // → pad
  SHOW(EXPAND_STRING(THIRD(pad ,a ,b)));        // → pad
  SHOW(EXPAND_STRING(THIRD(pad ,a ,b ,c)));     // → c
  SHOW(EXPAND_STRING(THIRD(pad ,a ,b ,c ,d)));  // → c

  //---------------------------------------------------------------------------
  // REST returns the tail after the first item
  //---------------------------------------------------------------------------
  SHOW(EXPAND_STRING(REST()));                  // → (empty)
  SHOW(EXPAND_STRING(REST(a)));                 // → (empty)
  SHOW(EXPAND_STRING(REST(a ,b)));              // → b
  SHOW(EXPAND_STRING(REST(a ,b ,c)));           // → b ,c

  return 0;
}
