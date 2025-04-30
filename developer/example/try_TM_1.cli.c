/*
  try_TM_1.cli.c - Example use of TM·AU type with array backend.
*/

#include <stdio.h>
#include "cpp_ext.c"
#include "TM·Array.lib.c"

#define _TM·CVT_ AU
#include "TM·Array.lib.c"
#define SET_Binding__TM·Array·AU

int main(){
  printf("running try_TM_1.cli.c on %s at %s\n", __DATE__, __TIME__);

  // Create a tape with 5 AU values
  AU tape[] = {1 ,2 ,3 ,4 ,5};
  extent_t·AU extent = sizeof(tape) / sizeof(AU) - 1;

  // Init the tableau
  TM·AU·Array·Tableau t;
  TM·AU tm = TM·AU·Array·init_pe(&t ,tape ,extent);

  if( Binding·call(tm ,on_tape) ){
    do{
      printf( "%02x" , (unsigned int) Binding·call(tm ,read) );
      if( Binding·call(tm ,on_rightmost) ) break;
      putchar(' ');
      Binding·call(tm ,step);
    }while(1);
    printf("\n");
  }

  return 0;
}

#define LOCAL
#define _TM·CVT_ AU
#include "TM·Array.lib.c"
#define SET_TM·Array·LOCAL__TM·AU
