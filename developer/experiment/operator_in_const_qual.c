
#include <stdio.h>

typedef enum{
  X_i=1
  ,X_j
} X;

int main(){
  int k0 = X_i | X_j;
  const int k1 = X_i | X_j;
  printf("k1: %x\n" ,k1);
}
