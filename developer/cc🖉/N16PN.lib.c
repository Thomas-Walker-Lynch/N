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

#define N16PN·DEBUG

#ifndef FACE
#define N16PN·IMPLEMENTATION
#define FACE
#endif

//--------------------------------------------------------------------------------
// Interface

#ifndef N16PN·FACE
#define N16PN·FACE

  #include <stdint.h>
  #include <stdbool.h>
  #include <stdarg.h>
  #include <stdlib.h>

  //----------------------------------------
  // Instance Data (Declaration Only)

  typedef uint16_t Extent;
  typedef uint16_t Digit;

  typedef struct N16PN·T N16PN·T;

  extern N16PN·T *N16PN·zero;
  extern N16PN·T *N16PN·one;
  extern N16PN·T *N16PN·all_one_bit;
  extern N16PN·T *N16PN·lsb;
  extern N16PN·T *N16PN·msb;

  //----------------------------------------
  // Return/Error Status and handlers

  typedef enum{
    N16PN·Status·ok = 0
   ,N16PN·Status·overflow = 1
   ,N16PN·Status·accumulator1_overflow = 2
   ,N16PN·Status·carry = 3
   ,N16PN·Status·borrow = 4
   ,N16PN·Status·undefined_divide_by_zero = 5
   ,N16PN·Status·undefined_modulus_zero = 6
   ,N16PN·Status·gt_max_shift_count = 7
   ,N16PN·Status·spill_eq_operand = 8 // not currently signaled, result will be spill value
   ,N16PN·Status·one_word_product = 9
   ,N16PN·Status·two_word_product = 10
  } N16PN·Status;

  typedef enum{
    N16PN·Order_lt = -1
   ,N16PN·Order_eq = 0
   ,N16PN·Order_gt = 1
  } N16PN·Order;

  typedef N16PN·T *( *N16PN·Allocate_MemoryFault )(Extent);

  //----------------------------------------
  // Interface

  typedef struct{

    N16PN·T *(*allocate_array_zero)(Extent, N16PN·Allocate_MemoryFault);
    N16PN·T *(*allocate_array)(Extent, N16PN·Allocate_MemoryFault);
    void (*deallocate)(N16PN·T*);

    void (*copy)(N16PN·T*, N16PN·T*);
    void (*bit_and)(N16PN·T*, N16PN·T*, N16PN·T*);
    void (*bit_or)(N16PN·T*, N16PN·T*, N16PN·T*);
    void (*bit_complement)(N16PN·T*, N16PN·T*);
    void (*bit_twos_complement)(N16PN·T*, N16PN·T*);
    N16PN·Order (*compare)(N16PN·T*, N16PN·T*);
    bool (*lt)(N16PN·T*, N16PN·T*);
    bool (*gt)(N16PN·T*, N16PN·T*);
    bool (*eq)(N16PN·T*, N16PN·T*);
    bool (*eq_zero)(N16PN·T*);
    N16PN·Status (*accumulate)(N16PN·T *accumulator1 ,N16PN·T *accumulator0 ,...);
    N16PN·Status (*add)(N16PN·T*, N16PN·T*, N16PN·T*);
    bool (*increment)(N16PN·T *a);
    N16PN·Status (*subtract)(N16PN·T*, N16PN·T*, N16PN·T*);
    N16PN·Status (*multiply)(N16PN·T*, N16PN·T*, N16PN·T*, N16PN·T*);
    N16PN·Status (*divide)(N16PN·T*, N16PN·T*, N16PN·T*, N16PN·T*);
    N16PN·Status (*modulus)(N16PN·T*, N16PN·T*, N16PN·T*);
    N16PN·Status (*shift_left)(Extent, N16PN·T*, N16PN·T*, N16PN·T*);
    N16PN·Status (*shift_right)(Extent, N16PN·T*, N16PN·T*, N16PN·T*);
    N16PN·Status (*arithmetic_shift_right)(Extent, N16PN·T*, N16PN·T*);

    N16PN·T* (*access)(N16PN·T*, Extent);
    void (*from_uint32)(N16PN·T *destination ,uint32_t value);
  } N16PN·Λ;

  Local const N16PN·Λ N16PN·λ; // initialized in the LOCAL section

#endif

//--------------------------------------------------------------------------------
// Implementation

#ifdef N16PN·IMPLEMENTATION

  // this part goes into the library
  #ifndef LOCAL

    #include <stdarg.h>
    #include <stdlib.h>

    struct N16PN·T{
      Digit d0;
    };

    N16PN·T N16PN·constant[4] = {
    {.d0 = 0},
    {.d0 = 1},
    {.d0 = ~(uint16_t)0},
    {.d0 = 1 << 15}
    };

    N16PN·T *N16PN·zero = &N16PN·constant[0];
    N16PN·T *N16PN·one = &N16PN·constant[1];
    N16PN·T *N16PN·all_one_bit = &N16PN·constant[2];
    N16PN·T *N16PN·msb = &N16PN·constant[3];
    N16PN·T *N16PN·lsb = &N16PN·constant[1];

    // the allocate an array of N16
    N16PN·T *N16PN·allocate_array(Extent extent ,N16PN·Allocate_MemoryFault memory_fault){
      N16PN·T *instance = malloc((extent + 1) * sizeof(N16PN·T));
      if(!instance){
        return memory_fault ? memory_fault(extent) : NULL;
      }
      return instance;
    }

    N16PN·T *N16PN·allocate_array_zero(Extent extent ,N16PN·Allocate_MemoryFault memory_fault){
      N16PN·T *instance = calloc(extent + 1, sizeof(N16PN·T));
      if(!instance){
        return memory_fault ? memory_fault(extent) : NULL;
      }
      return instance;
    }

    void N16PN·deallocate(N16PN·T *unencumbered){
      free(unencumbered);
    }


  #endif

  // This part is included after the library user's code
  #ifdef LOCAL

    // instance

    struct N16PN·T{
      Digit d0;
    };

    // temporary variables
    Local N16PN·T N16PN·t[4];

    // allocation 

    extern N16PN·T *N16PN·allocate_array(Extent, N16PN·Allocate_MemoryFault);
    extern N16PN·T *N16PN·allocate_array_zero(Extent, N16PN·Allocate_MemoryFault);
    extern void N16PN·deallocate(N16PN·T *);

    // so the user can access numbers in an array allocation
    Local N16PN·T* N16PN·access(N16PN·T *array ,Extent index){
      return &array[index];
    }

    Local void N16PN·from_uint32(N16PN·T *destination ,uint32_t value){
      if(destination == NULL) return;
      destination->d0 = (uint16_t)(value & 0xFFFF);
    }

    // copy, convenience copy

    Local void N16PN·copy(N16PN·T *destination ,N16PN·T *source){
      if(source == destination) return;
      *destination = *source;
    }

    Local void N16PN·set_to_zero(N16PN·T *instance){
      instance->d0 = 0;
    }

    Local void N16PN·set_to_one(N16PN·T *instance){
      instance->d0 = 1;
    }

    // bit operations

    Local void N16PN·bit_and(N16PN·T *result, N16PN·T *a, N16PN·T *b){
      result->d0 = a->d0 & b->d0;
    }

    Local void N16PN·bit_or(N16PN·T *result, N16PN·T *a, N16PN·T *b){
      result->d0 = a->d0 | b->d0;
    }

    Local void N16PN·bit_complement(N16PN·T *result, N16PN·T *a){
      result->d0 = ~a->d0;
    }

    Local void N16PN·bit_twos_complement(N16PN·T *result ,N16PN·T *a){
      result->d0 = (uint16_t)(~a->d0 + 1);
    }

    // test functions

    Local N16PN·Order N16PN·compare(N16PN·T *a, N16PN·T *b){
      if(a->d0 < b->d0) return N16PN·Order_lt;
      if(a->d0 > b->d0) return N16PN·Order_gt;
      return N16PN·Order_eq;
    }

    Local bool N16PN·lt(N16PN·T *a ,N16PN·T *b){
      return a->d0 < b->d0;
    }    

    Local bool N16PN·gt(N16PN·T *a ,N16PN·T *b){
      return a->d0 > b->d0;
    }    

    Local bool N16PN·eq(N16PN·T *a ,N16PN·T *b){
      return a->d0 == b->d0;
    }    

    Local bool N16PN·eq_zero(N16PN·T *a){
      return a->d0 == 0;
    }    

    // arithmetic operations

    Local N16PN·Status N16PN·accumulate(N16PN·T *accumulator1 ,N16PN·T *accumulator0 ,...){

      va_list args;
      va_start(args ,accumulator0);
      uint32_t sum = accumulator0->d0;
      uint32_t carry = 0;
      N16PN·T *current;

      while( (current = va_arg(args ,N16PN·T*)) ){
        sum += current->d0;
        if(sum < current->d0){
          (carry)++;
          if(carry == 0){
            va_end(args);
            return N16PN·Status·accumulator1_overflow;
          }
        }
      }
      va_end(args);

      accumulator1->d0 = (uint16_t)carry;
      return N16PN·Status·ok;
    }

    Local N16PN·Status N16PN·add(N16PN·T *sum ,N16PN·T *a ,N16PN·T *b){
      uint32_t result = (uint32_t)a->d0 + (uint32_t)b->d0;
      sum->d0 = (uint16_t)(result & 0xFFFF);
      return (result >> 16) ? N16PN·Status·carry : N16PN·Status·ok;
    }

    Local bool N16PN·increment(N16PN·T *a){
      a->d0++;
      return (a->d0 == 0);
    }

    Local N16PN·Status N16PN·subtract(N16PN·T *difference ,N16PN·T *a ,N16PN·T *b){
      uint32_t diff = (uint32_t)a->d0 - (uint32_t)b->d0;
      difference->d0 = (uint16_t)(diff & 0xFFFF);
      return (diff > a->d0) ? N16PN·Status·borrow : N16PN·Status·ok;
    }

    Local N16PN·Status N16PN·multiply(N16PN·T *product1 ,N16PN·T *product0 ,N16PN·T *a ,N16PN·T *b){
      uint32_t product = (uint32_t)a->d0 * (uint32_t)b->d0;
      product0->d0 = (uint16_t)(product & 0xFFFF);
      product1->d0 = (uint16_t)((product >> 16) & 0xFFFF);

      if(product1->d0 == 0) return N16PN·Status·one_word_product;
      return N16PN·Status·two_word_product;
    }

    Local N16PN·Status N16PN·divide(N16PN·T *remainder ,N16PN·T *quotient ,N16PN·T *a ,N16PN·T *b){
      if(b->d0 == 0) return N16PN·Status·undefined_divide_by_zero;

      uint32_t dividend = a->d0;
      uint32_t divisor = b->d0;
      quotient->d0 = (uint16_t)(dividend / divisor);
      remainder->d0 = (uint16_t)(dividend - (quotient->d0 * divisor));

      return N16PN·Status·ok;
    }

    Local N16PN·Status N16PN·modulus(N16PN·T *remainder ,N16PN·T *a ,N16PN·T *b){
      if(b->d0 == 0) return N16PN·Status·undefined_modulus_zero;
      uint32_t dividend = a->d0;
      uint32_t divisor = b->d0;
      uint32_t q = dividend / divisor;
      remainder->d0 = (uint16_t)(dividend - (q * divisor));
      return N16PN·Status·ok;
    }

    // bit motion

    typedef uint16_t (*ShiftOp)(uint16_t, uint16_t);

    Local uint16_t shift_left_op(uint16_t value, uint16_t amount){
      return value << amount;
    }

    Local uint16_t shift_right_op(uint16_t value, uint16_t amount){
      return (uint16_t)(value >> amount);
    }

    Local N16PN·Status N16PN·shift
    (
     uint16_t shift_count
     ,N16PN·T *spill
     ,N16PN·T *operand
     ,N16PN·T *fill
     ,ShiftOp shift_op
     ,ShiftOp complement_shift_op
    ){

      if(operand == NULL && spill == NULL) return N16PN·Status·ok;

      if(operand == NULL){
        operand = &N16PN·t[0];
        N16PN·copy(operand, N16PN·zero);
      }

      if(shift_count > 15) return N16PN·Status·gt_max_shift_count;

      N16PN·T *given_operand = &N16PN·t[1];
      N16PN·copy(given_operand, operand);

      operand->d0 = shift_op(given_operand->d0, shift_count);
      if(fill != NULL){
        fill->d0 = complement_shift_op(fill->d0, (16 - shift_count));
        N16PN·bit_or(operand, operand, fill);
      }
      if(spill != NULL){
        spill->d0 = shift_op(spill->d0, shift_count);
        spill->d0 += complement_shift_op(given_operand->d0, (16 - shift_count));
      }

      return N16PN·Status·ok;
    }

    Local N16PN·Status 
    N16PN·shift_left(uint16_t shift_count, N16PN·T *spill, N16PN·T *operand, N16PN·T *fill){
      return N16PN·shift(shift_count, spill, operand, fill, shift_left_op, shift_right_op);
    }

    Local N16PN·Status 
    N16PN·shift_right(uint16_t shift_count, N16PN·T *spill, N16PN·T *operand, N16PN·T *fill){
      return N16PN·shift(shift_count, spill, operand, fill, shift_right_op, shift_left_op);
    }

    Local N16PN·Status 
    N16PN·arithmetic_shift_right(uint16_t shift_count, N16PN·T *operand, N16PN·T *spill){

      if(shift_count > 15) return N16PN·Status·gt_max_shift_count;

      if(operand == NULL){
        operand = &N16PN·t[0];
        N16PN·copy(operand, N16PN·zero);
      }

      N16PN·T *fill = (operand->d0 & 0x8000) ? N16PN·all_one_bit : N16PN·zero;
      return N16PN·shift_right(shift_count, spill, operand, fill);
    }

    Local const N16PN·Λ N16PN·λ = {

      .allocate_array = N16PN·allocate_array
     ,.allocate_array_zero = N16PN·allocate_array_zero
     ,.deallocate = N16PN·deallocate

     ,.copy = N16PN·copy
     ,.bit_and = N16PN·bit_and
     ,.bit_or = N16PN·bit_or
     ,.bit_complement = N16PN·bit_complement
     ,.bit_twos_complement = N16PN·bit_twos_complement
     ,.compare = N16PN·compare
     ,.lt = N16PN·lt
     ,.gt = N16PN·gt
     ,.eq = N16PN·eq
     ,.eq_zero = N16PN·eq_zero
     ,.accumulate = N16PN·accumulate
     ,.add = N16PN·add
     ,.increment = N16PN·increment
     ,.subtract = N16PN·subtract
     ,.multiply = N16PN·multiply
     ,.divide = N16PN·divide
     ,.modulus = N16PN·modulus
     ,.shift_left = N16PN·shift_left
     ,.shift_right = N16PN·shift_right
     ,.arithmetic_shift_right = N16PN·arithmetic_shift_right

     ,.access = N16PN·access
     ,.from_uint32 = N16PN·from_uint32
    };

  #endif

#endif
