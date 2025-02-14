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

#define N16n·DEBUG

#ifndef FACE
#define N16n·IMPLEMENTATION
#define FACE
#endif

//--------------------------------------------------------------------------------
// Interface

#ifndef N16n·FACE
#define N16n·FACE

  #include <stdint.h>
  #include <stdbool.h>
  #include <stdarg.h>
  #include <stdlib.h>

  //----------------------------------------
  // Instance Data (Declaration Only)

  typedef uint16_t Extent;
  typedef uint16_t Digit;

  typedef struct N16n·T N16n·T;

  extern N16n·T *N16n·zero;
  extern N16n·T *N16n·one;
  extern N16n·T *N16n·all_one_bit;
  extern N16n·T *N16n·lsb;
  extern N16n·T *N16n·msb;

  //----------------------------------------
  // Return/Error Status and handlers

  typedef enum{
    N16n·Status·ok = 0
   ,N16n·Status·overflow = 1
   ,N16n·Status·accumulator1_overflow = 2
   ,N16n·Status·carry = 3
   ,N16n·Status·borrow = 4
   ,N16n·Status·undefined_divide_by_zero = 5
   ,N16n·Status·undefined_modulus_zero = 6
   ,N16n·Status·gt_max_shift_count = 7
   ,N16n·Status·spill_eq_operand = 8 // not currently signaled, result will be spill value
   ,N16n·Status·one_word_product = 9
   ,N16n·Status·two_word_product = 10
  } N16n·Status;

  typedef enum{
    N16n·Order_lt = -1
   ,N16n·Order_eq = 0
   ,N16n·Order_gt = 1
  } N16n·Order;

  typedef N16n·T *( *N16n·Allocate_MemoryFault )(Extent);

  //----------------------------------------
  // Interface

  typedef struct{

    N16n·T *(*allocate_array_zero)(Extent, N16n·Allocate_MemoryFault);
    N16n·T *(*allocate_array)(Extent, N16n·Allocate_MemoryFault);
    void (*deallocate)(N16n·T*);

    void (*copy)(N16n·T*, N16n·T*);
    void (*bit_and)(N16n·T*, N16n·T*, N16n·T*);
    void (*bit_or)(N16n·T*, N16n·T*, N16n·T*);
    void (*bit_complement)(N16n·T*, N16n·T*);
    void (*bit_twos_complement)(N16n·T*, N16n·T*);
    N16n·Order (*compare)(N16n·T*, N16n·T*);
    bool (*lt)(N16n·T*, N16n·T*);
    bool (*gt)(N16n·T*, N16n·T*);
    bool (*eq)(N16n·T*, N16n·T*);
    bool (*eq_zero)(N16n·T*);
    N16n·Status (*accumulate)(N16n·T *accumulator1 ,N16n·T *accumulator0 ,...);
    N16n·Status (*add)(N16n·T*, N16n·T*, N16n·T*);
    bool (*increment)(N16n·T *a);
    N16n·Status (*subtract)(N16n·T*, N16n·T*, N16n·T*);
    N16n·Status (*multiply)(N16n·T*, N16n·T*, N16n·T*, N16n·T*);
    N16n·Status (*divide)(N16n·T*, N16n·T*, N16n·T*, N16n·T*);
    N16n·Status (*modulus)(N16n·T*, N16n·T*, N16n·T*);
    N16n·Status (*shift_left)(Extent, N16n·T*, N16n·T*, N16n·T*);
    N16n·Status (*shift_right)(Extent, N16n·T*, N16n·T*, N16n·T*);
    N16n·Status (*arithmetic_shift_right)(Extent, N16n·T*, N16n·T*);

    N16n·T* (*access)(N16n·T*, Extent);
    void (*from_uint32)(N16n·T *destination ,uint32_t value);
  } N16n·Λ;

  Local const N16n·Λ N16n·λ; // initialized in the LOCAL section

#endif

//--------------------------------------------------------------------------------
// Implementation

#ifdef N16n·IMPLEMENTATION

  // this part goes into the library
  #ifndef LOCAL

    #include <stdarg.h>
    #include <stdlib.h>

    struct N16n·T{
      Digit d0;
    };

    N16n·T N16n·constant[4] = {
    {.d0 = 0},
    {.d0 = 1},
    {.d0 = ~(uint16_t)0},
    {.d0 = 1 << 15}
    };

    N16n·T *N16n·zero = &N16n·constant[0];
    N16n·T *N16n·one = &N16n·constant[1];
    N16n·T *N16n·all_one_bit = &N16n·constant[2];
    N16n·T *N16n·msb = &N16n·constant[3];
    N16n·T *N16n·lsb = &N16n·constant[1];

    // the allocate an array of N16
    N16n·T *N16n·allocate_array(Extent extent ,N16n·Allocate_MemoryFault memory_fault){
      N16n·T *instance = malloc((extent + 1) * sizeof(N16n·T));
      if(!instance){
        return memory_fault ? memory_fault(extent) : NULL;
      }
      return instance;
    }

    N16n·T *N16n·allocate_array_zero(Extent extent ,N16n·Allocate_MemoryFault memory_fault){
      N16n·T *instance = calloc(extent + 1, sizeof(N16n·T));
      if(!instance){
        return memory_fault ? memory_fault(extent) : NULL;
      }
      return instance;
    }

    void N16n·deallocate(N16n·T *unencumbered){
      free(unencumbered);
    }


  #endif

  // This part is included after the library user's code
  #ifdef LOCAL

    // instance

    struct N16n·T{
      Digit d0;
    };

    // temporary variables
    Local N16n·T N16n·t[4];

    // allocation 

    extern N16n·T *N16n·allocate_array(Extent, N16n·Allocate_MemoryFault);
    extern N16n·T *N16n·allocate_array_zero(Extent, N16n·Allocate_MemoryFault);
    extern void N16n·deallocate(N16n·T *);

    // so the user can access numbers in an array allocation
    Local N16n·T* N16n·access(N16n·T *array ,Extent index){
      return &array[index];
    }

    Local void N16n·from_uint32(N16n·T *destination ,uint32_t value){
      if(destination == NULL) return;
      destination->d0 = (uint16_t)(value & 0xFFFF);
    }

    // copy, convenience copy

    Local void N16n·copy(N16n·T *destination ,N16n·T *source){
      if(source == destination) return;
      *destination = *source;
    }

    Local void N16n·set_to_zero(N16n·T *instance){
      instance->d0 = 0;
    }

    Local void N16n·set_to_one(N16n·T *instance){
      instance->d0 = 1;
    }

    // bit operations

    Local void N16n·bit_and(N16n·T *result, N16n·T *a, N16n·T *b){
      result->d0 = a->d0 & b->d0;
    }

    Local void N16n·bit_or(N16n·T *result, N16n·T *a, N16n·T *b){
      result->d0 = a->d0 | b->d0;
    }

    Local void N16n·bit_complement(N16n·T *result, N16n·T *a){
      result->d0 = ~a->d0;
    }

    Local void N16n·bit_twos_complement(N16n·T *result ,N16n·T *a){
      result->d0 = (uint16_t)(~a->d0 + 1);
    }

    // test functions

    Local N16n·Order N16n·compare(N16n·T *a, N16n·T *b){
      if(a->d0 < b->d0) return N16n·Order_lt;
      if(a->d0 > b->d0) return N16n·Order_gt;
      return N16n·Order_eq;
    }

    Local bool N16n·lt(N16n·T *a ,N16n·T *b){
      return a->d0 < b->d0;
    }    

    Local bool N16n·gt(N16n·T *a ,N16n·T *b){
      return a->d0 > b->d0;
    }    

    Local bool N16n·eq(N16n·T *a ,N16n·T *b){
      return a->d0 == b->d0;
    }    

    Local bool N16n·eq_zero(N16n·T *a){
      return a->d0 == 0;
    }    

    // arithmetic operations

    Local N16n·Status N16n·accumulate(N16n·T *accumulator1 ,N16n·T *accumulator0 ,...){

      va_list args;
      va_start(args ,accumulator0);
      uint32_t sum = accumulator0->d0;
      uint32_t carry = 0;
      N16n·T *current;

      while( (current = va_arg(args ,N16n·T*)) ){
        sum += current->d0;
        if(sum < current->d0){
          (carry)++;
          if(carry == 0){
            va_end(args);
            return N16n·Status·accumulator1_overflow;
          }
        }
      }
      va_end(args);

      accumulator1->d0 = (uint16_t)carry;
      return N16n·Status·ok;
    }

    Local N16n·Status N16n·add(N16n·T *sum ,N16n·T *a ,N16n·T *b){
      uint32_t result = (uint32_t)a->d0 + (uint32_t)b->d0;
      sum->d0 = (uint16_t)(result & 0xFFFF);
      return (result >> 16) ? N16n·Status·carry : N16n·Status·ok;
    }

    Local bool N16n·increment(N16n·T *a){
      a->d0++;
      return (a->d0 == 0);
    }

    Local N16n·Status N16n·subtract(N16n·T *difference ,N16n·T *a ,N16n·T *b){
      uint32_t diff = (uint32_t)a->d0 - (uint32_t)b->d0;
      difference->d0 = (uint16_t)(diff & 0xFFFF);
      return (diff > a->d0) ? N16n·Status·borrow : N16n·Status·ok;
    }

    Local N16n·Status N16n·multiply(N16n·T *product1 ,N16n·T *product0 ,N16n·T *a ,N16n·T *b){
      uint32_t product = (uint32_t)a->d0 * (uint32_t)b->d0;
      product0->d0 = (uint16_t)(product & 0xFFFF);
      product1->d0 = (uint16_t)((product >> 16) & 0xFFFF);

      if(product1->d0 == 0) return N16n·Status·one_word_product;
      return N16n·Status·two_word_product;
    }

    Local N16n·Status N16n·divide(N16n·T *remainder ,N16n·T *quotient ,N16n·T *a ,N16n·T *b){
      if(b->d0 == 0) return N16n·Status·undefined_divide_by_zero;

      uint32_t dividend = a->d0;
      uint32_t divisor = b->d0;
      quotient->d0 = (uint16_t)(dividend / divisor);
      remainder->d0 = (uint16_t)(dividend - (quotient->d0 * divisor));

      return N16n·Status·ok;
    }

    Local N16n·Status N16n·modulus(N16n·T *remainder ,N16n·T *a ,N16n·T *b){
      if(b->d0 == 0) return N16n·Status·undefined_modulus_zero;
      uint32_t dividend = a->d0;
      uint32_t divisor = b->d0;
      uint32_t q = dividend / divisor;
      remainder->d0 = (uint16_t)(dividend - (q * divisor));
      return N16n·Status·ok;
    }

    // bit motion

    typedef uint16_t (*ShiftOp)(uint16_t, uint16_t);

    Local uint16_t shift_left_op(uint16_t value, uint16_t amount){
      return value << amount;
    }

    Local uint16_t shift_right_op(uint16_t value, uint16_t amount){
      return (uint16_t)(value >> amount);
    }

    Local N16n·Status N16n·shift
    (
     uint16_t shift_count
     ,N16n·T *spill
     ,N16n·T *operand
     ,N16n·T *fill
     ,ShiftOp shift_op
     ,ShiftOp complement_shift_op
    ){

      if(operand == NULL && spill == NULL) return N16n·Status·ok;

      if(operand == NULL){
        operand = &N16n·t[0];
        N16n·copy(operand, N16n·zero);
      }

      if(shift_count > 15) return N16n·Status·gt_max_shift_count;

      N16n·T *given_operand = &N16n·t[1];
      N16n·copy(given_operand, operand);

      operand->d0 = shift_op(given_operand->d0, shift_count);
      if(fill != NULL){
        fill->d0 = complement_shift_op(fill->d0, (16 - shift_count));
        N16n·bit_or(operand, operand, fill);
      }
      if(spill != NULL){
        spill->d0 = shift_op(spill->d0, shift_count);
        spill->d0 += complement_shift_op(given_operand->d0, (16 - shift_count));
      }

      return N16n·Status·ok;
    }

    Local N16n·Status 
    N16n·shift_left(uint16_t shift_count, N16n·T *spill, N16n·T *operand, N16n·T *fill){
      return N16n·shift(shift_count, spill, operand, fill, shift_left_op, shift_right_op);
    }

    Local N16n·Status 
    N16n·shift_right(uint16_t shift_count, N16n·T *spill, N16n·T *operand, N16n·T *fill){
      return N16n·shift(shift_count, spill, operand, fill, shift_right_op, shift_left_op);
    }

    Local N16n·Status 
    N16n·arithmetic_shift_right(uint16_t shift_count, N16n·T *operand, N16n·T *spill){

      if(shift_count > 15) return N16n·Status·gt_max_shift_count;

      if(operand == NULL){
        operand = &N16n·t[0];
        N16n·copy(operand, N16n·zero);
      }

      N16n·T *fill = (operand->d0 & 0x8000) ? N16n·all_one_bit : N16n·zero;
      return N16n·shift_right(shift_count, spill, operand, fill);
    }

    Local const N16n·Λ N16n·λ = {

      .allocate_array = N16n·allocate_array
     ,.allocate_array_zero = N16n·allocate_array_zero
     ,.deallocate = N16n·deallocate

     ,.copy = N16n·copy
     ,.bit_and = N16n·bit_and
     ,.bit_or = N16n·bit_or
     ,.bit_complement = N16n·bit_complement
     ,.bit_twos_complement = N16n·bit_twos_complement
     ,.compare = N16n·compare
     ,.lt = N16n·lt
     ,.gt = N16n·gt
     ,.eq = N16n·eq
     ,.eq_zero = N16n·eq_zero
     ,.accumulate = N16n·accumulate
     ,.add = N16n·add
     ,.increment = N16n·increment
     ,.subtract = N16n·subtract
     ,.multiply = N16n·multiply
     ,.divide = N16n·divide
     ,.modulus = N16n·modulus
     ,.shift_left = N16n·shift_left
     ,.shift_right = N16n·shift_right
     ,.arithmetic_shift_right = N16n·arithmetic_shift_right

     ,.access = N16n·access
     ,.from_uint32 = N16n·from_uint32
    };

  #endif

#endif
