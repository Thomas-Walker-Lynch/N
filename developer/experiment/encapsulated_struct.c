#include <stdlib.h>
#include <stdio.h>

typedef struct Incognito Incognito;

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
This works when the encapsulated_array did not. We never even defined Incognito, where
as the Incognito array had a definition.


2025-02-19T06:57:34Z[developer]
Thomas-developer@Stanley§/home/Thomas-masu/developer/N/developer/experiment§
> gcc encapsulated_struct.c 
encapsulated_struct.c:6:17: error: parameter 1 (‘x’) has incomplete type
    6 | int f(Incognito x){ return 0;}
      |       ~~~~~~~~~~^


*/
