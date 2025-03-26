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
  Defer macros: these help “hide” recursive calls for additional expansion passes.
===========================================================================*/

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
