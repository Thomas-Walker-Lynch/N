#include <stdio.h>

// Static macros
#define VALUE 42
#define OTHER 99
#define JOIN(a, b) a##b
#define SUFFIX ID

// Symbolic macro name and value
#define NAME myvar

// Assign dynamically: #define myvar 42
#assign(NAME, VALUE)

// Assign dynamically: #define pre_ID 99
#assign(JOIN(pre_, SUFFIX), OTHER)

int main(void)
{
    // Check that macros were defined
    printf("myvar: %d\n", myvar);         // Should print 42
    printf("pre_ID: %d\n", pre_ID);       // Should print 99

    return 0;
}
