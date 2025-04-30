/*
  try_TM_3.cli.c - Tests repeat includes, and more TM functions
*/

#include <stdio.h>
#include "cpp_ext.c"
#include "TM.lib.c"

// Include the same type twice (should be gate-guarded)
#define _TM·CVT_ AU
#include "TM.lib.c"
#define SET_Binding__TM·AU

#define _TM·CVT_ Str
typedef char* Str;
#include "TM.lib.c"
#define SET_Binding__TM·Str

// Include LOCAL twice as well
#define LOCAL
#define _TM·CVT_ AU
#include "TM.lib.c"
#define SET_TM·LOCAL__TM·AU

#define LOCAL
#define _TM·CVT_ Str
#include "TM.lib.c"
#define SET_TM·LOCAL__TM·Str

int main(){
  printf("running try_TM_3.cli.c on %s at %s\n", __DATE__, __TIME__);

  // AU tape
  AU tape[] = {0xaa ,0x55 ,0xc2};
  TM·AU·Array·Tableau t_au;
  TM·AU tm_au = TM·AU·Array·init_pe(&t_au ,tape ,3);

  printf("Initial TM·AU tape:\n");
  if( Binding·call(tm_au ,on_tape) ){
    Binding·call(tm_au ,rewind);
    do{
      printf("%02x", (unsigned int) Binding·call(tm_au ,read));
      if( Binding·call(tm_au ,on_rightmost) ) break;
      putchar(' ');
      Binding·call(tm_au ,step);
    }while(1);
    printf("\n");
  }

  // Str tape
  Str tape2[] = {"hello" ,"world" ,"Calderis"};
  TM·Str·Array·Tableau t_str;
  TM·Str tm_str = TM·Str·Array·init_pe(&t_str ,tape2 ,3);

  printf("Initial TM·Str tape:\n");
  Binding·call(tm_str ,rewind);
  do{
    printf("\"%s\"", Binding·call(tm_str ,read));
    if( Binding·call(tm_str ,on_rightmost) ) break;
    putchar(' ');
    Binding·call(tm_str ,step);
  }while(1);
  printf("\n");

  // Test dismount + mount cycle
  Binding·call(tm_au ,dismount);
  Binding·call(tm_au ,mount);
  printf("After dismount/remount, AU read: %02x\n",
    (unsigned int) Binding·call(tm_au ,read));

  // Test step_left from position 1
  Binding·call(tm_au ,rewind);
  Binding·call(tm_au ,step);
  Binding·call(tm_au ,step_left);
  printf("After step to right and left again: %02x\n",
    (unsigned int) Binding·call(tm_au ,read));

  return 0;
}
