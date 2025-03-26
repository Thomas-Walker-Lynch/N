#include <stdio.h>

/* Stringizing helpers */
#define STR(x) #x
#define XSTR(x) STR(x)

#include "macro_logic.c"

int main(void) {
    /* Test with three tokens */
    printf(
      "FIRST(DEFAULT ,apple ,banana ,cherry) = %s\n" 
      ,XSTR(FIRST(DEFAULT ,apple ,banana ,cherry))
    );
    printf(
      "SECOND(DEFAULT ,apple ,banana ,cherry) = %s\n" 
      ,XSTR(SECOND(DEFAULT ,apple ,banana ,cherry))
    );
    printf(
      "THIRD(DEFAULT ,apple ,banana ,cherry) = %s\n" 
      ,XSTR(THIRD(DEFAULT ,apple ,banana ,cherry))
    );
    
    printf("test default\n");

    printf(
     "FIRST(DEFAULT) = %s\n" 
     ,XSTR(FIRST(DEFAULT))
    );
    printf(
     "FIRST(DEFAULT ,apple) = %s\n" 
     ,XSTR(FIRST(DEFAULT ,apple))
    );
    printf(
     "SECOND(DEFAULT ,apple) = %s\n" 
     ,XSTR(SECOND(DEFAULT ,apple))
    );
    printf(
      "THIRD(DEFAULT ,apple) = %s\n" 
      ,XSTR(THIRD(DEFAULT ,apple))
    );
    
    return 0;
}
