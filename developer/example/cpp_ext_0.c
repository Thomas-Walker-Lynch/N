/*
  See also 
    https://github.com/18sg/uSHET/blob/master/lib/cpp_magic.h 
    and tutorial at: http://jhnet.co.uk/articles/cpp_magic

    documents in $REPO_HOME/developer/document🖉

1. Provides:

  Raw constants: ZERO, ONE, COMMA, SEMICOLON

  Token tagging: _TWION_0, _TWION_1

  Controlled concatenation: CAT2, CAT3, CAT4

  Existence (now NOT_EMPTY) with token safety via ##

  Rewrite matchers: MATCH_RWR, NOT_MATCH_RWR

  Logical base: _AND, _OR, _NOT, BOOL, EQ, NOT_EQ

  Structural access: FIRST, REST, SECOND, THIRD

  Branching: IF_ELSE(...), with macro-safe conditioning

  Tests for all major features: try_eq.c, try_access.c, try_3_if.c, etc.

2.
  These are the non-recursive extensions.  See cpp_ext_1 for the recursive extensions.
 
3.
  The 'twion' is cute. It is reminiscent of a complex number in math. Like the complex number that is fed into scalar equestions.  The twion is a pair given to functions that only take singletons.

4.
  0 is false
  1 is true

  Macros starting with an '_' (underscore) are private.

  EQ comparisons apart from logic comparisons, must be registered in advance. They take the form of, _RWR_EQ__<x>__oo__<y>, note comments below.

5. todo

cpp_ext as separate project
make the try into tests
make EXISTS true, leave an empty element as false

IS_LIST to distinguish element from list, though it will be tough to
detect IS_LIST on a list with EMPTY elements,  then a list with all
empty elements is still a list, still exits.

mv BOOL AND OR 
make AND and OR versions of FIND on EXISTS  (existence quantification)


*/

#ifndef CPP_EXT_0
#define CPP_EXT_0

/*===========================================================================
DEBUG
===========================================================================*/

#include <stdio.h>
#define DEBUG_CPP
#define STR(...) #__VA_ARGS__

// print the macro and the evaluation of the macro
#define SHOW(expr) printf("%s -> %s\n", #expr, STR(expr))

/*===========================================================================
Constants
===========================================================================*/

#define COMMA ,
#define SEMICOLON ;

#define ZERO 0
#define ONE  1

#define FALSE 0
#define TRUE  1

//---------

#define _TWION_0 ~,0
#define _TWION_1 ~,1

// RWR == rewrite rule, RWR_ is followed by macro name it is used in `_<name>_`
// Matching text is replaced with nothing, making it empty
// Potential for aliasing if x or y values have '__oo__' embedded in them.
#define _RWR_NOT__0 
#define _RWR_AND__1__oo__1
#define _RWR_OR__0__oo__0

// add more of this form to register other equivalences
#define _RWR_EQ__0__oo__0
#define _RWR_EQ__1__oo__1

/*===========================================================================
Primitive Concatenation
  Not due to elegance, as `##` is convenient, rather this is 
  used to force evaluation of arguments before `##`.
  There will be a recursive CAT of n things in cpp_ext_1.c
===========================================================================*/

#define _CAT2(a ,b) a ## b
#define CAT2(a ,b) _CAT2(a ,b)

#define _CAT3(a ,b ,c) a ## b ## c
#define CAT3(a ,b ,c) _CAT3(a ,b ,c)

#define _CAT4(a ,b ,c ,d) a ## b ## c ## d
#define CAT4(a ,b ,c ,d) _CAT4(a ,b ,c ,d)

#define APPEND(list ,...) list ,__VA_ARGS__



/*===========================================================================
Existence
===========================================================================*/

  //----------------------------------------
  // primitive access

  // note: _FIRST of nothing, _FIRST(), means passing an empty_item as the first item
  // so it will return empty.
  #define _FIRST(a ,...) a
  #define _SECOND(a ,b ,...) b

  //----------------------------------------
  // existence
  //
  // `##` prevents rewrite of _TWION_ in the _NOT_EMPTY_ITEM_1 macro, don't
  // replace that with CAT!

  #define _NOT_EMPTY_ITEM_2(x_item) _SECOND(x_item ,1) 
  #define _NOT_EMPTY_ITEM_1(x_item) _NOT_EMPTY_ITEM_2(_TWION_0##x_item)

  #define NOT_EMPTY_ITEM(x_item)   _NOT_EMPTY_ITEM_1(x_item)
  #define NOT_EMPTY(...) NOT_EMPTY_ITEM( _FIRST(__VA_ARGS__) )

  #define _EMPTY_ITEM_2(x_item) _SECOND(x_item ,0) 
  #define _EMPTY_ITEM_1(x_item) _EMPTY_ITEM_2(_TWION_1##x_item)

  #define EMPTY_ITEM(x_item)   _EMPTY_ITEM_1(x_item)
  #define EMPTY(...) EMPTY_ITEM( _FIRST(__VA_ARGS__) )

  // useful to use with rewrite rules that substitute to nothing
  #define MATCH_RWR(x_item) EMPTY(x_item)
  #define NOT_MATCH_RWR(x_item) NOT_EMPTY(x_item)

/*===========================================================================
Logic Connectors
===========================================================================*/

  #define _NOT(x_item) \
    MATCH_RWR( CAT2(_RWR_NOT__ ,x_item) )

  #define _AND(x_item ,y_item) \
    MATCH_RWR( CAT4(_RWR_AND__ ,x_item ,__oo__ ,y_item) )

  #define _OR(x_item ,y_item) \
    NOT_MATCH_RWR( CAT4(_RWR_OR__ ,x_item ,__oo__ ,y_item) )

  #define _BOOL(x_item) \
    _AND( \
       NOT_EMPTY_ITEM( x_item ) \
      ,NOT_MATCH_RWR( CAT2(_RWR_EQ__0__oo__ ,x_item) )  \
    )
  #define BOOL(x_item) _BOOL(_FIRST(x_item))
  
  #define NOT(x_item)         _NOT(BOOL(x_item))
  #define AND(x_item ,y_item) _AND(BOOL(x_item) ,BOOL(y_item))
  #define  OR(x_item ,y_item)  _OR(BOOL(x_item) ,BOOL(y_item))

/*===========================================================================
  Equality

  more general than a connector because more rules can be added.

  each registered equality rule has the form
     _RWR_EQ__<x>__oo__<y>
  for example, logic equalities are already registered:
     _RWR_EQ__0__oo__0
     _RWR_EQ__1__oo__1

===========================================================================*/

  #define EQ(x_item ,y_item) \
        MATCH_RWR( CAT4(_RWR_EQ__ ,x_item ,__oo__ ,y_item) )

  #define NOT_EQ(x_item ,y_item) \
    NOT_MATCH_RWR( CAT4(_RWR_EQ__ ,x_item ,__oo__ ,y_item) )

/*===========================================================================
  IF-ELSE construct.
  Usage: IF_ELSE(condition)(<true case>)(<false case>)

  A most amazing little macro. It has no dependencies on the other macros
  in this file, though many will be useful for setting (condition)

  The seemingly extra layer prevents BOOL_(condition) from being pasted with a ## which, if done, would prevent it from being evaluated.  Recall, the first step in evaluation is a literal copy in of the arguments.  ===symbol ========================================================================*/

  #define IF(predicate)  CAT2(_IF_ ,BOOL(predicate))
  #define _IF_1(...)          __VA_ARGS__ _IF_1_ELSE
  #define _IF_0(...)                      _IF_0_ELSE
  #define _IF_1_ELSE(...)
  #define _IF_0_ELSE(...)      __VA_ARGS__

/*===========================================================================
Access
    see ext_1 with recursion for `Nth`

===========================================================================*/

  // _FIRST defined in the logic section
  #define FIRST(pad ,...)\
    IF \
      ( EMPTY(__VA_ARGS__) ) \
      (pad)                       \
      ( _FIRST(__VA_ARGS__) )  

  #define _REST(a ,...) __VA_ARGS__
  #define REST(...)\
    IF \
      ( EMPTY(__VA_ARGS__) ) \
      ()                          \
      ( _REST(__VA_ARGS__) )  

  // _SECOND defined in the logic section
  #define SECOND(pad ,...) \
    IF \
      ( EMPTY(__VA_ARGS__) ) \
      (pad)                       \
      ( _SECOND(__VA_ARGS__ ,pad) )  

  #define _THIRD(a ,b ,c ,...) c
  #define THIRD(pad ,...) \
    IF \
      ( EMPTY(__VA_ARGS__) ) \
      (pad)                       \
      ( _THIRD(__VA_ARGS__ ,pad, pad) )  

#endif
