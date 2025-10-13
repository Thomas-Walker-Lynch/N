//----------------------------------------
// Function Declarations

void N_32_copy(N_32 *destination, N_32 *source);
void N_32_bit_and(N_32 *result, N_32 *a, N_32 *b);
void N_32_bit_or(N_32 *result, N_32 *a, N_32 *b);
void N_32_bit_complement(N_32 *result, N_32 *a);
N_32_Order N_32_compare(N_32 *a, N_32 *b);
bool N_32_lt(N_32 *a, N_32 *b);
bool N_32_gt(N_32 *a, N_32 *b);
bool N_32_eq(N_32 *a, N_32 *b);
N_32_Status N_32_add(N_32 *sum, N_32 *a, N_32 *b);
N_32_Status N_32_subtract(N_32 *difference, N_32 *a, N_32 *b);
N_32_Status N_32_multiply(N_32 *overflow, N_32 *result, N_32 *a, N_32 *b);
N_32_Status N_32_divide(N_32 *remainder, N_32 *quotient, N_32 *a, N_32 *b);
void N_32_shift_left(Extent shift, N_32 *sink, N_32 *result, N_32 *source);
void N_32_shift_right(Extent shift, N_32 *source, N_32 *result, N_32 *sink);


/*
  N_32 - a processor native type

  For binary operations:  a op b -> c

  To use N_32, first allocate a block of N_32. Do the arithmetic,
  if any results need be kept, copy them to another block. Then deallocate
  the block. Do not allocate numbers one at a time, or it would be slow.
*/

#ifndef IFACE
#define N_32_IMPLEMENTATION
#define IFACE
#endif

#ifndef N_32_IFACE
#define N_32_IFACE

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>

//----------------------------------------
// Instance Data (Declaration Only)

typedef uint32_t Extent;
typedef uint32_t Digit;

typedef struct N_32 N_32;

extern N_32 *N_32_zero;
extern N_32 *N_32_one;
extern N_32 *N_32_all_one_bit;
extern N_32 *N_32_lsb;
extern N_32 *N_32_msb;

//----------------------------------------
// Return Status

typedef enum {
  N_32_Status_ok = 0,
  N_32_Status_overflow = 1,
  N_32_Status_accumulator_overflow = 2,
  N_32_Status_carry = 3,
  N_32_Status_borrow = 4,
  N_32_Status_undefined_divide_by_zero = 5,
  N_32_Status_undefined_modulus_zero = 6
} N_32_Status;

typedef enum {
  N_32_Order_lt = -1,
  N_32_Order_eq = 0,
  N_32_Order_gt = 1
} N_32_Order;

typedef N_32 *( *N_32_Allocate_MemoryFault )(Extent);

//----------------------------------------
// Interface

typedef struct {
  N_32 *( *allocate )(Extent, N_32_Allocate_MemoryFault);
  void ( *deallocate )(N_32 *);
  void ( *copy )(N_32 *, N_32 *);
  void ( *bit_and )(N_32 *, N_32 *, N_32 *);
  void ( *bit_or )(N_32 *, N_32 *, N_32 *);
  void ( *bit_complement )(N_32 *, N_32 *);
  N_32_Order ( *compare )(N_32 *, N_32 *);
  bool ( *lt )(N_32 *, N_32 *);
  bool ( *gt )(N_32 *, N_32 *);
  bool ( *eq )(N_32 *, N_32 *);
  N_32_Status ( *add )(N_32 *, N_32 *, N_32 *);
  N_32_Status ( *subtract )(N_32 *, N_32 *, N_32 *);
  N_32_Status ( *multiply )(N_32 *, N_32 *, N_32 *, N_32 *);
  N_32_Status ( *divide )(N_32 *, N_32 *, N_32 *, N_32 *);
  void ( *shift_left )(Extent, N_32 *, N_32 *, N_32 *);
  void ( *shift_right )(Extent, N_32 *, N_32 *, N_32 *);
} N_32_Interface;

extern const N_32_Interface N_32_interface;

#endif

#ifdef N_32_IMPLEMENTATION

#ifndef LOCAL

struct{
  Digit d0;
} N_32;

N_32 N_32_constant[4] = {
  { .d0 = 0 },
  { .d0 = 1 },
  { .d0 = ~(uint32_t)0 },
  { .d0 = 1 << 31 }
};

N_32 *N_32_zero = &N_32_constant[0];
N_32 *N_32_one = &N_32_constant[1];
N_32 *N_32_all_one_bit = &N_32_constant[2];
N_32 *N_32_msb = &N_32_constant[3];
N_32 *N_32_lsb = &N_32_constant[1];

#endif

#ifdef LOCAL

struct{
  Digit d0;
} N_32;

Local void N_32_copy(N_32 *destination, N_32 *source) {
  *destination = *source;
}

Local void N_32_bit_and(N_32 *result, N_32 *a, N_32 *b) {
  result->d0 = a->d0 & b->d0;
}

Local void N_32_bit_or(N_32 *result, N_32 *a, N_32 *b) {
  result->d0 = a->d0 | b->d0;
}

Local void N_32_bit_complement(N_32 *result, N_32 *a) {
  result->d0 = ~a->d0;
}

Local N_32_Order N_32_compare(N_32 *a, N_32 *b) {
  if (a->d0 < b->d0) return N_32_Order_lt;
  if (a->d0 > b->d0) return N_32_Order_gt;
  return N_32_Order_eq;
}

Local void N_32_shift_left(Extent shift, N_32 *sink, N_32 *result, N_32 *source) {
  *sink = *source;
  result->d0 = source->d0 << shift;
  sink->d0 = source->d0 >> (32 - shift);
}

Local void N_32_shift_right(Extent shift, N_32 *source, N_32 *result, N_32 *sink) {
  *sink = *source;
  result->d0 = source->d0 >> shift;
  sink->d0 = source->d0 << (32 - shift);
}

Local const N_32_Interface N_32_interface = {
  .copy = N_32_copy,
  .bit_and = N_32_bit_and,
  .bit_or = N_32_bit_or,
  .bit_complement = N_32_bit_complement,
  .compare = N_32_compare,
  .shift_left = N_32_shift_left,
  .shift_right = N_32_shift_right
};

#endif

#endif
