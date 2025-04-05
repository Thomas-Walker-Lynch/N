/*
  try_TM.cli.c - Example use of TM·AU type with array backend.
*/

#include "cpp_ext.c"
#include "TM.lib.c"

#define _TM·CVT_ AU
#include "TM.lib.c"
#define SET__Binding__TM·AU

int main(){
  printf("running try_TM.cli.c on %s at %s\n", __DATE__, __TIME__);

  // Create a tape with 5 AU values
  AU tape[] = {1 ,2 ,3 ,4 ,5};
  extent_t·AU extent = sizeof(tape) - 1;

  // Init the tableau
  TM·AU·Array·Tableau t;
  TM·AU tm = TM·AU·Array·init_pe(&t ,tape ,extent);

#if 0

  if( Binding·call(tm ,Head·on_tape) ){
    do{
      printf( "%x" ,Binding·call(tm ,read) );
      if( Binding·call(tm ,)


  }
   

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
#define _TM·CVT_ AU
#include "TM.lib.c"
#define SET__TM·LOCAL__TM·AU
