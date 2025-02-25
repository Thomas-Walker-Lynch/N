
/*
  M - The type for the function dictionary (manifold).
  m - a manifold instance, there can be many, m0, m1 ...
  T - Is the type for the tableau. 

*/
#define N32_1x32·DEBUG

#ifndef FACE
#define N32_1x32·IMPLEMENTATION
#define FACE
#endif

//--------------------------------------------------------------------------------
// Interface

#ifndef N32_1x32·FACE
#define N32_1x32·FACE

  #include <stdint.h>
  #include <stdbool.h>
  #include <stdarg.h>
  #include <stdlib.h>

  //----------------------------------------
  // Instance Data (Declaration Only)

  typedef uint64_t Address;

  // tableau type, encapsulated data is unavailable to user code
  typedef struct N32_1x32·T N32_1x32·T;

  extern N32_1x32·T *N32_1x32·zero;
  extern N32_1x32·T *N32_1x32·one;
  extern N32_1x32·T *N32_1x32·all_one_bit;
  extern N32_1x32·T *N32_1x32·lsb;
  extern N32_1x32·T *N32_1x32·msb;

  //----------------------------------------
  // Return/Error Status and handlers

  typedef enum{
    N32_1x32·Status·ok,
    N32_1x32·Status·overflow,
    N32_1x32·Status·accumulator_overflow,
    N32_1x32·Status·carry,
    N32_1x32·Status·borrow,
    N32_1x32·Status·undefined_divide_by_zero,
    N32_1x32·Status·undefined_modulus_zero,
    N32_1x32·Status·gt_max_shift_count,
    N32_1x32·Status·spill_eq_operand, // not currently signaled, result will be spill value
    N32_1x32·Status·one_word_product,
    N32_1x32·Status·two_word_product,
    N32_1x32·Status·ConversionOverflow
  } N32_1x32·Status;

  typedef enum{
    N32_1x32·Order_lt = -1
    ,N32_1x32·Order_eq = 0
    ,N32_1x32·Order_gt = 1
  } N32_1x32·Order;

  // Incomplete conversion N32_1x32·T -> PNT,  N32_1x32·T leftovers
  typedef struct {
    size_t scale; // this is in bytes
    N32_1x32·T *d;      // digits, programmer must point this to a register
  } N32_1x32·Leftover_N;

  // Incomplete conversion PNT -> N32_1x32·T, PNT leftovers
  #define N32_1x32·LEFTOVER_PNT(PNT)\
    typedef struct {\
      size_t scale; // this is in bytes\
      PNT leftover;   // Residual value in PNT format\
    } N32_1x32·Leftover_##PNT;

  #ifdef UINT8_MAX
    N32_1x32·LEFTOVER_PNT(uint8_t)
  #endif
  #ifdef UINT16_MAX
    N32_1x32·LEFTOVER_PNT(uint16_t)
  #endif
  #ifdef UINT32_MAX
    N32_1x32·LEFTOVER_PNT(uint32_t)
  #endif
  #ifdef UINT64_MAX
    N32_1x32·LEFTOVER_PNT(uint64_t)
  #endif
  #ifdef __UINT128_MAX
    N32_1x32·LEFTOVER_PNT(__uint128_t)
  #endif

  // when alloc runs out of memory
  typedef N32_1x32·T *( *N32_1x32·Allocate_MemoryFault )(Address);

  //----------------------------------------
  // Interface

  #define N32_1x32·TO_TYPE(PNT) N32_1x32·Status (*to_##PNT)(const N32_1x32·T *, PNT *, N32_1x32·Leftover_N *)
  #define N32_1x32·FROM_TYPE(PNT) N32_1x32·Status (*from_##PNT)(const PNT *, N32_1x32·T * ,N32_1x32·Leftover_##PNT *)

  typedef struct{

    // memory allocation 
    N32_1x32·T *(*allocate_array_zero)(Address, N32_1x32·Allocate_MemoryFault);
    N32_1x32·T *(*allocate_array)(Address, N32_1x32·Allocate_MemoryFault);
    void (*deallocate)(N32_1x32·T*);
    N32_1x32·T* (*access)(N32_1x32·T*, Address);

    // results fits in operand type functions
    void (*copy)(N32_1x32·T*, N32_1x32·T*);
    void (*bit_and)(N32_1x32·T*, N32_1x32·T*, N32_1x32·T*);
    void (*bit_or)(N32_1x32·T*, N32_1x32·T*, N32_1x32·T*);
    void (*bit_complement)(N32_1x32·T*, N32_1x32·T*);
    void (*bit_twos_complement)(N32_1x32·T*, N32_1x32·T*);

    // tests  
    N32_1x32·Order (*compare)(N32_1x32·T*, N32_1x32·T*);
    bool (*lt)(N32_1x32·T*, N32_1x32·T*);
    bool (*gt)(N32_1x32·T*, N32_1x32·T*);
    bool (*eq)(N32_1x32·T*, N32_1x32·T*);
    bool (*eq_zero)(N32_1x32·T*);

    // arithmetic
    N32_1x32·Status (*accumulate)(N32_1x32·T *accumulator1 ,N32_1x32·T *accumulator0 ,...);
    N32_1x32·Status (*add)(N32_1x32·T*, N32_1x32·T*, N32_1x32·T*);
    bool (*increment)(N32_1x32·T *a);
    N32_1x32·Status (*subtract)(N32_1x32·T*, N32_1x32·T*, N32_1x32·T*);
    N32_1x32·Status (*multiply)(N32_1x32·T*, N32_1x32·T*, N32_1x32·T*, N32_1x32·T*);
    N32_1x32·Status (*divide)(N32_1x32·T*, N32_1x32·T*, N32_1x32·T*, N32_1x32·T*);
    N32_1x32·Status (*modulus)(N32_1x32·T*, N32_1x32·T*, N32_1x32·T*);

    // shift
    N32_1x32·Status (*shift_left)(Address, N32_1x32·T*, N32_1x32·T*, N32_1x32·T*);
    N32_1x32·Status (*shift_right)(Address, N32_1x32·T*, N32_1x32·T*, N32_1x32·T*);
    N32_1x32·Status (*arithmetic_shift_right)(Address, N32_1x32·T*, N32_1x32·T*);

    // import/export
    char *(*to_string)(N32_1x32·T *);

    #ifdef UINT8_MAX
      N32_1x32·TO_TYPE(uint8_t)
      N32_1x32·FROM_TYPE(uint8_t)
    #endif
    #ifdef UINT16_MAX
      N32_1x32·TO_TYPE(uint16_t)
      N32_1x32·FROM_TYPE(uint16_t)
    #endif
    #ifdef UINT32_MAX
      N32_1x32·TO_TYPE(uint32_t)
      N32_1x32·FROM_TYPE(uint32_t)
    #endif
    #ifdef UINT64_MAX
      N32_1x32·TO_TYPE(uint64_t)
      N32_1x32·FROM_TYPE(uint64_t)
    #endif
    #ifdef __UINT128_MAX
      N32_1x32·TO_TYPE(__uint128_t)
      N32_1x32·FROM_TYPE(__uint128_t)
    #endif

  } N32_1x32·M;

  Local const N32_1x32·M N32_1x32·m; // initialized in the LOCAL section

#endif

//--------------------------------------------------------------------------------
// Implementation

#ifdef N32_1x32·IMPLEMENTATION

  typedef uint32_t Digit;
  const uint8_t digit_array_extent = {3};

  // full type definition for Tableau
  struct N32_1x32·T{
    Digit d[digit_array_extent + 1];
  };

  // this part goes into Nlib.a
  #ifndef LOCAL

    #include <stdarg.h>
    #include <stdlib.h>
    #include <stdio.h>

    // the allocate an array of N32
    N32_1x32·T *N32_1x32·allocate_array(Address extent ,N32_1x32·Allocate_MemoryFault memory_fault){
      N32_1x32·T *instance = malloc((extent + 1) * sizeof(N32_1x32·T) );
      if(!instance){
        return memory_fault ? memory_fault(extent) : NULL;
      }
      return instance;
    }

    N32_1x32·T *N32_1x32·allocate_array_zero(Address extent ,N32_1x32·Allocate_MemoryFault memory_fault){
      N32_1x32·T *instance = calloc( extent + 1 ,sizeof(N32_1x32·T) );
      if(!instance){
        return memory_fault ? memory_fault(extent) : NULL;
      }
      return instance;
    }

    void N32_1x32·deallocate(N32_1x32·T *unencumbered){
      free(unencumbered);
    }

  char *to_string(N32_1x32·T *n) {
    // Each byte requires two hex characters, plus "0x" prefix and null terminator
    const Address string_length = (sizeof(Digit) * (digit_array_extent + 1) * 2) + 3;   
    char *buffer = malloc(string_length);
    if (!buffer) {
      return NULL;  // Handle allocation failure
    }

    strcpy(buffer, "0x");  // Prefix the hex representation
    char *ps = buffer + 2;  // Pointer to string buffer (after "0x")

    // Pointer to the most significant digit
    Digit *pd = n->d + digit_array_extent;

    for (; pd >= n->d; pd--) {
      sprintf(ps, "%0*X", (int)(sizeof(Digit) * 2), *pd);
      ps += sizeof(Digit) * 2;  // Move forward in buffer
    }

    return buffer;  // Caller must free the allocated buffer
  }

  #endif

  // This part is included after the user's code. If the code at top is a 'header, then this is a 'tailer'.
  #ifdef LOCAL

    #include "Copy.lib.c"

    CON32_1x32TANTS_BLOCK

    N32_1x32·T *N32_1x32·zero = N32_1x32·constant + 0;
    N32_1x32·T *N32_1x32·one =  N32_1x32·constant + 1;
    N32_1x32·T *N32_1x32·all_one_bit = N32_1x32·constant + 2;
    N32_1x32·T *N32_1x32·msb = &N32_1x32·constant + 3;
    N32_1x32·T *N32_1x32·lsb = &N32_1x32·constant + 1;

    // temporary variables
    // making these LOCAL rather than reserving one block in the library is thread safe
    // allocating a block once is more efficient
    // library code writes these, they are not on the interface

    Local N32_1x32·T N32_1x32·t[4];


    // allocation 

    extern N32_1x32·T *N32_1x32·allocate_array(Address, N32_1x32·Allocate_MemoryFault);
    extern N32_1x32·T *N32_1x32·allocate_array_zero(Address, N32_1x32·Allocate_MemoryFault);
    extern void N32_1x32·deallocate(N32_1x32·T *);

    // so the user can access numbers in an array allocation
    Local N32_1x32·T* N32_1x32·access(N32_1x32·T *array ,Address index){
      return array + index;
    }

    // copy

    Local void N32_1x32·copy(N32_1x32·T *destination ,N32_1x32·T *source){
      if(source == destination) return; // that was easy! 
      *destination = *source;
    }

    Local void N32_1x32·set_to_zero(N32_1x32·T *instance){
      instance->d0 = 0;
    }

    Local void N32_1x32·set_to_one(N32_1x32·T *instance){
      instance->d0 = 1;
    }

    // bit operations

    Local void N32_1x32·bit_and(N32_1x32·T *result, N32_1x32·T *a, N32_1x32·T *b){
      result->d0 = a->d0 & b->d0;
    }

    // result can be one of the operands
    Local void N32_1x32·bit_or(N32_1x32·T *result, N32_1x32·T *a, N32_1x32·T *b){
      result->d0 = a->d0 | b->d0;
    }

    // result can the same as the operand
    Local void N32_1x32·bit_complement(N32_1x32·T *result, N32_1x32·T *a){
      result->d0 = ~a->d0;
    }

    // result can the same as the operand
    Local void N32_1x32·bit_twos_complement(N32_1x32·T *result ,N32_1x32·T *a){
      result->d0 = ~a->d0 + 1;
    }

    // test functions

    Local N32_1x32·Order N32_1x32·compare(N32_1x32·T *a, N32_1x32·T *b){
      if(a->d0 < b->d0) return N32_1x32·Order_lt;
      if(a->d0 > b->d0) return N32_1x32·Order_gt;
      return N32_1x32·Order_eq;
    }

    Local bool N32_1x32·lt(N32_1x32·T *a ,N32_1x32·T *b){
      return  a->d0 < b->d0;
    }    

    Local bool N32_1x32·gt(N32_1x32·T *a ,N32_1x32·T *b){
      return  a->d0 > b->d0;
    }    

    Local bool N32_1x32·eq(N32_1x32·T *a ,N32_1x32·T *b){
      return  a->d0 == b->d0;
    }    

    Local bool N32_1x32·eq_zero(N32_1x32·T *a){
      return  a->d0 == 0;
    }    


    // arithmetic operations

    // For a large number of summands for the lower precision Natural implementations, for accumulate/add/sub, the 'overflow' operand could overflow and thus this routine will halt and return N32_1x32·Status·accumulator1_overflow
    //
    // When accumulator1 and accumulator0 point to the same location, the result is the accumulator1 value.
    Local N32_1x32·Status N32_1x32·accumulate(N32_1x32·T *accumulator1 ,N32_1x32·T *accumulator0 ,...){

      va_list args;
      va_start(args ,accumulator0);
      uint32_t sum = accumulator0->d0;
      uint32_t carry = 0;
      N32_1x32·T *current;

      while( (current = va_arg(args ,N32_1x32·T *)) ){
        sum += current->d0;
        if(sum < current->d0){  // Accumulator1 into carry
          (carry)++;
          if(carry == 0){
            va_end(args);
            return N32_1x32·Status·accumulator1_overflow;
          }
        }
      }
      va_end(args);

      // wipes out prior value of accumulator1
      accumulator1->d0 = carry;

      return N32_1x32·Status·ok;
    }

    Local N32_1x32·Status N32_1x32·add(N32_1x32·T *sum ,N32_1x32·T *a ,N32_1x32·T *b){
      uint64_t result = (uint64_t)a->d0 + (uint64_t)b->d0;
      sum->d0 = (uint32_t)result;
      return (result >> 32) ? N32_1x32·Status·carry : N32_1x32·Status·ok;
    }

    Local bool N32_1x32·increment(N32_1x32·T *a){
      a->d0++;
      return a->d0 == 0;
    }

    Local N32_1x32·Status N32_1x32·subtract(N32_1x32·T *difference ,N32_1x32·T *a ,N32_1x32·T *b){
      uint64_t diff = (uint64_t) a->d0 - (uint64_t) b->d0;
      difference->d0 = (uint32_t)diff;
      return (diff > a->d0) ? N32_1x32·Status·borrow : N32_1x32·Status·ok;
    }

    Local N32_1x32·Status N32_1x32·multiply(N32_1x32·T *product1 ,N32_1x32·T *product0 ,N32_1x32·T *a ,N32_1x32·T *b){
      uint64_t product = (uint64_t)a->d0 * (uint64_t)b->d0;
      product0->d0 = (uint32_t)product;
      product1->d0 = (uint32_t)(product >> 32);

      if(product1->d0 == 0) return N32_1x32·Status·one_word_product;
      return N32_1x32·Status·two_word_product;
    }

    Local N32_1x32·Status N32_1x32·divide(N32_1x32·T *remainder ,N32_1x32·T *quotient ,N32_1x32·T *a ,N32_1x32·T *b){
      if(b->d0 == 0) return N32_1x32·Status·undefined_divide_by_zero; 

      quotient->d0 = a->d0 / b->d0;
      remainder->d0 = a->d0 - (quotient->d0 * b->d0);

      return N32_1x32·Status·ok;
    }

    Local N32_1x32·Status N32_1x32·modulus(N32_1x32·T *remainder ,N32_1x32·T *a ,N32_1x32·T *b){
      if(b->d0 == 0) return N32_1x32·Status·undefined_modulus_zero; 
      uint32_t quotient = a->d0 / b->d0;
      remainder->d0 = a->d0 - (quotient * b->d0);
      return N32_1x32·Status·ok;
    }

    // bit motion

    typedef uint32_t (*ShiftOp)(uint32_t, uint32_t);

    Local uint32_t shift_left_op(uint32_t value, uint32_t amount){
      return value << amount;
    }

    Local uint32_t shift_right_op(uint32_t value, uint32_t amount){
      return value >> amount;
    }

    // modifies all three of its operands
    // in the case of duplicate operands this is the order: first modifies operand, then fill, then spill, 
    Local N32_1x32·Status N32_1x32·shift
    (
     uint32_t shift_count
     ,N32_1x32·T *spill
     ,N32_1x32·T *operand
     ,N32_1x32·T *fill
     ,ShiftOp shift_op
     ,ShiftOp complement_shift_op
     ){

      // If no result is needed, return immediately.
      if(operand == NULL && spill == NULL) return N32_1x32·Status·ok;

      // Treat NULL operand as zero.
      if(operand == NULL){
        operand = &N32_1x32·t[0];
        N32_1x32·copy(operand, N32_1x32·zero);
      }

      // Shifting more than one word breaks our fill/spill model.
      if(shift_count > 31) return N32_1x32·Status·gt_max_shift_count;

      // The given operand is still required after it is modified, so we copy it.
      N32_1x32·T *given_operand = &N32_1x32·t[1];
      N32_1x32·copy(given_operand, operand);

      // Perform the shift
      operand->d0 = shift_op(given_operand->d0, shift_count);
      if(fill != NULL){
        fill->d0 = complement_shift_op(fill->d0, (32 - shift_count));
        N32_1x32·bit_or(operand, operand, fill);
      }
      if(spill != NULL){
        spill->d0 = shift_op(spill->d0, shift_count);
        spill->d0 += complement_shift_op(given_operand->d0, (32 - shift_count));
      }

      return N32_1x32·Status·ok;
    }

    // Define concrete shift functions using valid C function pointers
    Local N32_1x32·Status 
    N32_1x32·shift_left(uint32_t shift_count, N32_1x32·T *spill, N32_1x32·T *operand, N32_1x32·T *fill){
      return N32_1x32·shift(shift_count, spill, operand, fill, shift_left_op, shift_right_op);
    }

    Local N32_1x32·Status 
    N32_1x32·shift_right(uint32_t shift_count, N32_1x32·T *spill, N32_1x32·T *operand, N32_1x32·T *fill){
      return N32_1x32·shift(shift_count, spill, operand, fill, shift_right_op, shift_left_op);
    }

    Local N32_1x32·Status 
    N32_1x32·arithmetic_shift_right(uint32_t shift_count, N32_1x32·T *operand, N32_1x32·T *spill){

      // Guard against excessive shift counts
      if(shift_count > 31) return N32_1x32·Status·gt_max_shift_count;

      // A NULL operand is treated as zero
      if(operand == NULL){
        operand = &N32_1x32·t[0];
        N32_1x32·copy(operand, N32_1x32·zero);
      }

      // Pick the fill value based on the sign bit
      N32_1x32·T *fill = (operand->d0 & 0x80000000) ? N32_1x32·all_one_bit : N32_1x32·zero;

      // Call shift_right with the appropriate fill
      return N32_1x32·shift_right(shift_count, spill, operand, fill);
    }

    #ifdef UINT8_MAX
    #endif
    #ifdef UINT16_MAX
    #endif
    #ifdef UINT32_MAX
    #endif
    #ifdef UINT64_MAX
    #endif

    // Tableau share dictionary
    Local const N32_1x32·M N32_1x32·m = {

      .allocate_array = N32_1x32·allocate_array
      ,.allocate_array_zero = N32_1x32·allocate_array_zero
      ,.deallocate = N32_1x32·deallocate

      ,.copy = N32_1x32·copy
      ,.bit_and = N32_1x32·bit_and
      ,.bit_or = N32_1x32·bit_or
      ,.bit_complement = N32_1x32·bit_complement
      ,.bit_twos_complement = N32_1x32·bit_twos_complement
      ,.compare = N32_1x32·compare
      ,.lt = N32_1x32·lt
      ,.gt = N32_1x32·gt
      ,.eq = N32_1x32·eq
      ,.eq_zero = N32_1x32·eq_zero
      ,.accumulate = N32_1x32·accumulate
      ,.add = N32_1x32·add
      ,.increment = N32_1x32·increment
      ,.subtract = N32_1x32·subtract
      ,.multiply = N32_1x32·multiply
      ,.divide = N32_1x32·divide
      ,.modulus = N32_1x32·modulus
      ,.shift_left = N32_1x32·shift_left
      ,.shift_right = N32_1x32·shift_right
      ,.arithmetic_shift_right = N32_1x32·arithmetic_shift_right

      ,.access = N32_1x32·access
      ,.from_uint32 = N32_1x32·from_uint32

      #ifdef UINT8_MAX
      #endif
      #ifdef UINT16_MAX
      #endif
      #ifdef UINT32_MAX
      #endif
      #ifdef UINT64_MAX
      #endif
      #ifdef __UINT128_MAX
      #endif

    };

    #undef FACE
    #include "Copy.lib.c"

  #endif

#endif
