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

#define N32n·DEBUG

#ifndef FACE
#define N32n·IMPLEMENTATION
#define FACE
#endif

//--------------------------------------------------------------------------------
// Interface

#ifndef N32n·FACE
#define N32n·FACE

  #include <stdint.h>
  #include <stdbool.h>
  #include <stdarg.h>
  #include <stdlib.h>

  //----------------------------------------
  // Instance Data (Declaration Only)

  typedef uint32_t Extent;
  typedef uint32_t Digit;

  typedef struct N32n·T N32n·T;

  extern N32n·T *N32n·zero;
  extern N32n·T *N32n·one;
  extern N32n·T *N32n·all_one_bit;
  extern N32n·T *N32n·lsb;
  extern N32n·T *N32n·msb;

  //----------------------------------------
  // Return/Error Status and handlers

  typedef enum{
    N32n·Status·ok = 0
    ,N32n·Status·overflow = 1
    ,N32n·Status·accumulator1_overflow = 2
    ,N32n·Status·carry = 3
    ,N32n·Status·borrow = 4
    ,N32n·Status·undefined_divide_by_zero = 5
    ,N32n·Status·undefined_modulus_zero = 6
    ,N32n·Status·gt_max_shift_count = 7
    ,N32n·Status·spill_eq_operand = 8 // not currently signaled, result will be spill value
    ,N32n·Status·one_word_product = 9
    ,N32n·Status·two_word_product = 10
  } N32n·Status;

  typedef enum{
    N32n·Order_lt = -1
    ,N32n·Order_eq = 0
    ,N32n·Order_gt = 1
  } N32n·Order;

  typedef N32n·T *( *N32n·Allocate_MemoryFault )(Extent);

  //----------------------------------------
  // Interface

  typedef struct{

    N32n·T *(*allocate_array_zero)(Extent, N32n·Allocate_MemoryFault);
    N32n·T *(*allocate_array)(Extent, N32n·Allocate_MemoryFault);
    void (*deallocate)(N32n·T*);

    void (*copy)(N32n·T*, N32n·T*);
    void (*bit_and)(N32n·T*, N32n·T*, N32n·T*);
    void (*bit_or)(N32n·T*, N32n·T*, N32n·T*);
    void (*bit_complement)(N32n·T*, N32n·T*);
    void (*bit_twos_complement)(N32n·T*, N32n·T*);
    N32n·Order (*compare)(N32n·T*, N32n·T*);
    bool (*lt)(N32n·T*, N32n·T*);
    bool (*gt)(N32n·T*, N32n·T*);
    bool (*eq)(N32n·T*, N32n·T*);
    bool (*eq_zero)(N32n·T*);
    N32n·Status (*accumulate)(N32n·T *accumulator1 ,N32n·T *accumulator0 ,...);
    N32n·Status (*add)(N32n·T*, N32n·T*, N32n·T*);
    bool (*increment)(N32n·T *a);
    N32n·Status (*subtract)(N32n·T*, N32n·T*, N32n·T*);
    N32n·Status (*multiply)(N32n·T*, N32n·T*, N32n·T*, N32n·T*);
    N32n·Status (*divide)(N32n·T*, N32n·T*, N32n·T*, N32n·T*);
    N32n·Status (*modulus)(N32n·T*, N32n·T*, N32n·T*);
    N32n·Status (*shift_left)(Extent, N32n·T*, N32n·T*, N32n·T*);
    N32n·Status (*shift_right)(Extent, N32n·T*, N32n·T*, N32n·T*);
    N32n·Status (*arithmetic_shift_right)(Extent, N32n·T*, N32n·T*);

    N32n·T* (*access)(N32n·T*, Extent);
    void (*from_uint32)(N32n·T *destination ,uint32_t value);
  } N32n·Λ;

  Local const N32n·Λ N32n·λ; // initialized in the LOCAL section

#endif

//--------------------------------------------------------------------------------
// Implementation

#ifdef N32n·IMPLEMENTATION

  // this part goes into the library
  #ifndef LOCAL

    #include <stdarg.h>
    #include <stdlib.h>

    struct N32n·T{
      Digit d0;
    };

    N32n·T N32n·constant[4] = {
      {.d0 = 0},
      {.d0 = 1},
      {.d0 = ~(uint32_t)0},
      {.d0 = 1 << 31}
    };

    N32n·T *N32n·zero = &N32n·constant[0];
    N32n·T *N32n·one = &N32n·constant[1];
    N32n·T *N32n·all_one_bit = &N32n·constant[2];
    N32n·T *N32n·msb = &N32n·constant[3];
    N32n·T *N32n·lsb = &N32n·constant[1];

    // the allocate an array of N32
    N32n·T *N32n·allocate_array(Extent extent ,N32n·Allocate_MemoryFault memory_fault){
      N32n·T *instance = malloc((extent + 1) * sizeof(N32n·T) );
      if(!instance){
        return memory_fault ? memory_fault(extent) : NULL;
      }
      return instance;
    }

    N32n·T *N32n·allocate_array_zero(Extent extent ,N32n·Allocate_MemoryFault memory_fault){
      N32n·T *instance = calloc( extent + 1 ,sizeof(N32n·T) );
      if(!instance){
        return memory_fault ? memory_fault(extent) : NULL;
      }
      return instance;
    }

    void N32n·deallocate(N32n·T *unencumbered){
      free(unencumbered);
    }

  #endif

  // This part is included after the library user's code
  #ifdef LOCAL

    // instance

    struct N32n·T{
      Digit d0;
    };

    // temporary variables
    // making these LOCAL rather than reserving one block in the library is thread safe
    // allocating a block once is more efficient
    // library code writes these, they are not on the interface

    Local N32n·T N32n·t[4];


    // allocation 

    extern N32n·T *N32n·allocate_array(Extent, N32n·Allocate_MemoryFault);
    extern N32n·T *N32n·allocate_array_zero(Extent, N32n·Allocate_MemoryFault);
    extern void N32n·deallocate(N32n·T *);

    // so the user can access numbers in an array allocation
    Local N32n·T* N32n·access(N32n·T *array ,Extent index){
      return &array[index];
    }

    Local void N32n·from_uint32(N32n·T *destination ,uint32_t value){
      if(destination == NULL) return;
      destination->d0 = value;
    }

    // copy, convenience copy

    Local void N32n·copy(N32n·T *destination ,N32n·T *source){
      if(source == destination) return; // that was easy! 
      *destination = *source;
    }

    Local void N32n·set_to_zero(N32n·T *instance){
      instance->d0 = 0;
    }

    Local void N32n·set_to_one(N32n·T *instance){
      instance->d0 = 1;
    }

    // bit operations

    Local void N32n·bit_and(N32n·T *result, N32n·T *a, N32n·T *b){
      result->d0 = a->d0 & b->d0;
    }

    // result can be one of the operands
    Local void N32n·bit_or(N32n·T *result, N32n·T *a, N32n·T *b){
      result->d0 = a->d0 | b->d0;
    }

    // result can the same as the operand
    Local void N32n·bit_complement(N32n·T *result, N32n·T *a){
      result->d0 = ~a->d0;
    }

    // result can the same as the operand
    Local void N32n·bit_twos_complement(N32n·T *result ,N32n·T *a){
      result->d0 = ~a->d0 + 1;
    }

    // test functions

    Local N32n·Order N32n·compare(N32n·T *a, N32n·T *b){
      if(a->d0 < b->d0) return N32n·Order_lt;
      if(a->d0 > b->d0) return N32n·Order_gt;
      return N32n·Order_eq;
    }

    Local bool N32n·lt(N32n·T *a ,N32n·T *b){
      return  a->d0 < b->d0;
    }    

    Local bool N32n·gt(N32n·T *a ,N32n·T *b){
      return  a->d0 > b->d0;
    }    

    Local bool N32n·eq(N32n·T *a ,N32n·T *b){
      return  a->d0 == b->d0;
    }    

    Local bool N32n·eq_zero(N32n·T *a){
      return  a->d0 == 0;
    }    


    // arithmetic operations

    // For a large number of summands for the lower precision Natural implementations, for accumulate/add/sub, the 'overflow' operand could overflow and thus this routine will halt and return N32n·Status·accumulator1_overflow
    //
    // When accumulator1 and accumulator0 point to the same location, the result is the accumulator1 value.
    Local N32n·Status N32n·accumulate(N32n·T *accumulator1 ,N32n·T *accumulator0 ,...){

      va_list args;
      va_start(args ,accumulator0);
      uint32_t sum = accumulator0->d0;
      uint32_t carry = 0;
      N32n·T *current;

      while( (current = va_arg(args ,N32n·T *)) ){
        sum += current->d0;
        if(sum < current->d0){  // Accumulator1 into carry
          (carry)++;
          if(carry == 0){
            va_end(args);
            return N32n·Status·accumulator1_overflow;
          }
        }
      }
      va_end(args);

      // wipes out prior value of accumulator1
      accumulator1->d0 = carry;

      return N32n·Status·ok;
    }

    Local N32n·Status N32n·add(N32n·T *sum ,N32n·T *a ,N32n·T *b){
      uint64_t result = (uint64_t)a->d0 + (uint64_t)b->d0;
      sum->d0 = (uint32_t)result;
      return (result >> 32) ? N32n·Status·carry : N32n·Status·ok;
    }

    Local bool N32n·increment(N32n·T *a){
      a->d0++;
      return a->d0 == 0;
    }

    Local N32n·Status N32n·subtract(N32n·T *difference ,N32n·T *a ,N32n·T *b){
      uint64_t diff = (uint64_t) a->d0 - (uint64_t) b->d0;
      difference->d0 = (uint32_t)diff;
      return (diff > a->d0) ? N32n·Status·borrow : N32n·Status·ok;
    }


    Local N32n·Status N32n·multiply(N32n·T *product1 ,N32n·T *product0 ,N32n·T *a ,N32n·T *b){
      uint64_t product = (uint64_t)a->d0 * (uint64_t)b->d0;
      product0->d0 = (uint32_t)product;
      product1->d0 = (uint32_t)(product >> 32);

      if(product1->d0 == 0) return N32n·Status·one_word_product;
      return N32n·Status·two_word_product;
    }

    Local N32n·Status N32n·divide(N32n·T *remainder ,N32n·T *quotient ,N32n·T *a ,N32n·T *b){
      if(b->d0 == 0) return N32n·Status·undefined_divide_by_zero; 

      quotient->d0 = a->d0 / b->d0;
      remainder->d0 = a->d0 - (quotient->d0 * b->d0);

      return N32n·Status·ok;
    }

    Local N32n·Status N32n·modulus(N32n·T *remainder ,N32n·T *a ,N32n·T *b){
      if(b->d0 == 0) return N32n·Status·undefined_modulus_zero; 
      uint32_t quotient = a->d0 / b->d0;
      remainder->d0 = a->d0 - (quotient * b->d0);
      return N32n·Status·ok;
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
    Local N32n·Status N32n·shift
    (
     uint32_t shift_count
     ,N32n·T *spill
     ,N32n·T *operand
     ,N32n·T *fill
     ,ShiftOp shift_op
     ,ShiftOp complement_shift_op
     ){

      // If no result is needed, return immediately.
      if(operand == NULL && spill == NULL) return N32n·Status·ok;

      // Treat NULL operand as zero.
      if(operand == NULL){
        operand = &N32n·t[0];
        N32n·copy(operand, N32n·zero);
      }

      // Shifting more than one word breaks our fill/spill model.
      if(shift_count > 31) return N32n·Status·gt_max_shift_count;

      // The given operand is still required after it is modified, so we copy it.
      N32n·T *given_operand = &N32n·t[1];
      N32n·copy(given_operand, operand);

      // Perform the shift
      operand->d0 = shift_op(given_operand->d0, shift_count);
      if(fill != NULL){
        fill->d0 = complement_shift_op(fill->d0, (32 - shift_count));
        N32n·bit_or(operand, operand, fill);
      }
      if(spill != NULL){
        spill->d0 = shift_op(spill->d0, shift_count);
        spill->d0 += complement_shift_op(given_operand->d0, (32 - shift_count));
      }

      return N32n·Status·ok;
    }

    // Define concrete shift functions using valid C function pointers
    Local N32n·Status 
    N32n·shift_left(uint32_t shift_count, N32n·T *spill, N32n·T *operand, N32n·T *fill){
      return N32n·shift(shift_count, spill, operand, fill, shift_left_op, shift_right_op);
    }

    Local N32n·Status 
    N32n·shift_right(uint32_t shift_count, N32n·T *spill, N32n·T *operand, N32n·T *fill){
      return N32n·shift(shift_count, spill, operand, fill, shift_right_op, shift_left_op);
    }

    Local N32n·Status 
    N32n·arithmetic_shift_right(uint32_t shift_count, N32n·T *operand, N32n·T *spill){

      // Guard against excessive shift counts
      if(shift_count > 31) return N32n·Status·gt_max_shift_count;

      // A NULL operand is treated as zero
      if(operand == NULL){
        operand = &N32n·t[0];
        N32n·copy(operand, N32n·zero);
      }

      // Pick the fill value based on the sign bit
      N32n·T *fill = (operand->d0 & 0x80000000) ? N32n·all_one_bit : N32n·zero;

      // Call shift_right with the appropriate fill
      return N32n·shift_right(shift_count, spill, operand, fill);
    }

    Local const N32n·Λ N32n·λ = {

      .allocate_array = N32n·allocate_array
      ,.allocate_array_zero = N32n·allocate_array_zero
      ,.deallocate = N32n·deallocate

      ,.copy = N32n·copy
      ,.bit_and = N32n·bit_and
      ,.bit_or = N32n·bit_or
      ,.bit_complement = N32n·bit_complement
      ,.bit_twos_complement = N32n·bit_twos_complement
      ,.compare = N32n·compare
      ,.lt = N32n·lt
      ,.gt = N32n·gt
      ,.eq = N32n·eq
      ,.eq_zero = N32n·eq_zero
      ,.accumulate = N32n·accumulate
      ,.add = N32n·add
      ,.increment = N32n·increment
      ,.subtract = N32n·subtract
      ,.multiply = N32n·multiply
      ,.divide = N32n·divide
      ,.modulus = N32n·modulus
      ,.shift_left = N32n·shift_left
      ,.shift_right = N32n·shift_right
      ,.arithmetic_shift_right = N32n·arithmetic_shift_right

      ,.access = N32n·access
      ,.from_uint32 = N32n·from_uint32
    };

  #endif

#endif
