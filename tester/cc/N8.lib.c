/*
  N8 - PN = refers to the use of processor native accumulator 

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

#define N8·DEBUG

#ifndef FACE
#define N8·IMPLEMENTATION
#define FACE
#endif

//--------------------------------------------------------------------------------
// Interface

#ifndef N8·FACE
#define N8·FACE

  #include <stdint.h>
  #include <stdbool.h>
  #include <stdarg.h>
  #include <stdlib.h>

  //----------------------------------------
  // Instance Data (Declaration Only)

  typedef uint8_t Extent;
  typedef uint8_t Digit;

  typedef struct N8·T N8·T;

  extern N8·T *N8·zero;
  extern N8·T *N8·one;
  extern N8·T *N8·all_one_bit;
  extern N8·T *N8·lsb;
  extern N8·T *N8·msb;

  //----------------------------------------
  // Return/Error Status and handlers

  typedef enum{
    N8·Status·ok = 0
   ,N8·Status·overflow = 1
   ,N8·Status·accumulator1_overflow = 2
   ,N8·Status·carry = 3
   ,N8·Status·borrow = 4
   ,N8·Status·undefined_divide_by_zero = 5
   ,N8·Status·undefined_modulus_zero = 6
   ,N8·Status·gt_max_shift_count = 7
   ,N8·Status·spill_eq_operand = 8
   ,N8·Status·one_word_product = 9
   ,N8·Status·two_word_product = 10
  } N8·Status;

  typedef enum{
    N8·Order_lt = -1
   ,N8·Order_eq = 0
   ,N8·Order_gt = 1
  } N8·Order;

  typedef N8·T *( *N8·Allocate_MemoryFault )(Extent);

  //----------------------------------------
  // Interface

  typedef struct{

    N8·T *(*allocate_array_zero)(Extent, N8·Allocate_MemoryFault);
    N8·T *(*allocate_array)(Extent, N8·Allocate_MemoryFault);
    void (*deallocate)(N8·T*);

    void (*copy)(N8·T*, N8·T*);
    void (*bit_and)(N8·T*, N8·T*, N8·T*);
    void (*bit_or)(N8·T*, N8·T*, N8·T*);
    void (*bit_complement)(N8·T*, N8·T*);
    void (*bit_twos_complement)(N8·T*, N8·T*);
    N8·Order (*compare)(N8·T*, N8·T*);
    bool (*lt)(N8·T*, N8·T*);
    bool (*gt)(N8·T*, N8·T*);
    bool (*eq)(N8·T*, N8·T*);
    bool (*eq_zero)(N8·T*);
    N8·Status (*accumulate)(N8·T *accumulator1 ,N8·T *accumulator0 ,...);
    N8·Status (*add)(N8·T*, N8·T*, N8·T*);
    bool (*increment)(N8·T *a);
    N8·Status (*subtract)(N8·T*, N8·T*, N8·T*);
    N8·Status (*multiply)(N8·T*, N8·T*, N8·T*, N8·T*);
    N8·Status (*divide)(N8·T*, N8·T*, N8·T*, N8·T*);
    N8·Status (*modulus)(N8·T*, N8·T*, N8·T*);
    N8·Status (*shift_left)(Extent, N8·T*, N8·T*, N8·T*);
    N8·Status (*shift_right)(Extent, N8·T*, N8·T*, N8·T*);
    N8·Status (*arithmetic_shift_right)(Extent, N8·T*, N8·T*);

    N8·T* (*access)(N8·T*, Extent);
    void (*from_uint32)(N8·T *destination ,uint32_t value);
  } N8·Λ;

  Local const N8·Λ N8·λ; // initialized in the LOCAL section

#endif

//--------------------------------------------------------------------------------
// Implementation

#ifdef N8·IMPLEMENTATION

  // this part goes into the library
  #ifndef LOCAL

  #include <stdarg.h>
  #include <stdlib.h>

  struct N8·T{
    Digit d0;
  };

  N8·T N8·constant[4] = {
  {.d0 = 0},
  {.d0 = 1},
  {.d0 = ~(uint8_t)0},
  {.d0 = 1 << 7}
  };

  N8·T *N8·zero = &N8·constant[0];
  N8·T *N8·one = &N8·constant[1];
  N8·T *N8·all_one_bit = &N8·constant[2];
  N8·T *N8·msb = &N8·constant[3];
  N8·T *N8·lsb = &N8·constant[1];

  // the allocate an array of N8
  N8·T *N8·allocate_array(Extent extent ,N8·Allocate_MemoryFault memory_fault){
    N8·T *instance = malloc((extent + 1) * sizeof(N8·T));
    if(!instance){
      return memory_fault ? memory_fault(extent) : NULL;
    }
    return instance;
  }

  N8·T *N8·allocate_array_zero(Extent extent ,N8·Allocate_MemoryFault memory_fault){
    N8·T *instance = calloc(extent + 1, sizeof(N8·T));
    if(!instance){
      return memory_fault ? memory_fault(extent) : NULL;
    }
    return instance;
  }

  void N8·deallocate(N8·T *unencumbered){
    free(unencumbered);
  }


  #endif

  // This part is included after the library user's code
  #ifdef LOCAL

    // instance

    struct N8·T{
      Digit d0;
    };

    // temporary variables
    Local N8·T N8·t[4];

    // allocation 

    extern N8·T *N8·allocate_array(Extent, N8·Allocate_MemoryFault);
    extern N8·T *N8·allocate_array_zero(Extent, N8·Allocate_MemoryFault);
    extern void N8·deallocate(N8·T *);

    // so the user can access numbers in an array allocation
    Local N8·T* N8·access(N8·T *array ,Extent index){
      return &array[index];
    }

    Local void N8·from_uint32(N8·T *destination ,uint32_t value){
      if(destination == NULL) return;
      destination->d0 = (uint8_t)(value & 0xFF);
    }

    // copy, convenience copy

    Local void N8·copy(N8·T *destination ,N8·T *source){
      if(source == destination) return;
      *destination = *source;
    }

    Local void N8·set_to_zero(N8·T *instance){
      instance->d0 = 0;
    }

    Local void N8·set_to_one(N8·T *instance){
      instance->d0 = 1;
    }

    // bit operations

    Local void N8·bit_and(N8·T *result, N8·T *a, N8·T *b){
      result->d0 = a->d0 & b->d0;
    }

    Local void N8·bit_or(N8·T *result, N8·T *a, N8·T *b){
      result->d0 = a->d0 | b->d0;
    }

    Local void N8·bit_complement(N8·T *result, N8·T *a){
      result->d0 = ~a->d0;
    }

    Local void N8·bit_twos_complement(N8·T *result ,N8·T *a){
      result->d0 = (uint8_t)(~a->d0 + 1);
    }

    // test functions

    Local N8·Order N8·compare(N8·T *a, N8·T *b){
      if(a->d0 < b->d0) return N8·Order_lt;
      if(a->d0 > b->d0) return N8·Order_gt;
      return N8·Order_eq;
    }

    Local bool N8·lt(N8·T *a ,N8·T *b){
      return a->d0 < b->d0;
    }    

    Local bool N8·gt(N8·T *a ,N8·T *b){
      return a->d0 > b->d0;
    }    

    Local bool N8·eq(N8·T *a ,N8·T *b){
      return a->d0 == b->d0;
    }    

    Local bool N8·eq_zero(N8·T *a){
      return a->d0 == 0;
    }    

    // arithmetic operations

    Local N8·Status N8·accumulate(N8·T *accumulator1 ,N8·T *accumulator0 ,...){

      va_list args;
      va_start(args ,accumulator0);
      uint32_t sum = accumulator0->d0;
      uint32_t carry = 0;
      N8·T *current;

      while( (current = va_arg(args ,N8·T*)) ){
        sum += current->d0;
        if(sum < current->d0){
          (carry)++;
          if(carry == 0){
            va_end(args);
            return N8·Status·accumulator1_overflow;
          }
        }
      }
      va_end(args);

      accumulator1->d0 = (uint8_t)carry;
      return N8·Status·ok;
    }

    Local N8·Status N8·add(N8·T *sum ,N8·T *a ,N8·T *b){
      uint32_t result = (uint32_t)a->d0 + (uint32_t)b->d0;
      sum->d0 = (uint8_t)(result & 0xFF);
      return (result >> 8) ? N8·Status·carry : N8·Status·ok;
    }

    Local bool N8·increment(N8·T *a){
      a->d0++;
      return (a->d0 == 0);
    }

    Local N8·Status N8·subtract(N8·T *difference ,N8·T *a ,N8·T *b){
      uint32_t diff = (uint32_t)a->d0 - (uint32_t)b->d0;
      difference->d0 = (uint8_t)(diff & 0xFF);
      return (diff > a->d0) ? N8·Status·borrow : N8·Status·ok;
    }

    Local N8·Status N8·multiply(N8·T *product1 ,N8·T *product0 ,N8·T *a ,N8·T *b){
      uint32_t product = (uint32_t)a->d0 * (uint32_t)b->d0;
      product0->d0 = (uint8_t)(product & 0xFF);
      product1->d0 = (uint8_t)((product >> 8) & 0xFF);

      if(product1->d0 == 0) return N8·Status·one_word_product;
      return N8·Status·two_word_product;
    }

    Local N8·Status N8·divide(N8·T *remainder ,N8·T *quotient ,N8·T *a ,N8·T *b){
      if(b->d0 == 0) return N8·Status·undefined_divide_by_zero;

      uint32_t dividend = a->d0;
      uint32_t divisor = b->d0;
      quotient->d0 = (uint8_t)(dividend / divisor);
      remainder->d0 = (uint8_t)(dividend - (quotient->d0 * divisor));

      return N8·Status·ok;
    }

    Local N8·Status N8·modulus(N8·T *remainder ,N8·T *a ,N8·T *b){
      if(b->d0 == 0) return N8·Status·undefined_modulus_zero;
      uint32_t dividend = a->d0;
      uint32_t divisor = b->d0;
      uint32_t q = dividend / divisor;
      remainder->d0 = (uint8_t)(dividend - (q * divisor));
      return N8·Status·ok;
    }

    // bit motion

    typedef uint8_t (*ShiftOp)(uint8_t, uint8_t);

    Local uint8_t shift_left_op(uint8_t value, uint8_t amount){
      return (uint8_t)(value << amount);
    }

    Local uint8_t shift_right_op(uint8_t value, uint8_t amount){
      return (uint8_t)(value >> amount);
    }

    Local N8·Status N8·shift
    (
     uint8_t shift_count
     ,N8·T *spill
     ,N8·T *operand
     ,N8·T *fill
     ,ShiftOp shift_op
     ,ShiftOp complement_shift_op
    ){

      if(operand == NULL && spill == NULL) return N8·Status·ok;

      if(operand == NULL){
        operand = &N8·t[0];
        N8·copy(operand, N8·zero);
      }

      if(shift_count > 7) return N8·Status·gt_max_shift_count;

      N8·T *given_operand = &N8·t[1];
      N8·copy(given_operand, operand);

      operand->d0 = shift_op(given_operand->d0, shift_count);
      if(fill != NULL){
        fill->d0 = complement_shift_op(fill->d0, (8 - shift_count));
        N8·bit_or(operand, operand, fill);
      }
      if(spill != NULL){
        spill->d0 = shift_op(spill->d0, shift_count);
        spill->d0 += complement_shift_op(given_operand->d0, (8 - shift_count));
      }

      return N8·Status·ok;
    }

    Local N8·Status 
    N8·shift_left(uint8_t shift_count, N8·T *spill, N8·T *operand, N8·T *fill){
      return N8·shift(shift_count, spill, operand, fill, shift_left_op, shift_right_op);
    }

    Local N8·Status 
    N8·shift_right(uint8_t shift_count, N8·T *spill, N8·T *operand, N8·T *fill){
      return N8·shift(shift_count, spill, operand, fill, shift_right_op, shift_left_op);
    }

    Local N8·Status 
    N8·arithmetic_shift_right(uint8_t shift_count, N8·T *operand, N8·T *spill){

      if(shift_count > 7) return N8·Status·gt_max_shift_count;

      if(operand == NULL){
        operand = &N8·t[0];
        N8·copy(operand, N8·zero);
      }

      N8·T *fill = (operand->d0 & 0x80) ? N8·all_one_bit : N8·zero;
      return N8·shift_right(shift_count, spill, operand, fill);
    }

    Local const N8·Λ N8·λ = {

      .allocate_array = N8·allocate_array
     ,.allocate_array_zero = N8·allocate_array_zero
     ,.deallocate = N8·deallocate

     ,.copy = N8·copy
     ,.bit_and = N8·bit_and
     ,.bit_or = N8·bit_or
     ,.bit_complement = N8·bit_complement
     ,.bit_twos_complement = N8·bit_twos_complement
     ,.compare = N8·compare
     ,.lt = N8·lt
     ,.gt = N8·gt
     ,.eq = N8·eq
     ,.eq_zero = N8·eq_zero
     ,.accumulate = N8·accumulate
     ,.add = N8·add
     ,.increment = N8·increment
     ,.subtract = N8·subtract
     ,.multiply = N8·multiply
     ,.divide = N8·divide
     ,.modulus = N8·modulus
     ,.shift_left = N8·shift_left
     ,.shift_right = N8·shift_right
     ,.arithmetic_shift_right = N8·arithmetic_shift_right

     ,.access = N8·access
     ,.from_uint32 = N8·from_uint32
    };

  #endif

#endif
