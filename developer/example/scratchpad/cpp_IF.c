#include <stdio.h>
#include "macro_lib.c"

/* SUM_IF(condition, a, b)
   - If condition is true (nonzero), returns (a)+(b)
   - Otherwise, returns (a)-(b)
   This uses the IF_ELSE construct from the macro library.
*/
#define SUM_IF(condition, a, b) IF_ELSE(condition)( (a) + (b) )( (a) - (b) )

int main(void) {
    int result_true = SUM_IF(1, 5, 3);   // Expect 5+3 = 8
    int result_false = SUM_IF(0, 5, 3);  // Expect 5-3 = 2

    printf("SUM_IF(1, 5, 3) = %d\n", result_true);
    printf("SUM_IF(0, 5, 3) = %d\n", result_false);
    return 0;
}
