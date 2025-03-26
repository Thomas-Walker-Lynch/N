/*===========================================================================
Constants
===========================================================================*/

#define COMMA ,
#define SEMICOLON ;

#define _REWRITE__·FALSE 
#define _REWRITE__·TRUE__EQ__·TRUE
#define _REWRITE__·FALSE__EQ__·FALSE


#define rewrite_rule ·FALSE
#define rewrite_  be_true(
#define _rule    )
#define be_true(...) ·TRUE

/*===========================================================================
Access
===========================================================================*/

#define FIRST_1(a ,...) a
#define FIRST(pad ,...) FIRST_1(__VA_ARGS__ ,pad)

#define SECOND_1(a ,b ,...) b
#define SECOND(pad ,...) SECOND_1(__VA_ARGS__ ,pad)

#define THIRD_1(a ,b ,c,...) c
#define THIRD(pad ,...) THIRD_1(__VA_ARGS__ ,pad ,pad)


/*===========================================================================
  Defer macros: these help “hide” recursive calls for additional expansion passes.
===========================================================================*/
#define EMPTY()
#define DEFER1(m)            m EMPTY()
#define DEFER2(m)            m EMPTY EMPTY()()
#define DEFER3(m)            m EMPTY EMPTY EMPTY()()()
#define DEFER4(m)            m EMPTY EMPTY EMPTY EMPTY()()()  /* as needed */

/*===========================================================================
  Two Token concatenation, with optional separator
===========================================================================*/

#define CAT2(sep,a,b) a##sep##b


/*===========================================================================
LOGIC
===========================================================================*/

/* 
  This is deterministic, and free from possible aliasing with the arguments.
  It does not require that ·TRUE or ·FALSE not be in the data.

  No Default value is needed for SECOND, as there will always be at least two
  tokens in `x ,·TRUE`

  1. When __VA_ARGS__ is empty, FIRST returns the default value of `<empty>,·FALSE` for
     `x`. This makes x into a two token value, and the argument list for SECOND will be:

     `<empty> ,·FALSE ,·TRUE.
   
     So the SECOND value is ·FALSE.

  2. When __VA_ARGS__ is not empty, it sends a single value of x, so the argument list
     for SECOND will be:

     x ,·TRUE.

     So the second value will be ·TRUE
*/






#define PAIR() ~,·FALSE

#define EXIST_1(x) FIRST(~ ,x ,·TRUE)
#define EXIST(...) EXIST_1( \
  FIRST( \
    DEFER3(PAIR)()                         \
    ,__VA_ARGS__ \
  ))

#define NOT_EXIST_1(x) SECOND(,x ,·FALSE)
#define NOT_EXIST(...) NOT_EXIST_1( \
  FIRST( \
    DEFER1(CAT2)(,·TRUE) \
    ,__VA_ARGS__ \
  ))

// `NOT_1` Returns a ·TRUE, or ·FALSE
#define NOT_1(x) NOT_EXIST(_REWRITE__##x)
#define NOT(x) NOT_1(x)

#define BOOL_1(x) EXIST(_REWRITE__##x)
#define BOOL(x) BOOL_1(x)

#define EQUAL_1(x ,y) NOT_EXIST_1( _REWRITE__##x__EQ__##y )
#define EQUAL(x ,y) EQUAL_1(x ,y)


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


