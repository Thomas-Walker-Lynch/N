#include <stdio.h>

#define _Ξ(x,y) x ## y
#define Ξ(x,y) _Ξ(x,y)

//#define CONCAT(x,y) x ## y

int main() {
  Ξ(in ,t) z = 5;
  printf("%x\n", z);  // Fixed missing comma
  return 0;
}

/*
2025-03-18T07:34:43Z[developer]
Thomas-developer@Stanley§/home/Thomas-masu/developer/N/developer/experiment§
> gcc concat.c 

2025-03-18T07:39:42Z[developer]
Thomas-developer@Stanley§/home/Thomas-masu/developer/N/developer/experiment§
> ./a.out
5

2025-03-18T07:39:46Z[developer]
*/
