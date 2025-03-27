#include <stdio.h>

#define STR(...) #__VA_ARGS__
// no evaluation, and one pass of evaluation
#define SHOW(expr) printf("%s --> %s\n", #expr, STR(expr))

#define BE(...) __VA_ARGS__

#define EMPTY() 
#define GROW(x)     19 * _GROW EMPTY() ()(x)
#define _GROW() GROW


int main(void)
{
    printf("One-trampoline minimal example:\n\n");

    //19 * _GROW ()(7)
    SHOW(GROW(7));

    // 19 * 19 * _GROW ()(7)
    SHOW(BE(GROW(7)));

    // 19 * 19 * 19 * _GROW ()(7)
    SHOW(BE(BE(GROW(7))));


    return 0;
}
