/* macro_lib.c */
#ifndef MACRO_LIB
#define MACRO_LIB

/*===========================================================================
 Some Basics
===========================================================================*/

#define COMMA ,

#define _FIRST(a,...) a
#define FIRST(...) _FIRST(__VA_ARGS__,~)

#define _SECOND(a,b,...) b
#define SECOND(...) _SECOND(__VA_ARGS__,~,~)


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
  Token concatenation and basic utilities
===========================================================================*/

/* Assumes that EVAL,DEFER1,IF_ELSE,and HAS_ARGS are defined as in our macro library */

/* Helper: paste exactly two tokens with a separator.
   If 'sep' is empty,then a ## sep ## b is equivalent to a ## b.
*/
#define _CAT2(sep,a,b) a ## sep ## b

#define _CAT(sep,first,...)                                \
    IF_ELSE(HAS_ARGS(__VA_ARGS__))(                         \
         _CAT2(sep,first,DEFER1(_CAT)(sep,__VA_ARGS__))  \
    )(first)

/* Variadic CAT:
   - If no tokens are provided,returns nothing.
   - If one token is provided,returns that token.
   - Otherwise,it recursively concatenates the tokens with the given separator.
*/
#define CAT(sep,...) IF_ELSE(HAS_ARGS(__VA_ARGS__))( EVAL(_CAT(sep,__VA_ARGS__)) )(/* nothing */)

#define Ξ(...) CAT(·,__VA_ARGS__)
#define Append(...) CAT(COMMA,__VA_ARGS__)


/*===========================================================================
  Put a 'PROBE()' in a list then detect it later.

  If a single value is given to IS_PROBE it won't have a second member,but
  if the two element PROBE is given,the second element will be 1.

===========================================================================*/

#define PROBE() ~,1

#define __IS_PROBE(a,b,...) b
#define _IS_PROBE(...) __IS_PROBE(__VA_ARGS__,0,0)
#define IS_PROBE(...) _IS_PROBE(__VA_ARGS__)

#define _NOT_0 PROBE()

//  x == 0:  _NOT_##x -> _NOT_0 -> ~,1 -> 1
//  x == 1:  _NOT_##x -> _NOT_1 -> 0
//
#define _NOT(x)  IS_PROBE(_NOT_##x)
#define NOT(x)   _NOT(x)              
#define BOOL(x)  NOT(NOT(x))

// This is a rewrite rule,if there is a definition for _IS_EQ_<x>_<y> it is replaced by its #define definition,presumably that of a Probe.
// The rule will then be  #define _IS_EQ_<X>_<X> Probe()
#define _EQUAL_REWRITE(x,y) IS_PROBE( CAT(_IS_EQ_,x,_##y) )

#define EQUAL(x,y) BOOL( _EQUAL_REWRITE(x,y) )


/*===========================================================================
  IF-ELSE construct.
  Usage: IF_ELSE(condition)(<true case>)(<false case>)
===========================================================================*/
#define IF_ELSE(condition)   _IF_ELSE(BOOL(condition))
#define _IF_ELSE(condition)  CAT(_IF_,condition)
#define _IF_1(...)          __VA_ARGS__ _IF_1_ELSE
#define _IF_0(...)                      _IF_0_ELSE
#define _IF_1_ELSE(...)
#define _IF_0_ELSE(...)      __VA_ARGS__

/*===========================================================================
  HAS_ARGS: tests whether a variadic argument list is empty.

  A bit of a tricky one, _END_OF_ARGUMENTS will only be expanded if it
  is followed by parenthesis,otherwise it is taken literally.
===========================================================================*/
#define _END_OF_ARGUMENTS_() 0
#define HAS_ARGS(...) BOOL(FIRST(_END_OF_ARGUMENTS_ __VA_ARGS__)())

/*===========================================================================
  MATCH(x,list) returns 1 if x is a member in the list, else 0

  list is comma separated.

  when list is passed to EVAL(MATCH_IMPL(x,list)) it gets expanded
  which causes the elements to spread out.

  #define list dog,cat,parakeet
  Match(x,list) -> EVAL(MATCH_IMPL(x,dog,cat,parkeet))

  HAS_ARGS will return true if there is a 'first' list element, else false.

  #define _IS_EQ_<x>_<x> Probe() must be defined for the equality test to work.

===========================================================================*/
#define MATCH_IMPL(x,first,...)               \
  IF_ELSE                                       \
    ( HAS_ARGS(first) )                          \
    ( IF_ELSE                                    \
       ( EQUAL(x,first) )                       \
       (1)                                       \
       ( MATCH_IMPL(x,__VA_ARGS__) )            \
       )                                         \
    (0)

#define MATCH(x,list) EVAL(MATCH_IMPL(x,list))

/* End of macro_lib.c */
#endif
