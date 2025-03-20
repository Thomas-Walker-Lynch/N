#include<stdio.h>
#include<stdint.h>

int main(){

#define AU uint8_t
#define CVT AU

#if CVT == uint16_t
  printf("CVT == uint16_t\n");
#else
  printf("CVT !=  uint16_t\n");
#endif

#if CVT != uint16_t
  printf("CVT != uint16_t\n");
#else
  printf("CVT == uint16_t\n");
#endif

#if CVT == AU
  printf("CVT == AU\n");
#else
  printf("CVT != AU\n");
#endif

#if CVT != AU
  printf("CVT != AU\n");
#else
  printf("CVT == AU\n");
#endif

#if CVT == uint8_t
  printf("CVT == uint8_t\n");
#else
  printf("CVT !=  uint8_t\n");
#endif

#if CVT != uint8_t
  printf("CVT != uint8_t\n");
#else
  printf("CVT == uint8_t\n");
#endif

}

/*
  The macros is evaluated for its numeric value, so this does not work.

> gcc macro_equality.c 

> ./a.out
CVT == uint16_t
CVT == uint16_t
CVT == AU
CVT == AU
CVT == uint8_t
CVT == uint8_t

*/
