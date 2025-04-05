/*
  try_TM_5.cli.c - Demonstrate dismount, ignored rewind, remount,
                  mix of reads/writes, and verification.
*/

#include <stdio.h>
#include "cpp_ext.c"
#include "TM.lib.c"

#define _TM·CVT_ AU
#include "TM.lib.c"
#define SET__Binding__TM·AU

int main(){
  printf("running try_TM_5.cli.c on %s at %s\n", __DATE__, __TIME__);

  // Start with 4 AU values
  AU tape[] = { 0xAA ,0x55 ,0xC2 ,0x01 };
  extent_t·AU extent = sizeof(tape) / sizeof(AU) - 1;

  // Init the tableau and TM binding
  TM·AU·Array·Tableau t;
  TM·AU tm = TM·AU·Array·init_pe(&t ,tape ,extent);

  // Dismount tape
  Binding·call(tm ,dismount);

  // This rewind will do nothing, since tape is dismounted
  Binding·call(tm ,rewind);

  // Mount again (should put head at position[0])
  Binding·call(tm ,mount);

  // Overwrite tape with incrementing values starting at 0x10
  AU value = 0x10;
  if( Binding·call(tm ,on_tape) ){
    do{
      Binding·call(tm ,write ,&value);
      value++;
      if( Binding·call(tm ,on_rightmost) ) break;
      Binding·call(tm ,step);
    }while(1);
  }

  // Rewind and verify written values
  Binding·call(tm ,rewind);

  printf("Verifying written tape:\n");
  if( Binding·call(tm ,on_tape) ){
    do{
      AU x = Binding·call(tm ,read);
      printf("%02x", (unsigned int)x);
      if( Binding·call(tm ,on_rightmost) ) break;
      putchar(' ');
      Binding·call(tm ,step);
    }while(1);
    putchar('\n');
  }

  return 0;
}

#define LOCAL
#define _TM·CVT_ AU
#include "TM.lib.c"
#define SET__TM·LOCAL__TM·AU
