/*
  try_TM.cli.c - Example use of TM·AU type with array backend.
*/

#include "cpp_ext.c"
#include "Core.lib.c"
#include "Binding.lib.c"
#include "TM.lib.c"

#define TM·CVT AU
#include "TM.lib.c"
#define SET__Binding__TM·AU

int main(){
  printf("running try_TM.cli.c on %s at %s\n", __DATE__, __TIME__);

  SHOW(Ξ(TM·CVT ,FG));  // TM·AU·FG
  SHOW(Ξ(TM·CVT ,Tableau));  // TM·AU·Tableau

  // Create a tape with 5 AU values
  AU tape[] = {1 ,2 ,3 ,4 ,5};
  extent_t·AU extent = sizeof(tape) - 1;

  // Init the tableau
#if 0

  Ξ(TM ,AU)·Tableau t;
  Ξ(TM ,AU)·init_pe(&t ,tape ,extent);


  // Allocate a binding
  TM·AU tm;
  tm.fg = &Ξ(TM ,AU)·fg;
  tm.tableau = &t;

  // Call version of the TM status
  printf("Before any operations:\n");
  Binding·call(tm ,rewind);

  AU datum = Binding·call(tm ,read);
  printf("Initial datum: %u\n", datum);

  // Step and read again
  Binding·call(tm ,step);
  datum = Binding·call(tm ,read);
  printf("After step, datum: %u\n", datum);

  // Write a new value
  AU value = 99;
  Binding·call(tm ,write ,&value);

  // Rewind and read again
  Binding·call(tm ,rewind);
  datum = Binding·call(tm ,read);
  printf("After rewind, datum: %u\n", datum);

#endif
  return 0;
}

#define LOCAL
#include "Core.lib.c"
#include "Binding.lib.c"
#include "TM.lib.c"
