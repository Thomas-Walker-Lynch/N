def get_template():
    """
    Returns the base RT C code template, with placeholders like:
      {NAMESPACE}, {DIGIT_EXTENT}, {DIGIT_TYPE}, {EXTENT_TYPE}, {CONSTANTS_BLOCK}.
    
    The final generated .lib.c will replace these with user-specified values.
    """
    return r'''#define {NAMESPACE}DEBUG

#ifndef FACE
  #define {NAMESPACE}IMPLEMENTATION
  #define FACE
#endif

#ifndef {NAMESPACE}FACE
#define {NAMESPACE}FACE

  #include <stdint.h>
  #include <stdbool.h>
  #include <stdarg.h>
  #include <stdlib.h>

  typedef {DIGIT_TYPE} Digit;

  // Digit count is (DIGIT_EXTENT + 1)
  #define {NAMESPACE}DIGIT_COUNT ( {DIGIT_EXTENT} + 1 )

  typedef struct {NAMESPACE}T{{
    Digit d[{NAMESPACE}DIGIT_COUNT];
  }} {NAMESPACE}T;

  // forward declarations for constants
  extern {NAMESPACE}T *{NAMESPACE}zero;
  extern {NAMESPACE}T *{NAMESPACE}one;
  extern {NAMESPACE}T *{NAMESPACE}all_one_bit;
  extern {NAMESPACE}T *{NAMESPACE}msb;

  // forward declarations for allocations, etc.

#endif // {NAMESPACE}FACE

#ifdef {NAMESPACE}IMPLEMENTATION

#ifndef LOCAL
  #include <stdarg.h>
  #include <stdlib.h>

  // compile-time constants
  {CONSTANTS_BLOCK}

  {NAMESPACE}T *{NAMESPACE}zero        = &{NAMESPACE}constant[0];
  {NAMESPACE}T *{NAMESPACE}one         = &{NAMESPACE}constant[1];
  {NAMESPACE}T *{NAMESPACE}all_one_bit = &{NAMESPACE}constant[2];
  {NAMESPACE}T *{NAMESPACE}msb         = &{NAMESPACE}constant[3];

  // memory allocation prototypes, etc.

#endif // not LOCAL

#ifdef LOCAL
  // local code: actual function bodies, add, subtract, etc.
#endif // LOCAL

#endif // {NAMESPACE}IMPLEMENTATION
'''
