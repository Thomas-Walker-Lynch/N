#include <stdlib.h>
#include <stdio.h>

typedef void *Incognito;

int f(Incognito x){ return 0;}

int g(){
  void *x = NULL;
 return f(x); // type mismatch correct?
}

int main(){
  printf("All is good.");
  return 0;
}

/*
There is no type mismatch error as hoped.


2025-02-19T06:56:21Z[developer]
Thomas-developer@Stanley§/home/Thomas-masu/developer/N/developer/experiment§
> gcc encapsulated_array.c 

2025-02-19T06:56:39Z[developer]
Thomas-developer@Stanley§/home/Thomas-masu/developer/N/developer/experiment§
> ./a.out
All is good.
2025-02-19T06:56:43Z[developer]

*/
