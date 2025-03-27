#include <stdio.h>
#include "macro_lib.c"

/* Define decrement values for numbers 1 through 5 */
#define DEC_1 0
#define DEC_2 1
#define DEC_3 2
#define DEC_4 3
#define DEC_5 4
#define DEC(n) DEC_##n

/* 
   COUNT_DOWN(n):
   - If n is 0, outputs "Done".
   - Otherwise, outputs the current number (as a string), a space,
     and then recursively calls COUNT_DOWN on DEC(n).

   Note: DEFER1 is used to postpone the recursive call,
   and EVAL (via ESTR) will force the complete expansion.
*/
#define COUNT_DOWN(n) IF_ELSE(EQUAL(n,0))("Done") ( XSTR(n) " " DEFER1(COUNT_DOWN)(DEC(n)) )

/* Minimal stringization helpers.
   STR(x) stringizes without expanding;
   XSTR(x) expands x first then stringizes;
   ESTR(x) forces full expansion (using EVAL) then stringizes.
*/
#define STR(x) #x
#define XSTR(x) STR(x)
#define ESTR(x) XSTR(EVAL(x))

int main(void) {
    printf("Countdown from 5: %s\n", ESTR(COUNT_DOWN(5)));
    return 0;
}
