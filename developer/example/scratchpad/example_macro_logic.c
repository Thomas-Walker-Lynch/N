#include <stdio.h>

#include "macro_logic.c"

#define STR(x) #x
#define XSTR(x) STR(x)

/*===========================================================================
Example Program
===========================================================================*/
int main(void) {
    /* For testing purposes, we print the macro expansion results as strings.
       (Because TRUE and FALSE have no numeric value, we rely on stringized output.)
    */
    printf("TRUE: %s\n", XSTR(TRUE));

    printf("FIRST(a,b,c) = %s\n", XSTR( FIRST(0 ,a ,b ,c) ));
    printf("FIRST(a,b,c) = %s\n", XSTR( FIRST(DEFER3(PAIR)() ,a ,b, ,c) ));

    printf("FIRST() = %s\n", XSTR( FIRST(7) ));
    printf("FIRST() = %s\n", XSTR( FIRST(PAIR()) ));
    printf("FIRST() = %s\n", XSTR( FIRST(DEFER3(PAIR)()) ));


#if 0


    printf("EXIST() = %s\n", XSTR(EXIST()));
    printf("EXIST(a) = %s\n", XSTR(EXIST(a)));

    printf("NOT_EXIST(TRUE) = %s\n", XSTR(NOT_EXIST(TRUE)));
    printf("EXIST(FALSE) = %s\n", XSTR(EXIST(FALSE)));
    printf("NOT_EXIST(FALSE) = %s\n", XSTR(NOT_EXIST(FALSE)));

    printf("BOOL(TRUE) = %s\n", XSTR(BOOL(TRUE)));
    printf("BOOL(FALSE) = %s\n", XSTR(BOOL(FALSE)));

    printf("EQUAL(TRUE, TRUE) = %s\n", XSTR(EQUAL(TRUE, TRUE)));
    printf("EQUAL(FALSE, FALSE) = %s\n", XSTR(EQUAL(FALSE, FALSE)));
    printf("EQUAL(TRUE, FALSE) = %s\n", XSTR(EQUAL(TRUE, FALSE)));

    printf("IF_ELSE(TRUE)(Yes)(No) = %s\n", XSTR(IF_ELSE(TRUE)(Yes)(No)));
    printf("IF_ELSE(FALSE)(Yes)(No) = %s\n", XSTR(IF_ELSE(FALSE)(Yes)(No)));


#endif
    return 0;
}
