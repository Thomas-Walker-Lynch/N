#include <stdio.h>

// Helper to stringify a macro argument
#define STR(x) #x
#define SHOW(x) STR(x)

#define BAD(x ,y) x + y
#assign (Z ,12)
//#xassign (Z ,12)  // invalid directive

// Macros to test argument parsing and expansion
#define ZERO()         SHOW(ZERO())
#define ONE(x)         SHOW(ONE(x))
#define TWO(x ,y)      SHOW(TWO(x ,y))
#define VAR(...)       SHOW(VAR(__VA_ARGS__))
#define MIXED(x ,...)  SHOW(MIXED(x ,__VA_ARGS__))

int main() {
  int x = BAD(1 ,2);   // should work
  printf("x:%x\n" ,x);

  // int y = BAD(1 ,2 ,3);   // Too many args
  printf("%s\n" ,ZERO());                   // "ZERO()"
  printf("%s\n" ,ONE(42));                  // "ONE(42)"
  printf("%s\n" ,TWO(a ,b));                // "TWO(a ,b)"
  printf("%s\n" ,VAR());                    // "VAR()"
  printf("%s\n" ,VAR(1));                   // "VAR(1)"
  printf("%s\n" ,VAR(1 ,2 ,3));             // "VAR(1 ,2 ,3)"
  printf("%s\n" ,MIXED(0));                 // "MIXED(0)"
  printf("%s\n" ,MIXED(x ,y ,z));           // "MIXED(x ,y ,z)"
  printf("%s\n" ,ONE((1 ,2)));              // "ONE((1 ,2))"
  printf("%s\n" ,VAR((a ,b) ,c));           // "VAR((a ,b) ,c)"
  return 0;
}
