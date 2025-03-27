/*

Provides:

  COMMA, SEMICOLON, EXISTS, NOT_EXISTS, ,MATCH_RWR ,NOT_MATCH_RWR, BOOL, NOT, AND, OR, EQ, NOT_EQ, IF_ELSE

1.
  See also https://github.com/18sg/uSHET/blob/master/lib/cpp_magic.h 
  and tutorial at: http://jhnet.co.uk/articles/cpp_magic

  I could not have even thought about writing this without Jonathan Heathcote's
  little tutorial.

2.
  These are the non-recursive extensions.  See cpp_ext_1 for the recursive extensions.
 
3.
  The 'twion' is cute. It is reminiscent of a complex number in math. Like the complex number that is fed into scalar equestions.  The twion is a pair given to functions that only take singletons.

4.
  0 is false
  1 is true

  Macros starting with an '_' (underscore) are private.

  EQ comparisons apart from logic comparisons, must be registered in advance. They take the form of, _RWR_EQ__<x>__oo__<y>, note comments below.

5. Evaluation 

  1. cpp does not evaluate arguments that in the definition are attached to hash operator.

       #define STR(x) #x   // x is not evaluated.
       #define VAL(x) STR(x)  // x is evaluated recursively before STR is expanded

     The same applies for '##'

  2. cpp evaluates from left to right, for each macro it finds it does depth first revalution.

  3. Due to the left to right evaluation a funny things happens.

      #define EMPTY()
      #define NEGATE(x) -x
     
      #define NOT_SO_FAST(x)  NEGATE EMPTY() (x)

      What happens in left to right evaluation of NOT_SO_FAST(5)
      1. first NEGATE will be evaluated, there is nothing to expand
      2. EMPTY() is evaluated to nothing
      3. (x) is evaluated to 5.
      4. reached the right side, done result is:

      ->  NEGATE (x)

        > cat >test.c << EOF
        #include <stdio.h>
        int main(void){
          printf("example_eval.c\n");

          #define STR(x) #x

          // `#x` no evaluation of x 
          // `STR(x)` one left to right pass evaluation of `x` due to depth recursion
          #define SHOW(x) printf(#x " → %s\n", STR(x));

          #define EMPTY()
          #define NEGATE(x) -x
          #define NOT_SO_FAST(x)  NEGATE EMPTY() (x)
          SHOW(NOT_SO_FAST(5));

          #define BE(x) x
          SHOW(BE(NOT_SO_FAST(5)))
        }
        EOF

        > gcc test.c
        > ./a.out
        example_eval.c
        NOT_SO_FAST(5) → NEGATE (5)
        BE(NOT_SO_FAST(5)) → -5
        > 

    Consider the lines:

      #define BE(x) x
      SHOW(BE(NOT_SO_FAST(5)))
    
    cpp left to right scan finds one macro to expand: BE(NOT_SO_FAST(5))
    cpp depth first into that macro finds 5, which expands to -> 5
    going up a level cpp tries: NOT_SO_FAST(5) -> NEGATE (5)
    going up a level cpp tries: BE( NEGATE (5) ) ->  BE( -5 ) -> -5




*/

#ifndef CPP_EXT_0
#define CPP_EXT_0


/*===========================================================================
Constants
===========================================================================*/

#define COMMA ,
#define SEMICOLON ;

#define ZERO 0
#define ONE 1

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
Logic
===========================================================================*/

  //----------------------------------------
  // primitive access

  // note: _FIRST of nothing, _FIRST(), means passing an empty_item as the first item
  // so it will return empty.
  #define _FIRST(a ,...) a
  #define _SECOND(a ,b ,...) b

  //----------------------------------------
  // existence

  // `##` prevents rewrite of _TWION_ in the _EXISTS_ITEM_1 macro
  #define _EXISTS_ITEM_2(x_item) _SECOND(x_item ,1) 
  #define _EXISTS_ITEM_1(x_item) _EXISTS_ITEM_2(_TWION_0##x_item)

  #define EXISTS_ITEM(x_item)   _EXISTS_ITEM_1(x_item)
  #define EXISTS(...) EXISTS_ITEM( _FIRST(__VA_ARGS__) )

  #define _NOT_EXISTS_ITEM_2(x_item) _SECOND(x_item ,0) 
  #define _NOT_EXISTS_ITEM_1(x_item) _NOT_EXISTS_ITEM_2(_TWION_1##x_item)

  #define NOT_EXISTS_ITEM(x_item)   _NOT_EXISTS_ITEM_1(x_item)
  #define NOT_EXISTS(...) NOT_EXISTS_ITEM( _FIRST(__VA_ARGS__) )

  // useful to use with rewrite rules that substitute to nothing
  #define MATCH_RWR(x_item) NOT_EXISTS(x_item)
  #define NOT_MATCH_RWR(x_item) EXISTS(x_item)

/*===========================================================================
Logic Connectors
===========================================================================*/

  #define _NOT_1(x_item) MATCH_RWR( _RWR_NOT__##x_item )
  #define _NOT(x_item) _NOT_1(x_item)

  #define _AND_1(x_item ,y_item) MATCH_RWR( _RWR_AND__##x_item##__oo__##y_item )
  #define _AND(x_item ,y_item) _AND_1(x_item ,y_item)

  #define _OR_1(x_item ,y_item) NOT_MATCH_RWR( _RWR_OR__##x_item##__oo__##y_item )
  #define _OR(x_item ,y_item) _OR_1(x_item ,y_item)

  #define _BOOL_2(x_item) \
    _AND(\
       EXISTS_ITEM( _FIRST(x_item) )            \
      ,NOT_MATCH_RWR( _RWR_EQ__0__oo__##x_item) \
    )
  #define _BOOL_1(x_item) _BOOL_2(x_item)
  #define BOOL(x_item) _BOOL_1(_FIRST(x_item))

  #define NOT(x_item) _NOT(BOOL(x_item))
  #define AND(x_item ,y_item) _AND(BOOL(x_item) ,BOOL(y_item))
  #define OR(x_item ,y_item) _OR(BOOL(x_item) ,BOOL(y_item))

/*===========================================================================
  Equality

  more general than a connector because more rules can be added.

  each registered equality rule has the form
     _RWR_EQ__<x>__oo__<y>
  for example, logic equalities are already registered:
     _RWR_EQ__0__oo__0
     _RWR_EQ__1__oo__1

===========================================================================*/

  #define _EQ(x_item ,y_item) MATCH_RWR( _RWR_EQ__##x_item##__oo__##y_item )
  #define EQ(x_item ,y_item) _EQ(x_item ,y_item)

  #define _NOT_EQ(x_item ,y_item) EXISTS(_RWR_EQ__##x_item##__oo__##y_item)
  #define NOT_EQ(x_item ,y_item) _NOT_EQ(x_item ,y_item)


/*===========================================================================
  IF-ELSE construct.
  Usage: IF_ELSE(condition)(<true case>)(<false case>)

  A most amazing little macro. It has no dependencies on the other macros
  in this file, though many will be useful for setting (condition)
===========================================================================*/

  #define IF_ELSE(condition) _IF_ELSE(BOOL(condition))
  #define _IF_ELSE(condition)  _IF_##condition
  #define _IF_1(...)          __VA_ARGS__ _IF_1_ELSE
  #define _IF_0(...)                      _IF_0_ELSE
  #define _IF_1_ELSE(...)
  #define _IF_0_ELSE(...)      __VA_ARGS__

/*===========================================================================
Access
    see below the recursion section for Nth .. when it is written ;-)

===========================================================================*/

  // _FIRST defined in the logic section
  #define FIRST(pad ,...)\
    If_ELSE \
      ( NOT_EXISTS(__VA_ARGS__) ) \
      (pad)                       \
      ( _FIRST(__VA_ARGS__) )  

  #define _REST(a ,...) __VA_ARGS__
  #define REST(...)\
    If_ELSE \
      ( NOT_EXISTS(__VA_ARGS__) ) \
      ()                          \
      ( _REST(__VA_ARGS__) )  

  // _SECOND defined in the logic section
  #define SECOND(pad ,...) \
    If_ELSE \
      ( NOT_EXISTS(__VA_ARGS__) ) \
      (pad)                       \
      ( _SECOND(__VA_ARGS__ ,pad) )  

  #define _THIRD(a ,b ,c ,...) c
  #define THIRD(pad ,...) \
    If_ELSE \
      ( NOT_EXISTS(__VA_ARGS__) ) \
      (pad)                       \
      ( _THIRD(__VA_ARGS__ ,pad, pad) )  



#endif
