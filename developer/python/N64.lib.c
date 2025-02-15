#define N64·DEBUG

#ifndef FACE
  #define N64·IMPLEMENTATION
  #define FACE
#endif

#ifndef N64·FACE
#define N64·FACE

  #include <stdint.h>
  #include <stdbool.h>
  #include <stdarg.h>
  #include <stdlib.h>

  typedef uint8_t Digit;

  // Digit count is (DIGIT_EXTENT + 1)
  #define N64·DIGIT_COUNT ( 8 + 1 )

  typedef struct N64·T{
    Digit d[N64·DIGIT_COUNT];
  } N64·T;

  // forward declarations for constants
  extern N64·T *N64·zero;
  extern N64·T *N64·one;
  extern N64·T *N64·all_one_bit;
  extern N64·T *N64·msb;

  // forward declarations for allocations, etc.

#endif // N64·FACE

#ifdef N64·IMPLEMENTATION

#ifndef LOCAL
  #include <stdarg.h>
  #include <stdlib.h>

  // compile-time constants
  static N64·T N64·constant[4] = {
  {
    // zero
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 }
  },
  {
    // one
    { 1, 0, 0, 0, 0, 0, 0, 0, 0 }
  },
  {
    // all one bits
    { ( uint8_t )( -1 ), ( uint8_t )( -1 ), ( uint8_t )( -1 ), ( uint8_t )( -1 ), ( uint8_t )( -1 ), ( uint8_t )( -1 ), ( uint8_t )( -1 ), ( uint8_t )( -1 ), ( uint8_t )( -1 ) }
  },
  {
    // msb
    { 0, 0, 0, 0, 0, 0, 0, 0, ( uint8_t )1 << ((sizeof(uint8_t)*8) - 1) }
  }
};

  N64·T *N64·zero        = &N64·constant[0];
  N64·T *N64·one         = &N64·constant[1];
  N64·T *N64·all_one_bit = &N64·constant[2];
  N64·T *N64·msb         = &N64·constant[3];

  // memory allocation prototypes, etc.

#endif // not LOCAL

#ifdef LOCAL
  // local code: actual function bodies, add, subtract, etc.
#endif // LOCAL

#endif // N64·IMPLEMENTATION
