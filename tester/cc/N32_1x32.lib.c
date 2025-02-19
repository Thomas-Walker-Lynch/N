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

  typedef uint32_t Extent;
  typedef uint32_t Digit;

  typedef struct N32_1x32·T N32_1x32·T;

  extern N32_1x32·T *N32_1x32·zero;
  extern N32_1x32·T *N32_1x32·one;
  extern N32_1x32·T *N32_1x32·all_one_bit;
  extern N32_1x32·T *N32_1x32·lsb;
  extern N32_1x32·T *N32_1x32·msb;

  //----------------------------------------
  // Return/Error Status and handlers

  typedef enum{
    N32_1x32·Status·ok = 0
    ,N32_1x32·Status·overflow = 1
    ,N32_1x32·Status·accumulator1_overflow = 2
    ,N32_1x32·Status·carry = 3
    ,N32_1x32·Status·borrow = 4
    ,N32_1x32·Status·undefined_divide_by_zero = 5
    ,N32_1x32·Status·undefined_modulus_zero = 6
    ,N32_1x32·Status·gt_max_shift_count = 7
    ,N32_1x32·Status·spill_eq_operand = 8 // not currently signaled, result will be spill value
    ,N32_1x32·Status·one_word_product = 9
    ,N32_1x32·Status·two_word_product = 10
  } N32_1x32·Status;

  typedef enum{
    N32_1x32·Order_lt = -1
    ,N32_1x32·Order_eq = 0
    ,N32_1x32·Order_gt = 1
  } N32_1x32·Order;

  typedef N32_1x32·T *( *N32_1x32·Allocate_MemoryFault )(Extent);

  //----------------------------------------
  // Interface

  typedef struct{

    N32_1x32·T *(*allocate_array_zero)(Extent, N32_1x32·Allocate_MemoryFault);
    N32_1x32·T *(*allocate_array)(Extent, N32_1x32·Allocate_MemoryFault);
    void (*deallocate)(N32_1x32·T*);

    void (*copy)(N32_1x32·T*, N32_1x32·T*);
    void (*bit_and)(N32_1x32·T*, N32_1x32·T*, N32_1x32·T*);
    void (*bit_or)(N32_1x32·T*, N32_1x32·T*, N32_1x32·T*);
    void (*bit_complement)(N32_1x32·T*, N32_1x32·T*);
    void (*bit_twos_complement)(N32_1x32·T*, N32_1x32·T*);
    N32_1x32·Order (*compare)(N32_1x32·T*, N32_1x32·T*);
    bool (*lt)(N32_1x32·T*, N32_1x32·T*);
    bool (*gt)(N32_1x32·T*, N32_1x32·T*);
    bool (*eq)(N32_1x32·T*, N32_1x32·T*);
    bool (*eq_zero)(N32_1x32·T*);
    N32_1x32·Status (*accumulate)(N32_1x32·T *accumulator1 ,N32_1x32·T *accumulator0 ,...);
    N32_1x32·Status (*add)(N32_1x32·T*, N32_1x32·T*, N32_1x32·T*);
    bool (*increment)(N32_1x32·T *a);
    N32_1x32·Status (*subtract)(N32_1x32·T*, N32_1x32·T*, N32_1x32·T*);
    N32_1x32·Status (*multiply)(N32_1x32·T*, N32_1x32·T*, N32_1x32·T*, N32_1x32·T*);
    N32_1x32·Status (*divide)(N32_1x32·T*, N32_1x32·T*, N32_1x32·T*, N32_1x32·T*);
    N32_1x32·Status (*modulus)(N32_1x32·T*, N32_1x32·T*, N32_1x32·T*);
    N32_1x32·Status (*shift_left)(Extent, N32_1x32·T*, N32_1x32·T*, N32_1x32·T*);
    N32_1x32·Status (*shift_right)(Extent, N32_1x32·T*, N32_1x32·T*, N32_1x32·T*);
    N32_1x32·Status (*arithmetic_shift_right)(Extent, N32_1x32·T*, N32_1x32·T*);

    N32_1x32·T* (*access)(N32_1x32·T*, Extent);
    void (*from_uint32)(N32_1x32·T *destination ,uint32_t value);
  } N32_1x32·M;

  Local const N32_1x32·M N32_1x32·m; // initialized in the LOCAL section

#endif

//--------------------------------------------------------------------------------
// Implementation

#ifdef N32_1x32·IMPLEMENTATION

  // this part goes into the library
  #ifndef LOCAL

    #include <stdarg.h>
    #include <stdlib.h>

    struct N32_1x32·T{
      Digit d0;
    };

    N32_1x32·T N32_1x32·constant[4] = {
      {.d0 = 0},
      {.d0 = 1},
      {.d0 = ~(uint32_t)0},
      {.d0 = 1 << 31}
    };

    N32_1x32·T *N32_1x32·zero = &N32_1x32·constant[0];
    N32_1x32·T *N32_1x32·one = &N32_1x32·constant[1];
    N32_1x32·T *N32_1x32·all_one_bit = &N32_1x32·constant[2];
    N32_1x32·T *N32_1x32·msb = &N32_1x32·constant[3];
    N32_1x32·T *N32_1x32·lsb = &N32_1x32·constant[1];

    // the allocate an array of N32
    N32_1x32·T *N32_1x32·allocate_array(Extent extent ,N32_1x32·Allocate_MemoryFault memory_fault){
      N32_1x32·T *instance = malloc((extent + 1) * sizeof(N32_1x32·T) );
      if(!instance){
        return memory_fault ? memory_fault(extent) : NULL;
      }
      return instance;
    }

    N32_1x32·T *N32_1x32·allocate_array_zero(Extent extent ,N32_1x32·Allocate_MemoryFault memory_fault){
      N32_1x32·T *instance = calloc( extent + 1 ,sizeof(N32_1x32·T) );
      if(!instance){
        return memory_fault ? memory_fault(extent) : NULL;
      }
      return instance;
    }

    void N32_1x32·deallocate(N32_1x32·T *unencumbered){
      free(unencumbered);
    }

  #endif

  // This part is included after the library user's code
  #ifdef LOCAL

    // instance

    struct N32_1x32·T{
      Digit d0;
    };

    // temporary variables
    // making these LOCAL rather than reserving one block in the library is thread safe
    // allocating a block once is more efficient
    // library code writes these, they are not on the interface

    Local N32_1x32·T N32_1x32·t[4];


    // allocation 

    extern N32_1x32·T *N32_1x32·allocate_array(Extent, N32_1x32·Allocate_MemoryFault);
    extern N32_1x32·T *N32_1x32·allocate_array_zero(Extent, N32_1x32·Allocate_MemoryFault);
    extern void N32_1x32·deallocate(N32_1x32·T *);

    // so the user can access numbers in an array allocation
    Local N32_1x32·T* N32_1x32·access(N32_1x32·T *array ,Extent index){
      return &array[index];
    }

    Local void N32_1x32·from_uint32(N32_1x32·T *destination ,uint32_t value){
      if(destination == NULL) return;
      destination->d0 = value;
    }

    // copy, convenience copy

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
    };

  #endif

#endif
