#include <stdio.h>
#include "macro_lib.c"

/* Helper macros to turn expanded results into strings.
   Note that STR(x) simply stringizes x without expanding it.
   XSTR(x) expands x first (if possible) and then stringizes it.
   We use EVAL to force full expansion before stringizing.
*/

#define STR(x) #x
#define XSTR(x) STR(x)
#define ESTR(x) XSTR(EVAL(x))

/* --- Example Usage --- */

/* Test 1: FIRST and SECOND */
int main(void) {
  printf("macro_use.c\n");

  printf("FIRST(dog ,cat) = %s\n" ,ESTR(FIRST(dog ,cat)));

  printf("SECOND(dog ,cat) = %s\n" ,ESTR(SECOND(dog ,cat)));
  printf("SECOND(dog) = %s\n" ,ESTR(SECOND(dog)));

  printf("_IS_PROBE(dog ,cat) = %s\n" ,ESTR(_IS_PROBE(dog ,cat)));
  printf("_IS_PROBE(dog) = %s\n" ,ESTR(_IS_PROBE(dog)));

  /* Test 2: CAT with no separator and with COMMA */
  printf( "CAT( x ,dog ,cat ,parakeet) = %s\n" ,ESTR(CAT( ,dog ,cat ,parakeet)) );

  printf( "CAT( ,dog ,cat ,parakeet) = %s\n" ,ESTR(EVAL(CAT( ,dog ,cat ,parakeet))) );


#if 0

  printf("CAT(COMMA ,dog ,cat ,parakeet) = %s\n" ,ESTR(CAT(COMMA ,dog ,cat ,parakeet)));

  /* Test 3: Xi macro (using a center dot as separator) */
  printf("Ξ(foo ,bar) = %s\n" ,ESTR(Ξ(foo ,bar)));

  /* Test 4: Append macro (using COMMA as separator) */
  printf("Append(dog ,cat) = %s\n" ,ESTR(Append(dog ,cat)));
  printf("Append(dog) = %s\n" ,ESTR(Append(dog)));

  /* Test 5: MATCH macro */
  /* Define a static list of animals */
  #define static_animals dog,cat,parakeet
  /* Register equality for tokens that will be compared */
  #undef _IS_EQ_dog_dog
  #undef _IS_EQ_cat_cat
  #undef _IS_EQ_parakeet_parakeet
  #define _IS_EQ_dog_dog PROBE()
  #define _IS_EQ_cat_cat PROBE()
  #define _IS_EQ_parakeet_parakeet PROBE()
  #define _IS_EQ_lizard_lizard PROBE()  /* lizard is not in the list */

  printf("MATCH(dog ,static_animals) = %s\n" ,ESTR(MATCH(dog ,static_animals)));
  printf("MATCH(cat ,static_animals) = %s\n" ,ESTR(MATCH(cat ,static_animals)));
  printf("MATCH(parakeet ,static_animals) = %s\n" ,ESTR(MATCH(parakeet ,static_animals)));
  printf("MATCH(lizard ,static_animals) = %s\n" ,ESTR(MATCH(lizard ,static_animals)));
#endif

  return 0;
}
