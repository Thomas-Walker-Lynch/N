/*
These are the recursive extension.

Simple errors can lead to very long error outputs, which might be why
the cpp designers had obviously intended that recursion would not be possible.

To be added:

LIST_TOO_LONG
  
// scanning left to right finds first that exists
LEFTMOST_NOT_EMPTY

// scanning left to right finds last that exits
RIGHTMOST_NOT_EMPTY

DROP_EMPTY_ALL
DROP__EMPTY_LEFT
DROPE_EMPTY_RIGHT

#define _CAT(sep,first,...)                              \

*/

#ifndef CPP_EXT_1
#define CPP_EXT_1

/*===========================================================================
  Force extra macro expansion (the EVAL trick)
  This chain of EVAL macros forces the preprocessor to perform many rescans,
  which is necessary to “unroll” recursive macros.
===========================================================================*/
#define EVAL(...)            EVAL1024(__VA_ARGS__)
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
#define EMPTY()
#define DEFER1(m)            m EMPTY()
#define DEFER2(m)            m EMPTY EMPTY()()
#define DEFER3(m)            m EMPTY EMPTY EMPTY()()()
#define DEFER4(m)            m EMPTY EMPTY EMPTY EMPTY()()()  /* as needed */

/*===========================================================================
  Quantifiers
===========================================================================*/

// left right scan returns first element where predicate is true
#define _FIND(list ,predicate) \
  IF \
    ( EMPTY(list) ) \
    () \
    (IF \
      ( predicate(FIRST(list)) ) \
      ( FIRST(list) ) \
      ( DEFER1(_FIND_CONF)()(REST(list)) )

#dfine _FIND_CONF() _FIND

#define FIND(list ,predicate) EVAL64(_FIND(list ,predicate))


#endif  
