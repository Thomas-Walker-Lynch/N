/*
  N_32 - a processor native type

  For binary operations:  a op b -> c

  See the document on the proper use of the Natural types.

  On the subject of multiple pointers indicating the same location in memory:

  When a routine has multiple results, and one or more of the result location
  pointers point to the same storage, the routine will either return an error
  status, or have defined behavior.

  When a routine has multiple operands, in any combination, those
  pointers can point to the same location, and the routine will
  function as advertised.

  When an operand functions as both an input and a result, perhaps due
  to a result pointer pointing to the same place as an operand
  pointer, the routine will function as advertised. (Internally the
  routine might make a temporary copy of the operand to accomplish
  this.)

*/

#define N_32·DEBUG

#ifndef IFACE
#define N_32·IMPLEMENTATION
#define IFACE
#endif

//--------------------------------------------------------------------------------
// Interface

#ifndef N_32·IFACE
#define N_32·IFACE

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>

//----------------------------------------
// Instance Data (Declaration Only)

typedef uint32_t Extent;
typedef uint32_t Digit;

typedef struct N_32 N_32;

extern N_32 *N_32·zero;
extern N_32 *N_32·one;
extern N_32 *N_32·all_one_bit;
extern N_32 *N_32·lsb;
extern N_32 *N_32·msb;

//----------------------------------------
// Return/Error Status and handlers

typedef enum{
  N_32·Status·ok = 0
  ,N_32·Status·overflow = 1
  ,N_32·Status·accumulator1_overflow = 2
  ,N_32·Status·carry = 3
  ,N_32·Status·borrow = 4
  ,N_32·Status·undefined_divide_by_zero = 5
  ,N_32·Status·undefined_modulus_zero = 6
  ,N_32·Status·gt_max_lsb_index = 7
  ,N_32·Status·spill_eq_operand = 8 // not currently signaled, result will be spill value
  ,N_32·Status·one_word_product = 9
  ,N_32·Status·two_word_product = 10
} N_32·Status;

typedef enum{
  N_32·Order_lt = -1
  ,N_32·Order_eq = 0
  ,N_32·Order_gt = 1
} N_32·Order;

typedef N_32 *( *N_32·Allocate_MemoryFault )(Extent);

//----------------------------------------
// Interface

typedef struct{
  N_32 *( *allocate )(Extent, N_32·Allocate_MemoryFault);
  void ( *deallocate )(N_32 *);
  void ( *copy )(N_32 *, N_32 *);
  void ( *bit_and )(N_32 *, N_32 *, N_32 *);
  void ( *bit_or )(N_32 *, N_32 *, N_32 *);
  void ( *bit_complement )(N_32 *, N_32 *);
  void ( *bit_twos_complement )(N_32 *, N_32 *);
  N_32·Order ( *compare )(N_32 *, N_32 *);
  bool ( *lt )(N_32 *, N_32 *);
  bool ( *gt )(N_32 *, N_32 *);
  bool ( *eq )(N_32 *, N_32 *);
  N_32·Status ( *add )(N_32 *, N_32 *, N_32 *);
  N_32·Status ( *subtract )(N_32 *, N_32 *, N_32 *);
  N_32·Status ( *multiply )(N_32 *, N_32 *, N_32 *, N_32 *);
  N_32·Status ( *divide )(N_32 *, N_32 *, N_32 *, N_32 *);
  void ( *shift_left )(Extent, N_32 *, N_32 *, N_32 *);
  void ( *shift_right )(Extent, N_32 *, N_32 *, N_32 *);
} N_32·Interface;

extern const N_32·Interface N_32·interface;

#endif

//--------------------------------------------------------------------------------
// Implementation

#ifdef N_32·IMPLEMENTATION

// this part goes into the library
#ifndef LOCAL

#include <stdarg.h>
#include <stdlib.h>

struct N_32{
  Digit d0;
};

N_32 N_32·constant[4] ={
 {.d0 = 0},
 {.d0 = 1},
 {.d0 = ~(uint32_t)0},
 {.d0 = 1 << 31}
};

N_32 *N_32·zero = &N_32·constant[0];
N_32 *N_32·one = &N_32·constant[1];
N_32 *N_32·all_one_bit = &N_32·constant[2];
N_32 *N_32·msb = &N_32·constant[3];
N_32 *N_32·lsb = &N_32·constant[1];

// the allocate an array of N_32
N_32 *N_32·allocate_array( Extent extent ,N_32 *(*memory_fault)(Extent) ){
  N_32 *instance = malloc((extent + 1) * sizeof(N_32) );
  if(!instance){
    return memory_fault ? memory_fault(extent) : NULL;
  }
  return instance;
}

N_32 *N_32·alloc_array_zero( Extent extent ,N_32 *(*memory_fault)(Extent) ){
  N_32 *instance = calloc( extent + 1 ,sizeof(N_32) );
  if(!instance){
    return memory_fault ? memory_fault(extent) : NULL;
  }
  return instance;
}

void N_32·deallocate(N_32 *unencumbered){
  free(unencumbered);
}


#endif

// This part is included after the library user's code
#ifdef LOCAL

// instance

struct N_32{
  Digit d0;
};

// temporary variables
// making these LOCAL rather than reserving one block in the library is thread safe
// allocating a block once is more efficient
// library code writes these, they are not on the interface

Local N_32 N_32·t[4]


// allocation 

extern N_32 *N_32·allocate_array(Extent, N_32·Allocate_MemoryFault);
extern N_32 *N_32·alloc_array_zero(Extent, N_32·Allocate_MemoryFault);
extern void N_32·deallocate(N_32 *);

// copy, convenience copy

Local void N_32·copy(N_32 *destination ,N_32 *source){
  if(source == destination) return; // that was easy! 
  *destination = *source;
}

Local void N_32·set_to_zero(N_32 *instance){
  instance->d0 = 0;
}

Local void N_32·set_to_one(N_32 *instance){
  instance->d0 = 1;
}

// bit operations

Local void N_32·bit_and(N_32 *result, N_32 *a, N_32 *b){
  result->d0 = a->d0 & b->d0;
}

// result can be one of the operands
Local void N_32·bit_or(N_32 *result, N_32 *a, N_32 *b){
  result->d0 = a->d0 | b->d0;
}

// result can the same as the operand
Local void N_32·bit_complement(N_32 *result, N_32 *a){
  result->d0 = ~a->d0;
}

// result can the same as the operand
Local void N_32·bit_twos_complement(N_32 *result ,N_32 *a){
  result->d0 = ~a->d0 + 1;
}

// test functions

Local N_32·Order N_32·compare(N_32 *a, N_32 *b){
  if (a->d0 < b->d0) return N_32·Order_lt;
  if (a->d0 > b->d0) return N_32·Order_gt;
  return N_32·Order_eq;
}

Local bool N_32·lt(N_32 *a ,N_32 *b){
  return  a->d0 < b->d0;
}    

Local bool N_32·gt(N_32 *a ,N_32 *b){
  return  a->d0 > b->d0;
}    

Local bool N_32·eq(N_32 *a ,N_32 *b){
  return  a->d0 == b->d0;
}    

Local bool N_32·eq_zero(N_32 *a){
  return  a->d0 == 0;
}    


// arithmetic operations

// For a large number of summands for the lower precision Natural implementations, for accumulate/add/sub, the 'overflow' operand could overflow and thus this routine will halt and return N_32·Status·accumulator1_overflow
//
// When accumulator1 and accumulator0 point to the same location, the result is the accumulator1 value.
Local N_32·Status N_32·accumulate(N_32 *accumulator1 ,N_32 *accumulator0 ,...){

  va_list args;
  va_start(args ,accumulator0);

  uint64_t *sum = &accumulator0->d0;
  uint64_t *carry = 0;
  N_32 *current;

  while( (current = va_arg(args ,N_32 *)) ){
    *sum += current->d0;
    if(*sum < current->d0){  // Accumulator1 into carry
      (*carry)++;
      if(*carry == 0){
        va_end(args);
        return N_32·Status·accumulator1_overflow;
      }
    }
  }
  va_end(args);

  // wipes out prior value of accumulator1
  accumulator1->d0 = carry;

  return N_32·Status·ok;
}

Local N_32·Status N_32·add(N_32 *sum ,N_32 *a ,N_32 *b){
  uint64_t result = (uint64_t)a->d0 + (uint64_t)b->d0;
  sum->d0 = (uint32_t)result;
  return (result >> 32) ? N_32·Status·carry : N_32·Status·ok;
}

Local bool N_32·increment(N_32 *a){
  a->d0++;
  return a->d0 == 0;
}

Local N_32·Status N_32·subtract(N_32 *difference ,N_32 *a ,N_32 *b){
  uint64_t diff = (uint64_t)a->d0 - (uint64_t)b->d0;
  difference->d0 = (uint32_t)diff;
  return (diff > a->d0) ? N_32·Status·borrow : N_32·Status·ok;
}


Local N_32·Status N_32·multiply(N_32 *product1 ,N_32 *product0 ,N_32 *a ,N_32 *b){
  uint64_t product = (uint64_t)a->d0 * (uint64_t)b->d0;
  product0->d0 = (uint32_t)product;
  product1->d0 = (uint32_t)(product >> 32);

  if(product1->d0 == 0) return N_32·status·one_word_product;
  return N_32·status·two_word_product;
}

Local N_32·Status N_32·divide(N_32 *remainder ,N_32 *quotient ,N_32 *a ,N_32 *b){
  if(b->d0 == 0) return N_32·Status·undefined_divide_by_zero; 

  quotient->d0 = a->d0 / b->d0;
  remainder->d0 = a->d0 - (quotient->d0 * b->d0);

  return N_32·Status·ok;
}

Local N_32·Status N_32·modulus(N_32 *remainder ,N_32 *a ,N_32 *b){
  if(b->d0 == 0) return N_32·Status·undefined_modulus_zero; 
  uint32_t quotient = a->d0 / b->d0;
  remainder->d0 = a->d0 - (quotient * b->d0);
  return N_32·Status·ok;
}

// bit motion

// This is a bit shift, not a word shift. If you have a general shift count, shift words by
// the div of the shift count, then call this with the modulus 32 of the shift count.
//
// If spill and operand or fill are the same value, the result is the spill (for both).
//
// `new_lsb_index` aka the `shift_count`
// shifts a binary number the lsb bit index to as far as 31 
// NULL operand treated as zero
Local N_32·Status N_32·shift_left
  (
   uint32_t new_lsb_index
   ,N_32 *spill
   ,N_32 *operand
   ,N_32 *fill
   ){

  // If the caller doesn't want a result back, we are all done.
  if(operand == NULL && spill == NULL) return N_32·Status·ok;

  #ifdef N_32·DEBUG
  if( spill == operand ){
    fprintf(stderr, "N_32·shift_left spill == operand\n");
  }
  #endif

  // For the sake of setting spill, we will treat a NULL operand as zero.
  if(operand == NULL){
    operand = &N_32·t[0];
    N_32·copy(operand ,N_32·zero);
  }

  // Shifting more than one word breaks our fill/spill model.
  if(new_lsb_index > 31) return N_32·Status·gt_max_lsb_index;

  // The given operand is still required after it is modified, so we copy it.
  N_32 *given_operand = &N_32·t[1];
  N_32·copy(given_operand ,operand);

  // spill is overwritten with a new value
  operand->d0 = given_operand->d0 << new_lsb_index;
  if(fill != NULL){
    fill->d0 = fill->d0 >> (32 - new_lsb_index);
    N_32·bit_or(operand ,operand ,fill);
  }
  if(spill != NULL){
    spill->d0 = spill->d0 << new_lsb_index;
    spill->d0 += given_operand->d0 >> (32 - new_lsb_index);
  }
  return N_32·Status·ok;
}

Local N_32·Status N_32·shift_right
  (
   uint32_t msb_index_offset
   ,N_32 *spill
   ,N_32 *operand
   ,N_32 *fill
   ){

  // If the caller doesn't want a result back, we are all done.
  if(operand == NULL && spill == NULL) return N_32·Status·ok;

  // For the sake of setting spill, we will treat a NULL operand as zero.
  if(operand == NULL){
    operand = &N_32·t[0];
    N_32·copy(operand ,N_32·zero);
  }

  // Shifting more than one word breaks our fill/spill model.
  if(msb_index_offset > 31) return N_32·Status·gt_max_lsb_index;

  // In theory, fill should not have any bits in common with the shifted operand.
  #ifdef N_32·DEBUG
  if( !N_32·eq_zero(fill->d0 << (32 - msb_index_offset)) ){
    fprintf(stderr, "N_32·shift_right fill value overlaps with shifted operand\n");
  }
  #endif

  // The given operand is still required after it is modified, so we copy it.
  N_32 *given_operand = &N_32·t[1];
  N_32·copy(given_operand ,operand);

  // spill is overwritten with a new value
  operand->d0 = given_operand->d0 >> msb_index_offset;
  if(fill != NULL){
    fill->d0 = fill->d0 << (32 - msb_index_offset);
    N_32·bit_or(operand ,operand ,fill);
  }
  if(spill != NULL){
    spill->d0 = spill->d0 >> msb_index_offset;
    spill->d0 += given_operand->d0 << (32 - msb_index_offset);
  }
  return N_32·Status·ok;
}


Local void N_32·arithmetic_shift_right(Extent shift, N_32 *operand, N_32 *spill) {

  // caller wants us to do nothing, a little strange, but Ok ..
  if (operand == NULL || spill == NULL) return N_32·Status·ok;

  // check if we are shifting in ones or zeros
  N_32·bit_and(&N_32·t[0], operand, N_32·msb); 
  N_32 *source_pt = N_32·eq_zero(&N_32·t[0]) ? N_32·zero : N_32·all_one_bit;

  // uses `t[2]` because shift_right uses `t[0]` and `t[1]`
  N_32·copy(&N_32·t[2], source_pt);
  N_32·shift_right(shift, &N_32·t[2], operand, spill);
}

// an interface instance

Local const N_32·Interface N_32·interface = {
   .copy = N_32·copy
  ,.bit_and = N_32·bit_and
  ,.bit_or = N_32·bit_or
  ,.bit_complement = N_32·bit_complement
  ,.bit_twos_complement = N_32·bit_twos_complement
  ,.compare = N_32·compare
  ,.lt = N_32·lt
  ,.gt = N_32·gt
  ,.eq = N_32·eq
  ,.eq_zero = N_32·eq_zero
  ,.eq_one = N_32·eq_one
  ,.accumulate = N_32·accumulate
  ,.add = N_32·add
  ,.next = N_32·next
  ,.subtract = N_32·subtract
  ,.multiply = N_32·multiply
  ,.divide = N_32·divide
  ,.modulus = N_32·modulus
  ,.shift_left = N_32·shift_left
  ,.shift_right = N_32·shift_right
  ,.arithmetic_shift_right = N_32·arithmetic_shift_right
  ,.allocate_array = N_32·allocate_array
  ,.allocate_array_zero = N_32·alloc_array_zero
  ,.deallocate = N_32·deallocate
};

#endif

#endif
