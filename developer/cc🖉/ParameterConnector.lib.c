/*
Namespace: ParameterConnector

Defines a macro for declaring parameter connectors, and for calling a parameterized function.

Declares a parameter connector, does not initialize it.

*/

/*--------------------------------------------------------------------------------
 Interface
-------------------------------------------------------------------------------*/


// once per translation unit
#ifndef ParameterConnector·FACE
#define ParameterConnector·FACE

  #define ParameterConnector·DEBUG
  #ifdef ParameterConnector·DEBUG
    #include <stdio.h>
    #pragma message( "ParameterConnector·FACE" )
  #endif

  #include "cpp_ext.c"
  #include "Core.lib.c"

  /*
    usage e.g.: ParameterConnector·call(tm, function_name, arg1, arg2, ...)
    Expands to: (tm.pft->function_name)(tm ,arg1 ,arg2, ...)

    note the use of the comma operator to return the result from the b.pft->fn call
  */
  #ifdef ParameterConnector·DEBUG
    #include <assert.h>
    #define ParameterConnector·call(pc_instance ,fn ,...) ( \
       assert((pc_instance).pft != NULL) \
      ,assert((pc_instance).tableau != NULL) \
      ,(b).pft->fn(b __VA_OPT__(,) __VA_ARGS__) \
      )
  #else
    #define ParameterConnector·call(pc_instance ,fn ,...) \
      (pc_instance).pft->fn(pc_instance __VA_OPT__(,) __VA_ARGS__)
  #endif

  #define ParameterConnector·DECLARE(type)                \
    typedef struct ·(type ,PFT) ·(type ,PFT);               \
    typedef struct ·(type ,Tableau) ·(type ,Tableau);     \
    typedef struct type {                                 \
      ·(type ,Tableau) *tableau;                          \
      ·(type ,PFT) *pft;                                    \
    } type;

#endif



/*--------------------------------------------------------------------------------
 Local - at bottom of translation unit, to keep some functions private
   ParameterConnector currently has no local component
--------------------------------------------------------------------------------*/
#ifdef LOCAL

#endif

/*--------------------------------------------------------------------------------
 Library - compiled into a lib.a file by the current make
   ParameterConnector has no library component
--------------------------------------------------------------------------------*/
#ifdef LIBRARY

#endif 


