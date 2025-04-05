/*
Namespace: Binding
Template parameters:

  _BINDING_ - Type used to name the binding struct. Binding struct instances get passed as arguments to functions etc.

This files declares a binding struct named [_BINDING_]. It also provides
the macro `call` that uses the binding.

Note this file does not make an instance of the binding, and does not intialize
fields in an instance.



*/

/*--------------------------------------------------------------------------------
 Interface
-------------------------------------------------------------------------------*/


// once per translation unit
#ifndef Binding·FACE
#define Binding·FACE

  #define Binding·DEBUG
  #ifdef Binding·DEBUG
    #include <stdio.h>
    #pragma message( "in #ifndef Binding_LIST section" )
  #endif

  #include "cpp_ext.c"
  #include "Core.lib.c"

  /*
    usage e.g.: Binding·call(tm, function_name, arg1, arg2, ...)
    Expands to: (tm.fg->function_name)(tm ,arg1 ,arg2, ...)

    note the use of the comma operator to return the result from the b.fg->fn call
  */
  #ifdef Binding·DEBUG
    #include <assert.h>
    #define Binding·call(b ,fn ,...) ( \
       assert((b).fg != NULL) \
      ,assert((b).tableau != NULL) \
      ,(b).fg->fn(b __VA_OPT__(,) __VA_ARGS__) \
      )
  #else
    #define Binding·call(b ,fn ,...) \
      (b).fg->fn(b __VA_OPT__(,) __VA_ARGS__)
  #endif

#endif


// once per Binding value
#ifdef _BINDING_
#if BOOLEAN(NOT_IN(Binding·SET ,_BINDING_))
#ifdef Binding·DEBUG
  #pragma message( "adding binding for:" STR_VAL(_BINDING_) )
#endif

  struct ·(_BINDING_ ,FG);
  typedef struct ·(_BINDING_ ,FG) ·(_BINDING_ ,FG);

  struct ·(_BINDING_ ,Tableau);
  typedef struct ·(_BINDING_ ,Tableau) ·(_BINDING_ ,Tableau); 

  typedef struct _BINDING_{
    ·(_BINDING_ ,Tableau) *tableau;
    ·(_BINDING_ ,FG) *fg;
  } _BINDING_;

#endif 
#endif 


/*--------------------------------------------------------------------------------
 Local - at bottom of translation unit, to keep some functions private
   Binding currently has no local component
--------------------------------------------------------------------------------*/
#ifdef LOCAL

#endif

/*--------------------------------------------------------------------------------
 Library - compiled into a lib.a file by the current make
   Binding has no library component
--------------------------------------------------------------------------------*/
#ifdef LIBRARY

#endif 

/*--------------------------------------------------------------------------------
 undef the template parameters
-------------------------------------------------------------------------------*/
#undef _BINDING_

