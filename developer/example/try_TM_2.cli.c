/*
  try_TM_2.cli.c - Demonstrates two coexisting TM types: TM·AU and TM·str.
*/

#include <stdio.h>
#include "cpp_ext.c"
#include "TM.lib.c"

#define _TM·CVT_ AU
#include "TM.lib.c"
#define SET__Binding__TM·AU

typedef char* Str;
#define _TM·CVT_ Str
#include "TM.lib.c"
#define SET__Binding__TM·Str

int main(){
  printf("running try_TM_2.cli.c on %s at %s\n", __DATE__, __TIME__);

  // TM·AU example
  AU tape1[] = {0xAA ,0x55 ,0xC2};
  extent_t·AU extent1 = sizeof(tape1) / sizeof(AU) - 1;

  TM·AU·Array·Tableau t1;
  TM·AU tm1 = TM·AU·Array·init_pe(&t1 ,tape1 ,extent1);

  if( Binding·call(tm1 ,on_tape) ){
    printf("TM·AU tape: ");
    do{
      printf("%02x", (unsigned int) Binding·call(tm1 ,read));
      if( Binding·call(tm1 ,on_rightmost) ) break;
      putchar(' ');
      Binding·call(tm1 ,step);
    }while(1);
    printf("\n");
  }

  // TM·Str example
  Str tape2[] = {"hello", "world", "Calderis"};
  extent_t·Str extent2 = sizeof(tape2) / sizeof(Str) - 1;

  TM·Str·Array·Tableau t2;
  TM·Str tm2 = TM·Str·Array·init_pe(&t2 ,tape2 ,extent2);

  if( Binding·call(tm2 ,on_tape) ){
    printf("TM·Str tape: ");
    do{
      printf("\"%s\"", Binding·call(tm2 ,read));
      if( Binding·call(tm2 ,on_rightmost) ) break;
      putchar(' ');
      Binding·call(tm2 ,step);
    }while(1);
    printf("\n");
  }

  return 0;
}

// Bring in local implementations
#define LOCAL

#define _TM·CVT_ AU
#include "TM.lib.c"
#define SET__TM·LOCAL__TM·AU

#define _TM·CVT_ Str
#include "TM.lib.c"
#define SET__TM·LOCAL__TM·Str

