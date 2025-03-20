#include <stdio.h>

#include "xi.c"

struct a{
  int x;
} Ξ(X) ,Ξ(Y ,Y) ,Ξ(Z ,Z ,Z);


int main(){

  X.x = 3;
  Y·Y.x = 5;
  Z·Z·Z.x = 7;

  Ξ(); // should do nothing, as it expands to nothing.

  printf("%x" ,X.x); // notice no cdot, just 'X'
  printf("%x" ,Y·Y.x);
  printf("%x" ,Z·Z·Z.x);

}


/*
2025-03-20T08:20:34Z[developer]
Thomas-developer@Stanley§/home/Thomas-masu/developer/N/developer/experiment§
> gcc xi_test.c
In file included from xi_test.c:3:
xi.c:17:2: warning: #warning "beware all those who traverse here" [-Wcpp]
   17 | #warning "beware all those who traverse here"
      |  ^~~~~~~

2025-03-20T08:21:28Z[developer]
Thomas-developer@Stanley§/home/Thomas-masu/developer/N/developer/experiment§
> ./a.out
357
2025-03-20T08:21:34Z[developer]
*/  
