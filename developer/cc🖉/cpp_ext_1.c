/*
These are the recursive extension.

Simple errors can lead to very long error outputs, which might be why
the cpp designers had obviously intended that recursion would not be possible.

To be added:

LIST_TOO_LONG
  
// scanning left to right finds first that exists
LEFTMOST_EXISTENT 

// scanning left to right finds last that exits
RIGHTMOST_EXISTENT

DROP_NOT_EXISTS_ALL
DROP_NOT_EXISTS_LEFT
DROPE_NOT_EXISTS_RIGHT

#define _CAT(sep,first,...)                              \

*/

#ifndef CPP_EXT_1
#define CPP_EXT_1

/*===========================================================================
  Force extra macro expansion (the EVAL trick)
  This chain of EVAL macros forces the preprocessor to perform many rescans,
  which is necessary to “unroll” recursive macros.
===========================================================================*/
//#define EVAL(...)            EVAL1024(__VA_ARGS__)
#define EVAL(...)            EVAL32(__VA_ARGS__)
#define EVAL1024(...)        EVAL512(EVAL512(__VA_ARGS__))
#define EVAL512(...)         EVAL256(EVAL256(__VA_ARGS__))
#define EVAL256(...)         EVAL128(EVAL128(__VA_ARGS__))
#define EVAL128(...)         EVAL64(EVAL64(__VA_ARGS__))
#define EVAL64(...)          EVAL32(EVAL32(__VA_ARGS__))
#define EVAL32(...)          EVAL16(EVAL16(__VA_ARGS__))
#define EVAL16(...)          EVAL8(EVAL8(__VA_ARGS__))
#define EVAL8(...)           EVAL4(EVAL4(__VA_ARGS__))
#define EVAL4(...)           EVAL2(EVAL2(__VA_ARGS__))
#define EVAL2(...)           EVAL1(EVAL1(__VA_ARGS__))
#define EVAL1(...)           __VA_ARGS__

/*===========================================================================
  Defer macros: these help “hide” recursive calls for additional expansion passes.
===========================================================================*/
#define NULL_FN()
#define DEFER1(m)            m NULL_FN()
#define DEFER2(m)            m NULL_FN NULL_FN()()
#define DEFER3(m)            m NULL_FN NULL_FN NULL_FN()()()
#define DEFER4(m)            m NULL_FN NULL_FN NULL_FN NULL_FN()()()()

/*===========================================================================
  Quantifiers
===========================================================================*/

#define _FIND(predicate ,...) \
  IF \
    ( NOT_EXISTS(__VA_ARGS__) ) \
    () \
    (IF \
      ( predicate(FIRST(__VA_ARGS__)) ) \
      ( FIRST( ,__VA_ARGS__) )                    \
      ( DEFER3(_FIND_CONFEDERATE) ()(predicate ,REST(__VA_ARGS__)) )     \
     )
#define _FIND_CONFEDERATE() _FIND

// number of evals required depends upon length of not found list prefix
#define FIND(predicate ,...) EVAL( _FIND(predicate ,__VA_ARGS__) )


#endif  
