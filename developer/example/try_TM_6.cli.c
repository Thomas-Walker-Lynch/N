/*
  try_TM_6.cli.c — SingletonCircle test with TM·AU
*/

#include <stdio.h>
#include "cpp_ext.c"
#include "TM.lib.c"

#define _TM·CVT_ AU
#include "TM.lib.c"
#define SET__Binding__TM·AU

int main(){

  printf("running try_TM_6.cli.c on %s at %s\n", __DATE__, __TIME__);

  // Init SingletonCircle tape with initial value 0x3F
  TM·AU·SingletonCircle·Tableau t;
  TM·AU tm = TM·AU·SingletonCircle·init(&t ,0x3F);

  printf("Initial value: %02x\n", (unsigned int) Binding·call(tm ,read));

  // Overwrite the value
  AU new_val = 0xA5;
  Binding·call(tm ,write ,&new_val);

  // Step right  (should be no-op)
  Binding·call(tm ,step);

  // Confirm value persists
  printf("After stepping, value: %02x\n", (unsigned int) Binding·call(tm ,read));

  // Dismount and try to rewind (noop)
  Binding·call(tm ,dismount);
  Binding·call(tm ,rewind); // should do nothing

  // Mount again and verify value
  Binding·call(tm ,mount);
  printf("After remount, value: %02x\n", (unsigned int) Binding·call(tm ,read));

  return 0;
}

#define LOCAL
#define _TM·CVT_ AU
#include "TM.lib.c"
#define SET__TM·LOCAL__TM·AU
