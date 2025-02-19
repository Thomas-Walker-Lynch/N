/*
  N32 - a processor native type

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

#define N32PN·DEBUG

#ifndef FACE
#define N32PN·IMPLEMENTATION
#define FACE
#endif

//--------------------------------------------------------------------------------
// Interface

#ifndef N32PN·FACE
#define N32PN·FACE

  #include <stdint.h>
  #include <stdbool.h>
  #include <stdarg.h>
  #include <stdlib.h>

  //----------------------------------------
  // Instance Data (Declaration Only)

  typedef uint32_t Extent;
  typedef uint32_t Digit;

  typedef struct N32PN·T N32PN·T;

  extern N32PN·T *N32PN·zero;
  extern N32PN·T *N32PN·one;
  extern N32PN·T *N32PN·all_one_bit;
  extern N32PN·T *N32PN·lsb;
  extern N32PN·T *N32PN·msb;

  //----------------------------------------
  // Return/Error Status and handlers

  typedef enum{
    N32PN·Status·ok = 0
    ,N32PN·Status·overflow = 1
    ,N32PN·Status·accumulator1_overflow = 2
    ,N32PN·Status·carry = 3
    ,N32PN·Status·borrow = 4
    ,N32PN·Status·undefined_divide_by_zero = 5
    ,N32PN·Status·undefined_modulus_zero = 6
    ,N32PN·Status·gt_max_shift_count = 7
    ,N32PN·Status·spill_eq_operand = 8 // not currently signaled, result will be spill value
    ,N32PN·Status·one_word_product = 9
    ,N32PN·Status·two_word_product = 10
  } N32PN·Status;

  typedef enum{
    N32PN·Order_lt = -1
    ,N32PN·Order_eq = 0
    ,N32PN·Order_gt = 1
  } N32PN·Order;

  typedef N32PN·T *( *N32PN·Allocate_MemoryFault )(Extent);

  //----------------------------------------
  // Interface

  typedef struct{

    N32PN·T *(*allocate_array_zero)(Extent, N32PN·Allocate_MemoryFault);
    N32PN·T *(*allocate_array)(Extent, N32PN·Allocate_MemoryFault);
    void (*deallocate)(N32PN·T*);

    void (*copy)(N32PN·T*, N32PN·T*);
    void (*bit_and)(N32PN·T*, N32PN·T*, N32PN·T*);
    void (*bit_or)(N32PN·T*, N32PN·T*, N32PN·T*);
    void (*bit_complement)(N32PN·T*, N32PN·T*);
    void (*bit_twos_complement)(N32PN·T*, N32PN·T*);
    N32PN·Order (*compare)(N32PN·T*, N32PN·T*);
    bool (*lt)(N32PN·T*, N32PN·T*);
    bool (*gt)(N32PN·T*, N32PN·T*);
    bool (*eq)(N32PN·T*, N32PN·T*);
    bool (*eq_zero)(N32PN·T*);
    N32PN·Status (*accumulate)(N32PN·T *accumulator1 ,N32PN·T *accumulator0 ,...);
    N32PN·Status (*add)(N32PN·T*, N32PN·T*, N32PN·T*);
    bool (*increment)(N32PN·T *a);
    N32PN·Status (*subtract)(N32PN·T*, N32PN·T*, N32PN·T*);
    N32PN·Status (*multiply)(N32PN·T*, N32PN·T*, N32PN·T*, N32PN·T*);
    N32PN·Status (*divide)(N32PN·T*, N32PN·T*, N32PN·T*, N32PN·T*);
    N32PN·Status (*modulus)(N32PN·T*, N32PN·T*, N32PN·T*);
    N32PN·Status (*shift_left)(Extent, N32PN·T*, N32PN·T*, N32PN·T*);
    N32PN·Status (*shift_right)(Extent, N32PN·T*, N32PN·T*, N32PN·T*);
    N32PN·Status (*arithmetic_shift_right)(Extent, N32PN·T*, N32PN·T*);

    N32PN·T* (*access)(N32PN·T*, Extent);
    void (*from_uint32)(N32PN·T *destination ,uint32_t value);
  } N32PN·M;

  Local const N32PN·M N32PN·m; // initialized in the LOCAL section

#endif

//--------------------------------------------------------------------------------
// Implementation

#ifdef N32PN·IMPLEMENTATION

  // this part goes into the library
  #ifndef LOCAL

    #include <stdarg.h>
    #include <stdlib.h>

    struct N32PN·T{
      Digit d0;
    };

    N32PN·T N32PN·constant[4] = {
      {.d0 = 0},
      {.d0 = 1},
      {.d0 = ~(uint32_t)0},
      {.d0 = 1 << 31}
    };

    N32PN·T *N32PN·zero = &N32PN·constant[0];
    N32PN·T *N32PN·one = &N32PN·constant[1];
    N32PN·T *N32PN·all_one_bit = &N32PN·constant[2];
    N32PN·T *N32PN·msb = &N32PN·constant[3];
    N32PN·T *N32PN·lsb = &N32PN·constant[1];

    // the allocate an array of N32
    N32PN·T *N32PN·allocate_array(Extent extent ,N32PN·Allocate_MemoryFault memory_fault){
      N32PN·T *instance = malloc((extent + 1) * sizeof(N32PN·T) );
      if(!instance){
        return memory_fault ? memory_fault(extent) : NULL;
      }
      return instance;
    }

    N32PN·T *N32PN·allocate_array_zero(Extent extent ,N32PN·Allocate_MemoryFault memory_fault){
      N32PN·T *instance = calloc( extent + 1 ,sizeof(N32PN·T) );
      if(!instance){
        return memory_fault ? memory_fault(extent) : NULL;
      }
      return instance;
    }

    void N32PN·deallocate(N32PN·T *unencumbered){
      free(unencumbered);
    }

  #endif

  // This part is included after the library user's code
  #ifdef LOCAL

    // instance

    struct N32PN·T{
      Digit d0;
    };

    // temporary variables
    // making these LOCAL rather than reserving one block in the library is thread safe
    // allocating a block once is more efficient
    // library code writes these, they are not on the interface

    Local N32PN·T N32PN·t[4];


    // allocation 

    extern N32PN·T *N32PN·allocate_array(Extent, N32PN·Allocate_MemoryFault);
    extern N32PN·T *N32PN·allocate_array_zero(Extent, N32PN·Allocate_MemoryFault);
    extern void N32PN·deallocate(N32PN·T *);

    // so the user can access numbers in an array allocation
    Local N32PN·T* N32PN·access(N32PN·T *array ,Extent index){
      return &array[index];
    }

    Local void N32PN·from_uint32(N32PN·T *destination ,uint32_t value){
      if(destination == NULL) return;
      destination->d0 = value;
    }

    // copy, convenience copy

    Local void N32PN·copy(N32PN·T *destination ,N32PN·T *source){
      if(source == destination) return; // that was easy! 
      *destination = *source;
    }

    Local void N32PN·set_to_zero(N32PN·T *instance){
      instance->d0 = 0;
    }

    Local void N32PN·set_to_one(N32PN·T *instance){
      instance->d0 = 1;
    }

    // bit operations

    Local void N32PN·bit_and(N32PN·T *result, N32PN·T *a, N32PN·T *b){
      result->d0 = a->d0 & b->d0;
    }

    // result can be one of the operands
    Local void N32PN·bit_or(N32PN·T *result, N32PN·T *a, N32PN·T *b){
      result->d0 = a->d0 | b->d0;
    }

    // result can the same as the operand
    Local void N32PN·bit_complement(N32PN·T *result, N32PN·T *a){
      result->d0 = ~a->d0;
    }

    // result can the same as the operand
    Local void N32PN·bit_twos_complement(N32PN·T *result ,N32PN·T *a){
      result->d0 = ~a->d0 + 1;
    }

    // test functions

    Local N32PN·Order N32PN·compare(N32PN·T *a, N32PN·T *b){
      if(a->d0 < b->d0) return N32PN·Order_lt;
      if(a->d0 > b->d0) return N32PN·Order_gt;
      return N32PN·Order_eq;
    }

    Local bool N32PN·lt(N32PN·T *a ,N32PN·T *b){
      return  a->d0 < b->d0;
    }    

    Local bool N32PN·gt(N32PN·T *a ,N32PN·T *b){
      return  a->d0 > b->d0;
    }    

    Local bool N32PN·eq(N32PN·T *a ,N32PN·T *b){
      return  a->d0 == b->d0;
    }    

    Local bool N32PN·eq_zero(N32PN·T *a){
      return  a->d0 == 0;
    }    


    // arithmetic operations

    // For a large number of summands for the lower precision Natural implementations, for accumulate/add/sub, the 'overflow' operand could overflow and thus this routine will halt and return N32PN·Status·accumulator1_overflow
    //
    // When accumulator1 and accumulator0 point to the same location, the result is the accumulator1 value.
    Local N32PN·Status N32PN·accumulate(N32PN·T *accumulator1 ,N32PN·T *accumulator0 ,...){

      va_list args;
      va_start(args ,accumulator0);
      uint32_t sum = accumulator0->d0;
      uint32_t carry = 0;
      N32PN·T *current;

      while( (current = va_arg(args ,N32PN·T *)) ){
        sum += current->d0;
        if(sum < current->d0){  // Accumulator1 into carry
          (carry)++;
          if(carry == 0){
            va_end(args);
            return N32PN·Status·accumulator1_overflow;
          }
        }
      }
      va_end(args);

      // wipes out prior value of accumulator1
      accumulator1->d0 = carry;

      return N32PN·Status·ok;
    }

    Local N32PN·Status N32PN·add(N32PN·T *sum ,N32PN·T *a ,N32PN·T *b){
      uint64_t result = (uint64_t)a->d0 + (uint64_t)b->d0;
      sum->d0 = (uint32_t)result;
      return (result >> 32) ? N32PN·Status·carry : N32PN·Status·ok;
    }

    Local bool N32PN·increment(N32PN·T *a){
      a->d0++;
      return a->d0 == 0;
    }

    Local N32PN·Status N32PN·subtract(N32PN·T *difference ,N32PN·T *a ,N32PN·T *b){
      uint64_t diff = (uint64_t) a->d0 - (uint64_t) b->d0;
      difference->d0 = (uint32_t)diff;
      return (diff > a->d0) ? N32PN·Status·borrow : N32PN·Status·ok;
    }


    Local N32PN·Status N32PN·multiply(N32PN·T *product1 ,N32PN·T *product0 ,N32PN·T *a ,N32PN·T *b){
      uint64_t product = (uint64_t)a->d0 * (uint64_t)b->d0;
      product0->d0 = (uint32_t)product;
      product1->d0 = (uint32_t)(product >> 32);

      if(product1->d0 == 0) return N32PN·Status·one_word_product;
      return N32PN·Status·two_word_product;
    }

    Local N32PN·Status N32PN·divide(N32PN·T *remainder ,N32PN·T *quotient ,N32PN·T *a ,N32PN·T *b){
      if(b->d0 == 0) return N32PN·Status·undefined_divide_by_zero; 

      quotient->d0 = a->d0 / b->d0;
      remainder->d0 = a->d0 - (quotient->d0 * b->d0);

      return N32PN·Status·ok;
    }

    Local N32PN·Status N32PN·modulus(N32PN·T *remainder ,N32PN·T *a ,N32PN·T *b){
      if(b->d0 == 0) return N32PN·Status·undefined_modulus_zero; 
      uint32_t quotient = a->d0 / b->d0;
      remainder->d0 = a->d0 - (quotient * b->d0);
      return N32PN·Status·ok;
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
    Local N32PN·Status N32PN·shift
    (
     uint32_t shift_count
     ,N32PN·T *spill
     ,N32PN·T *operand
     ,N32PN·T *fill
     ,ShiftOp shift_op
     ,ShiftOp complement_shift_op
     ){

      // If no result is needed, return immediately.
      if(operand == NULL && spill == NULL) return N32PN·Status·ok;

      // Treat NULL operand as zero.
      if(operand == NULL){
        operand = &N32PN·t[0];
        N32PN·copy(operand, N32PN·zero);
      }

      // Shifting more than one word breaks our fill/spill model.
      if(shift_count > 31) return N32PN·Status·gt_max_shift_count;

      // The given operand is still required after it is modified, so we copy it.
      N32PN·T *given_operand = &N32PN·t[1];
      N32PN·copy(given_operand, operand);

      // Perform the shift
      operand->d0 = shift_op(given_operand->d0, shift_count);
      if(fill != NULL){
        fill->d0 = complement_shift_op(fill->d0, (32 - shift_count));
        N32PN·bit_or(operand, operand, fill);
      }
      if(spill != NULL){
        spill->d0 = shift_op(spill->d0, shift_count);
        spill->d0 += complement_shift_op(given_operand->d0, (32 - shift_count));
      }

      return N32PN·Status·ok;
    }

    // Define concrete shift functions using valid C function pointers
    Local N32PN·Status 
    N32PN·shift_left(uint32_t shift_count, N32PN·T *spill, N32PN·T *operand, N32PN·T *fill){
      return N32PN·shift(shift_count, spill, operand, fill, shift_left_op, shift_right_op);
    }

    Local N32PN·Status 
    N32PN·shift_right(uint32_t shift_count, N32PN·T *spill, N32PN·T *operand, N32PN·T *fill){
      return N32PN·shift(shift_count, spill, operand, fill, shift_right_op, shift_left_op);
    }

    Local N32PN·Status 
    N32PN·arithmetic_shift_right(uint32_t shift_count, N32PN·T *operand, N32PN·T *spill){

      // Guard against excessive shift counts
      if(shift_count > 31) return N32PN·Status·gt_max_shift_count;

      // A NULL operand is treated as zero
      if(operand == NULL){
        operand = &N32PN·t[0];
        N32PN·copy(operand, N32PN·zero);
      }

      // Pick the fill value based on the sign bit
      N32PN·T *fill = (operand->d0 & 0x80000000) ? N32PN·all_one_bit : N32PN·zero;

      // Call shift_right with the appropriate fill
      return N32PN·shift_right(shift_count, spill, operand, fill);
    }

    Local const N32PN·M N32PN·m = {

      .allocate_array = N32PN·allocate_array
      ,.allocate_array_zero = N32PN·allocate_array_zero
      ,.deallocate = N32PN·deallocate

      ,.copy = N32PN·copy
      ,.bit_and = N32PN·bit_and
      ,.bit_or = N32PN·bit_or
      ,.bit_complement = N32PN·bit_complement
      ,.bit_twos_complement = N32PN·bit_twos_complement
      ,.compare = N32PN·compare
      ,.lt = N32PN·lt
      ,.gt = N32PN·gt
      ,.eq = N32PN·eq
      ,.eq_zero = N32PN·eq_zero
      ,.accumulate = N32PN·accumulate
      ,.add = N32PN·add
      ,.increment = N32PN·increment
      ,.subtract = N32PN·subtract
      ,.multiply = N32PN·multiply
      ,.divide = N32PN·divide
      ,.modulus = N32PN·modulus
      ,.shift_left = N32PN·shift_left
      ,.shift_right = N32PN·shift_right
      ,.arithmetic_shift_right = N32PN·arithmetic_shift_right

      ,.access = N32PN·access
      ,.from_uint32 = N32PN·from_uint32
    };

  #endif

#endif
