#include<stdio.h>
#include<stdint.h>

#define X(x) x

#define _S(x) #x
#define S(x) _S(x)

#define _Ξ(a ,b) a##·##b
#define Ξ(a ,b) _Ξ(a ,b)

int main(){

  #if X(10)
    printf("goodness 10\n");
  #endif

  #define T S(abc)
  printf("T:%s \n" ,T);

  #define U S(Ξ(FG·FACE ,FG·Type))
  printf("U:%s \n" ,U);

  #define FG·Type uint32_t
  #define V S(Ξ(FG·FACE ,FG·Type))
  printf("V:%s \n" ,V);


  //#ifndef Ξ(FG·FACE ,FG·Type)
  // macro_guards.c: In function ‘main’:
  // macro_guards.c:29:12: warning: extra tokens at end of #ifndef directive
  //   29 |   #ifndef Ξ(FG·FACE ,FG·Type)
  //      |            ^

#if 0
  #ifndef Ξ(FG·FACE ,FG·Type)
    printf("goodness 32\n");
    #define Ξ(FG·FACE ,FG·Type)
  #else
    printf("badness 32\n");
  #endif

  #undef FG·Type
  #define FG·Type uint64_t
  #ifndef Ξ(FG·FACE ,FG·Type)
    printf("goodness 64\n");
    #define Ξ(FG·FACE ,FG·Type)
  #else
    printf("badness 64\n");
  #endif
#endif

}

/*

  Cannot expand a cpp macro to create the name of a cpp macro

> ./a.out
goodness 10
T:abc 
U:FG·FACE·FG·Type 
V:FG·FACE·uint32_t 


*/
