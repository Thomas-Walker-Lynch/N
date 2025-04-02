/*
This files declares a binding struct named [Binding·TYPE]. It also provides
the macro `call` that uses the binding.

Note this file does not make an instance of the binding, and does not intialize
fields in an instance.

Template parameters:

  Binding·TYPE - Type used to name the binding struct. Binding struct instances get passed as arguments to functions etc.

*/

//--------------------------------------------------------------------------------
// Interface
//--------------------------------------------------------------------------------


// once per translation unit
#ifndef Binding·FACE
#define Binding·FACE

  #define Binding·DEBUG
  #ifdef Binding·DEBUG
    #include <stdio.h>
    #pragma message( "in #ifndef Binding·TYPE_LIST section" )
  #endif

  #include "cpp_ext.c"
  #include "Core.lib.c"

  #define Binding·NO_BINDING_FOR(x) BOOLEAN(NOT(CAT2(Binding·TYPE· ,x)))

  /*
    usage e.g.: Binding·call(tm, function_name, arg1, arg2, ...)
    Expands to: (tm.fg->function_name)(tm ,arg1 ,arg2, ...)

    note the use of the comma operator to return the result from the b.fg->fn call
  */
  #define Binding·call(b, fn, ...) \
    ( \
      Binding·wellformed_binding(b) , \
      b.fg->fn(b.tableau __VA_OPT__(,) __VA_ARGS__) \
      )

#endif


// once per Binding·TYPE value
#ifdef Binding·TYPE
#if Binding·NO_BINDING_FOR(Binding·TYPE)
#ifdef Binding·DEBUG
  #pragma message( "adding binding for:" STR_VAL(Binding·TYPE) )
#endif

  struct Ξ(Binding·TYPE ,FG);
  typedef struct Ξ(Binding·TYPE ,FG) Ξ(Binding·TYPE ,FG);

  struct Ξ(Binding·TYPE ,Tableau);
  typedef struct Ξ(Binding·TYPE ,Tableau) Ξ(Binding·TYPE ,Tableau); 

  typedef struct Ξ(Binding·TYPE){
    Ξ(Binding·TYPE ,Tableau) *tableau;
    Ξ(Binding·TYPE ,FG) *fg;
  } Ξ(Binding·TYPE);

  static void Binding·wellformed_binding(Ξ(Binding·TYPE) b){
    #ifdef Binding·DEBUG
      Core·Guard·init_count(chk);
      Core·Guard·fg.check(&chk, 1, b.fg,      "NULL fg table");
      Core·Guard·fg.check(&chk, 1, b.tableau, "NULL tableau");
      Core·Guard·assert(chk);
    #endif
  }

#endif 
#endif 


//--------------------------------------------------------------------------------
// Local - at bottom of translation unit, to keep some functions private
//   Binding currently has no local component
//--------------------------------------------------------------------------------

#ifdef LOCAL

#endif

//--------------------------------------------------------------------------------
// Library - compiled into a lib.a file by the current make
//   Binding has no library component
//--------------------------------------------------------------------------------
#ifdef LIBRARY

#endif 

//--------------------------------------------------------------------------------
// undef the template parameters
//--------------------------------------------------------------------------------
#undef Binding·TYPE

