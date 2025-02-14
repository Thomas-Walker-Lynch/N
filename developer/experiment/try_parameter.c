#include <stdio.h>

#define N32· N96·

int N96·i = 10;
int N32·j = 20;

int main(){
  printf("N96·i :%x\n",N96·i);
  printf("N32·j :%x\n",N96·j);
  return 0;
}

/*

  try_parameter.c:10:24: error: ‘N96·j’ undeclared (first use in this function); did you mean ‘N96·i’?
   10 |   printf("N32·j :%x\n",N96·j);
      |                        ^~~~~
      |                        N96·i

As of C99 a macro must have a space after it, so the `N32·` of `N32·j` is not being recognized.


*/
