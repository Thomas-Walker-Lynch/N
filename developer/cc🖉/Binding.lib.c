/*
Template parameters:

  Binding·TYPE - Type used to name the binding struct. Binding struct instances get passed as arguments to functions etc.

*/

//--------------------------------------------------------------------------------
// Interface


// once per translation unit
#ifndef Binding·TYPE_LIST
#define Binding·TYPE_LIST

  #define Binding·DEBUG
  #ifdef Binding·DEBUG
    #include <stdio.h>
  #endif

  #include "cpp_ext.c"
  #include "Core.lib.c"

#endif

// once per Binding·TYPE value
#ifdef Binding·TYPE
#if NOT_CONTAINS(Binding·TYPE ,Binding·TYPE_LIST) )
#pragma message( STR_VAL(Binding·TYPE) )

  //this is what it takes to append to a list in cpp ...
  #undef TEMP
  #define TEMP Binding·TYPE_LIST ,Binding·TYPE
  #undef Binding·TYPE_LIST
  #define Binding·TYPE_LIST TEMP

  // a simplifying naming convention
  #define Binding         Ξ(Binding·TYPE)
  #define Binding·FG      Ξ(Binding·TYPE ,FG)
  #define Binding·Tableau Ξ(Binding·TYPE ,Tableau)

  // binds a tableau and Binding table instance together
  // as a convention, we name the binding after the Type
  typedef struct Binding·FG Binding·FG;
  typedef struct Binding·Tableau Binding·Tableau;
  typedef struct{
    Binding·FG *fg;
    Binding·Tableau *tableau;
  } Binding;

  Local inline void Binding·wellformed_binding(Binding b){
    #ifdef Binding·DEBUG
      Core·Guard·init_count(chk);
      Core·Guard·fg.check(&chk, 1, b.fg,      "NULL fg table");
      Core·Guard·fg.check(&chk, 1, b.tableau, "NULL tableau");
      Core·Guard·assert(chk);
    #endif
  }

  /*
    usage e.g.: Binding·call(tm, function_name, arg1, arg2, ...)
    Expands to: (tm.fg->function_name)(tm ,arg1 ,arg2, ...)

    note the use of the comma operator to return the result from the b.fg->fn call
  */
  #define Binding·call(b, fn, ...) \
    ( Binding·wellformed_binding(b) ,b.fg->fn(b, ##__VA_ARGS__) )

#endif
#endif

//--------------------------------------------------------------------------------
// Implementation
//   Binding has no implementation

#ifdef LOCAL

#endif

#ifdef LIBRARY

#endif 

//--------------------------------------------------------------------------------
// template variables work once

#undef Binding·TYPE
