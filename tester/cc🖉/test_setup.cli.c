/*

  A placeholder to see if make etc. is working.

*/

#define FACE
#include <stdio.h>
#include <stdlib.h>
#undef FACE

// No need to define IMPLEMENTATION as `main` is one and done.

int main(int argc ,char *argv[] ,char *envp[]){
  if(argc != 1){
    fprintf(stderr, "Usage: %s\n", argv[0]);
    return EXIT_FAILURE;
  }

  fprintf(stderr, "%s done\n", argv[0]);

  return 0;
}


#define LOCAL
#undef LOCAL
