
#ifndef CPP_EXT
#define CPP_EXT

 #include "cpp_ext_0.c"
 #include "cpp_ext_1.c"

  /*
    Types must be registered with predicates.

    // all legal FG·Type are registered here
    #define EQ__int__oo__int
    #define EQ__float__oo__float
    #define EQ__char__oo__char
    #define EQ__void__oo__void

  */
 
  #define FIRST_SEEN(type ,list) \
    NOT(FIND_ITEM(type ,list)) )

 // CAT(SEP, ...) will soon be added to cpp_ext_1, then this will be:
 // #define Ξ(...)  CAT(· ,__VA_ARGS__)

  // Individual macros for specific argument counts
  #define _Ξ0() 
  #define Ξ0() _Ξ0()

  #define _Ξ1(a) a
  #define Ξ1(a) _Ξ1(a)

  #define _Ξ2(a ,b) a##·##b
  #define Ξ2(a ,b) _Ξ2(a ,b)

  #define _Ξ3(a ,b ,c) a##·##b##·##c
  #define Ξ3(a ,b ,c) _Ξ3(a ,b ,c)

  #define _Ξ4(a ,b ,c ,d) a##·##b##·##c##·##d
  #define Ξ4(a ,b ,c ,d) _Ξ4(a ,b ,c ,d)

  #define _Ξ5(a ,b ,c ,d ,e) a##·##b##·##c##·##d##·##e
  #define Ξ5(a ,b ,c ,d ,e) _Ξ5(a ,b ,c ,d ,e)

  #define _Ξ6(a ,b ,c ,d ,e ,f) a##·##b##·##c##·##d##·##e##·##f
  #define Ξ6(a ,b ,c ,d ,e ,f) _Ξ6(a ,b ,c ,d ,e ,f)

  #define _Ξ7(a ,b ,c ,d ,e ,f ,g) a##·##b##·##c##·##d##·##e##·##f##·##g
  #define Ξ7(a ,b ,c ,d ,e ,f ,g) _Ξ7(a ,b ,c ,d ,e ,f ,g)

  #define _Ξ8(a ,b ,c ,d ,e ,f ,g ,h) a##·##b##·##c##·##d##·##e##·##f##·##g##·##h
  #define Ξ8(a ,b ,c ,d ,e ,f ,g ,h) _Ξ8(a ,b ,c ,d ,e ,f ,g ,h)

  #define _Ξ9(a ,b ,c ,d ,e ,f ,g ,h ,i) a##·##b##·##c##·##d##·##e##·##f##·##g##·##h##·##i
  #define Ξ9(a ,b ,c ,d ,e ,f ,g ,h ,i) _Ξ9(a ,b ,c ,d ,e ,f ,g ,h ,i)

  #define _Ξ10(a ,b ,c ,d ,e ,f ,g ,h ,i ,j) a##·##b##·##c##·##d##·##e##·##f##·##g##·##h##·##i##·##j
  #define Ξ10(a ,b ,c ,d ,e ,f ,g ,h ,i ,j) _Ξ10(a ,b ,c ,d ,e ,f ,g ,h ,i ,j)

  // Argument counting mechanism
  #define _ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
  #define COUNT_ARGS(...) _ARG_N(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

  // Macro name concatenation
  #define _CONCAT(a, b) a##b
  #define CONCAT(a, b) _CONCAT(a, b)

  // Selector that chooses the right macro based on argument count
  #define Ξ_EXPAND(count, ...) CONCAT(Ξ, count)(__VA_ARGS__)
  #define Ξ(...) Ξ_EXPAND(COUNT_ARGS(__VA_ARGS__), __VA_ARGS__)


#endif 
