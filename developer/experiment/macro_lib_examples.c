
/*===========================================================================
  Example Usage of the Macro Library
===========================================================================*/

#include "macro_lib.c"

/* Register equality for the tokens we want to compare.
   For each token X used in comparisons, define _IS_EQ_X_X as PROBE().
*/
#define _IS_EQ_dog_dog PROBE()
#define _IS_EQ_cat_cat PROBE()
#define _IS_EQ_parakeet_parakeet PROBE()
#define _IS_EQ_lizard_lizard PROBE()  // Even if lizard might not be added

/* Example 1: Using a static list defined as a macro */
#define static_animals dog,cat,parakeet

/* Example 2: Building a dynamic list using Append.
   We start with an empty list, then append items.
*/
#define list2 Append(dog)
#define list1 Append(list2,cat)
#define list Append(list1,parakeet)

int main(void) {
    /* Using the static list */
    printf("Static list animals:\n");
    printf("MATCH(dog, static_animals) = %d\n", MATCH(dog, static_animals));
    printf("MATCH(cat, static_animals) = %d\n", MATCH(cat, static_animals));
    printf("MATCH(lizard, static_animals) = %d\n", MATCH(lizard, static_animals));

    /* Using the dynamic list built with Append */
    printf("\nDynamic list 'list':\n");
    printf("MATCH(dog, list) = %d\n", MATCH(dog, list));
    printf("MATCH(cat, list) = %d\n", MATCH(cat, list));
    printf("MATCH(parakeet, list) = %d\n", MATCH(parakeet, list));
    printf("MATCH(lizard, list) = %d\n", MATCH(lizard, list));
    return 0;
}
