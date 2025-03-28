/*
Template parameters:

  FG·Type - Type used to name the binding struct. Binding struct instances get passed as arguments to functions etc.

Template variables are _use once_ per #include. (They are #undef at the bottom of this file.)
       
*/

#include "cpp_ext.c"

#ifndef FACE
#define FG·IMPLEMENTATION
#define FACE
#endif 

//--------------------------------------------------------------------------------
// Interface 

// once per translation unit
#ifndef FG·TYPE_LIST
#define FG·TYPE_LIST

  #define FG·DEBUG
  #ifdef FG·DEBUG
    #include <stdio.h>
  #endif

#endif

// once per FG·TYPE value
#if ! FIND_ITEM( FG·TYPE ,FG·TYPE_LIST )
#define FG·TYPE_LIST APPEND(FG·TYPE_LIST ,FG·TYPE)

  // a simplifying naming convention
  #define FG·Binding Ξ(FG·Type)
  #define FG·FG      Ξ(FG·Type ,FG)
  #define FG·Tableau Ξ(FG·Type ,Tableau)

  // as a convention, we name the binding after the type
  typedef struct{
    FG·FG *fg;
    FG·Tableau *tableau;
  } FG·Binding;

  // binds a tableau and FG tabel instance together
  inline void FG·wellformed_binding(FG·Binding b){
    #ifdef FG·DEBUG
      FG·Guard·init_count(chk);
      FG·Guard·fg.check(&chk, 1, b.fg,      "NULL fg table");
      FG·Guard·fg.check(&chk, 1, b.tableau, "NULL tableau");
      FG·Guard·assert(chk);
    #endif
  }

  /*
    usage e.g.: FG·call(tm, function_name, arg1, arg2, ...)
    Expands to: (tm.fg->function_name)((tm)->t, arg1, arg2, ...)

    note the use of the comma operator to return the result from the b.fg->fn call
  */
  #define FG·call(b, fn, ...) \
    ( FG·wellformed_binding(b) ,b.fg->fn(b.tableau, ##__VA_ARGS__) )

#endif // FACE

//--------------------------------------------------------------------------------
// Implementation

#ifdef FG·IMPLEMENTATION

  //----------------------------------------
  // implementation to go into the lib.a file
  //----------------------------------------
  #ifndef LOCAL
  #endif //#ifndef LOCAL

  //----------------------------------------
  // implementation to go at the bottom of the source file
  //----------------------------------------
  #ifdef LOCAL
  #endif // #ifdef LOCAL

#endif // IMPLEMENTATION

// template variables are use once
#undef FG·Type

