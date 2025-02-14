/*
  N16 - a processor native type

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

#define N16·DEBUG

#ifndef FACE
#define N16·IMPLEMENTATION
#define FACE
#endif

//--------------------------------------------------------------------------------
// Interface

#ifndef N16·FACE
#define N16·FACE

  #include <stdint.h>
  #include <stdbool.h>
  #include <stdarg.h>
  #include <stdlib.h>

  //----------------------------------------
  // Instance Data (Declaration Only)

  typedef uint16_t Extent;
  typedef uint16_t Digit;

  typedef struct N16·T N16·T;

  extern N16·T *N16·zero;
  extern N16·T *N16·one;
  extern N16·T *N16·all_one_bit;
  extern N16·T *N16·lsb;
  extern N16·T *N16·msb;

  //----------------------------------------
  // Return/Error Status and handlers

  typedef enum{
    N16·Status·ok = 0
   ,N16·Status·overflow = 1
   ,N16·Status·accumulator1_overflow = 2
   ,N16·Status·carry = 3
   ,N16·Status·borrow = 4
   ,N16·Status·undefined_divide_by_zero = 5
   ,N16·Status·undefined_modulus_zero = 6
   ,N16·Status·gt_max_shift_count = 7
   ,N16·Status·spill_eq_operand = 8 // not currently signaled, result will be spill value
   ,N16·Status·one_word_product = 9
   ,N16·Status·two_word_product = 10
  } N16·Status;

  typedef enum{
    N16·Order_lt = -1
   ,N16·Order_eq = 0
   ,N16·Order_gt = 1
  } N16·Order;

  typedef N16·T *( *N16·Allocate_MemoryFault )(Extent);

  //----------------------------------------
  // Interface

  typedef struct{

    N16·T *(*allocate_array_zero)(Extent, N16·Allocate_MemoryFault);
    N16·T *(*allocate_array)(Extent, N16·Allocate_MemoryFault);
    void (*deallocate)(N16·T*);

    void (*copy)(N16·T*, N16·T*);
    void (*bit_and)(N16·T*, N16·T*, N16·T*);
    void (*bit_or)(N16·T*, N16·T*, N16·T*);
    void (*bit_complement)(N16·T*, N16·T*);
    void (*bit_twos_complement)(N16·T*, N16·T*);
    N16·Order (*compare)(N16·T*, N16·T*);
    bool (*lt)(N16·T*, N16·T*);
    bool (*gt)(N16·T*, N16·T*);
    bool (*eq)(N16·T*, N16·T*);
    bool (*eq_zero)(N16·T*);
    N16·Status (*accumulate)(N16·T *accumulator1 ,N16·T *accumulator0 ,...);
    N16·Status (*add)(N16·T*, N16·T*, N16·T*);
    bool (*increment)(N16·T *a);
    N16·Status (*subtract)(N16·T*, N16·T*, N16·T*);
    N16·Status (*multiply)(N16·T*, N16·T*, N16·T*, N16·T*);
    N16·Status (*divide)(N16·T*, N16·T*, N16·T*, N16·T*);
    N16·Status (*modulus)(N16·T*, N16·T*, N16·T*);
    N16·Status (*shift_left)(Extent, N16·T*, N16·T*, N16·T*);
    N16·Status (*shift_right)(Extent, N16·T*, N16·T*, N16·T*);
    N16·Status (*arithmetic_shift_right)(Extent, N16·T*, N16·T*);

    N16·T* (*access)(N16·T*, Extent);
    void (*from_uint32)(N16·T *destination ,uint32_t value);
  } N16·Λ;

  Local const N16·Λ N16·λ; // initialized in the LOCAL section

#endif

//--------------------------------------------------------------------------------
// Implementation

#ifdef N16·IMPLEMENTATION

  // this part goes into the library
  #ifndef LOCAL

    #include <stdarg.h>
    #include <stdlib.h>

    struct N16·T{
      Digit d0;
    };

    N16·T N16·constant[4] = {
    {.d0 = 0},
    {.d0 = 1},
    {.d0 = ~(uint16_t)0},
    {.d0 = 1 << 15}
    };

    N16·T *N16·zero = &N16·constant[0];
    N16·T *N16·one = &N16·constant[1];
    N16·T *N16·all_one_bit = &N16·constant[2];
    N16·T *N16·msb = &N16·constant[3];
    N16·T *N16·lsb = &N16·constant[1];

    // the allocate an array of N16
    N16·T *N16·allocate_array(Extent extent ,N16·Allocate_MemoryFault memory_fault){
      N16·T *instance = malloc((extent + 1) * sizeof(N16·T));
      if(!instance){
        return memory_fault ? memory_fault(extent) : NULL;
      }
      return instance;
    }

    N16·T *N16·allocate_array_zero(Extent extent ,N16·Allocate_MemoryFault memory_fault){
      N16·T *instance = calloc(extent + 1, sizeof(N16·T));
      if(!instance){
        return memory_fault ? memory_fault(extent) : NULL;
      }
      return instance;
    }

    void N16·deallocate(N16·T *unencumbered){
      free(unencumbered);
    }


  #endif

  // This part is included after the library user's code
  #ifdef LOCAL

    // instance

    struct N16·T{
      Digit d0;
    };

    // temporary variables
    Local N16·T N16·t[4];

    // allocation 

    extern N16·T *N16·allocate_array(Extent, N16·Allocate_MemoryFault);
    extern N16·T *N16·allocate_array_zero(Extent, N16·Allocate_MemoryFault);
    extern void N16·deallocate(N16·T *);

    // so the user can access numbers in an array allocation
    Local N16·T* N16·access(N16·T *array ,Extent index){
      return &array[index];
    }

    Local void N16·from_uint32(N16·T *destination ,uint32_t value){
      if(destination == NULL) return;
      destination->d0 = (uint16_t)(value & 0xFFFF);
    }

    // copy, convenience copy

    Local void N16·copy(N16·T *destination ,N16·T *source){
      if(source == destination) return;
      *destination = *source;
    }

    Local void N16·set_to_zero(N16·T *instance){
      instance->d0 = 0;
    }

    Local void N16·set_to_one(N16·T *instance){
      instance->d0 = 1;
    }

    // bit operations

    Local void N16·bit_and(N16·T *result, N16·T *a, N16·T *b){
      result->d0 = a->d0 & b->d0;
    }

    Local void N16·bit_or(N16·T *result, N16·T *a, N16·T *b){
      result->d0 = a->d0 | b->d0;
    }

    Local void N16·bit_complement(N16·T *result, N16·T *a){
      result->d0 = ~a->d0;
    }

    Local void N16·bit_twos_complement(N16·T *result ,N16·T *a){
      result->d0 = (uint16_t)(~a->d0 + 1);
    }

    // test functions

    Local N16·Order N16·compare(N16·T *a, N16·T *b){
      if(a->d0 < b->d0) return N16·Order_lt;
      if(a->d0 > b->d0) return N16·Order_gt;
      return N16·Order_eq;
    }

    Local bool N16·lt(N16·T *a ,N16·T *b){
      return a->d0 < b->d0;
    }    

    Local bool N16·gt(N16·T *a ,N16·T *b){
      return a->d0 > b->d0;
    }    

    Local bool N16·eq(N16·T *a ,N16·T *b){
      return a->d0 == b->d0;
    }    

    Local bool N16·eq_zero(N16·T *a){
      return a->d0 == 0;
    }    

    // arithmetic operations

    Local N16·Status N16·accumulate(N16·T *accumulator1 ,N16·T *accumulator0 ,...){

      va_list args;
      va_start(args ,accumulator0);
      uint32_t sum = accumulator0->d0;
      uint32_t carry = 0;
      N16·T *current;

      while( (current = va_arg(args ,N16·T*)) ){
        sum += current->d0;
        if(sum < current->d0){
          (carry)++;
          if(carry == 0){
            va_end(args);
            return N16·Status·accumulator1_overflow;
          }
        }
      }
      va_end(args);

      accumulator1->d0 = (uint16_t)carry;
      return N16·Status·ok;
    }

    Local N16·Status N16·add(N16·T *sum ,N16·T *a ,N16·T *b){
      uint32_t result = (uint32_t)a->d0 + (uint32_t)b->d0;
      sum->d0 = (uint16_t)(result & 0xFFFF);
      return (result >> 16) ? N16·Status·carry : N16·Status·ok;
    }

    Local bool N16·increment(N16·T *a){
      a->d0++;
      return (a->d0 == 0);
    }

    Local N16·Status N16·subtract(N16·T *difference ,N16·T *a ,N16·T *b){
      uint32_t diff = (uint32_t)a->d0 - (uint32_t)b->d0;
      difference->d0 = (uint16_t)(diff & 0xFFFF);
      return (diff > a->d0) ? N16·Status·borrow : N16·Status·ok;
    }

    Local N16·Status N16·multiply(N16·T *product1 ,N16·T *product0 ,N16·T *a ,N16·T *b){
      uint32_t product = (uint32_t)a->d0 * (uint32_t)b->d0;
      product0->d0 = (uint16_t)(product & 0xFFFF);
      product1->d0 = (uint16_t)((product >> 16) & 0xFFFF);

      if(product1->d0 == 0) return N16·Status·one_word_product;
      return N16·Status·two_word_product;
    }

    Local N16·Status N16·divide(N16·T *remainder ,N16·T *quotient ,N16·T *a ,N16·T *b){
      if(b->d0 == 0) return N16·Status·undefined_divide_by_zero;

      uint32_t dividend = a->d0;
      uint32_t divisor = b->d0;
      quotient->d0 = (uint16_t)(dividend / divisor);
      remainder->d0 = (uint16_t)(dividend - (quotient->d0 * divisor));

      return N16·Status·ok;
    }

    Local N16·Status N16·modulus(N16·T *remainder ,N16·T *a ,N16·T *b){
      if(b->d0 == 0) return N16·Status·undefined_modulus_zero;
      uint32_t dividend = a->d0;
      uint32_t divisor = b->d0;
      uint32_t q = dividend / divisor;
      remainder->d0 = (uint16_t)(dividend - (q * divisor));
      return N16·Status·ok;
    }

    // bit motion

    typedef uint16_t (*ShiftOp)(uint16_t, uint16_t);

    Local uint16_t shift_left_op(uint16_t value, uint16_t amount){
      return value << amount;
    }

    Local uint16_t shift_right_op(uint16_t value, uint16_t amount){
      return (uint16_t)(value >> amount);
    }

    Local N16·Status N16·shift
    (
     uint16_t shift_count
     ,N16·T *spill
     ,N16·T *operand
     ,N16·T *fill
     ,ShiftOp shift_op
     ,ShiftOp complement_shift_op
    ){

      if(operand == NULL && spill == NULL) return N16·Status·ok;

      if(operand == NULL){
        operand = &N16·t[0];
        N16·copy(operand, N16·zero);
      }

      if(shift_count > 15) return N16·Status·gt_max_shift_count;

      N16·T *given_operand = &N16·t[1];
      N16·copy(given_operand, operand);

      operand->d0 = shift_op(given_operand->d0, shift_count);
      if(fill != NULL){
        fill->d0 = complement_shift_op(fill->d0, (16 - shift_count));
        N16·bit_or(operand, operand, fill);
      }
      if(spill != NULL){
        spill->d0 = shift_op(spill->d0, shift_count);
        spill->d0 += complement_shift_op(given_operand->d0, (16 - shift_count));
      }

      return N16·Status·ok;
    }

    Local N16·Status 
    N16·shift_left(uint16_t shift_count, N16·T *spill, N16·T *operand, N16·T *fill){
      return N16·shift(shift_count, spill, operand, fill, shift_left_op, shift_right_op);
    }

    Local N16·Status 
    N16·shift_right(uint16_t shift_count, N16·T *spill, N16·T *operand, N16·T *fill){
      return N16·shift(shift_count, spill, operand, fill, shift_right_op, shift_left_op);
    }

    Local N16·Status 
    N16·arithmetic_shift_right(uint16_t shift_count, N16·T *operand, N16·T *spill){

      if(shift_count > 15) return N16·Status·gt_max_shift_count;

      if(operand == NULL){
        operand = &N16·t[0];
        N16·copy(operand, N16·zero);
      }

      N16·T *fill = (operand->d0 & 0x8000) ? N16·all_one_bit : N16·zero;
      return N16·shift_right(shift_count, spill, operand, fill);
    }

    Local const N16·Λ N16·λ = {

      .allocate_array = N16·allocate_array
     ,.allocate_array_zero = N16·allocate_array_zero
     ,.deallocate = N16·deallocate

     ,.copy = N16·copy
     ,.bit_and = N16·bit_and
     ,.bit_or = N16·bit_or
     ,.bit_complement = N16·bit_complement
     ,.bit_twos_complement = N16·bit_twos_complement
     ,.compare = N16·compare
     ,.lt = N16·lt
     ,.gt = N16·gt
     ,.eq = N16·eq
     ,.eq_zero = N16·eq_zero
     ,.accumulate = N16·accumulate
     ,.add = N16·add
     ,.increment = N16·increment
     ,.subtract = N16·subtract
     ,.multiply = N16·multiply
     ,.divide = N16·divide
     ,.modulus = N16·modulus
     ,.shift_left = N16·shift_left
     ,.shift_right = N16·shift_right
     ,.arithmetic_shift_right = N16·arithmetic_shift_right

     ,.access = N16·access
     ,.from_uint32 = N16·from_uint32
    };

  #endif

#endif
