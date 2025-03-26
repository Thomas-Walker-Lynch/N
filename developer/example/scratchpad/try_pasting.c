#include <stdio.h>

#define STR(x) #x
#define XSTR(x) STR(x)

#define mess(...) rewrite_##__VA_ARGS__##_rule

int main(){

  printf("a mess %s\n" ,XSTR(mess(1)));
  printf("a mess %s\n" ,XSTR(mess(2)));
  printf("a mess %s\n" ,XSTR(mess(1,2)));

}

/*

  It does not like it when I send 2 parameters into mess.

  > gcc try_pasting.c 
try_pasting.c: In function ‘main’:
try_pasting.c:12:1: error: macro "STR" passed 2 arguments, but takes just 1
   12 |   printf("a mess %s\n" ,XSTR(mess(1,2)));
      | ^ ~~~~
try_pasting.c:3:9: note: macro "STR" defined here
    3 | #define STR(x) #x
      |         ^~~
try_pasting.c:4:17: error: ‘STR’ undeclared (first use in this function)
    4 | #define XSTR(x) STR(x)
      |                 ^~~
try_pasting.c:12:25: note: in expansion of macro ‘XSTR’
   12 |   printf("a mess %s\n" ,XSTR(mess(1,2)));
      |                         ^~~~
try_pasting.c:4:17: note: each undeclared identifier is reported only once for each function it appears in
    4 | #define XSTR(x) STR(x)
      |                 ^~~
try_pasting.c:12:25: note: in expansion of macro ‘XSTR’
   12 |   printf("a mess %s\n" ,XSTR(mess(1,2)));
      |                         ^~~~

*./
