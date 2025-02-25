/*
  N8PN - PN = refers to the use of processor native accumulator 

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

#define N8PN·DEBUG

#ifndef FACE
#define N8PN·IMPLEMENTATION
#define FACE
#endif

//--------------------------------------------------------------------------------
// Interface

#ifndef N8PN·FACE
#define N8PN·FACE

  #include <stdint.h>
  #include <stdbool.h>
  #include <stdarg.h>
  #include <stdlib.h>

  //----------------------------------------
  // Instance Data (Declaration Only)

  typedef uint8_t Extent;
  typedef uint8_t Digit;

  typedef struct N8PN·T N8PN·T;

  extern N8PN·T *N8PN·zero;
  extern N8PN·T *N8PN·one;
  extern N8PN·T *N8PN·all_one_bit;
  extern N8PN·T *N8PN·lsb;
  extern N8PN·T *N8PN·msb;

  //----------------------------------------
  // Return/Error Status and handlers

  typedef enum{
    N8PN·Status·ok = 0
   ,N8PN·Status·overflow = 1
   ,N8PN·Status·accumulator1_overflow = 2
   ,N8PN·Status·carry = 3
   ,N8PN·Status·borrow = 4
   ,N8PN·Status·undefined_divide_by_zero = 5
   ,N8PN·Status·undefined_modulus_zero = 6
   ,N8PN·Status·gt_max_shift_count = 7
   ,N8PN·Status·spill_eq_operand = 8
   ,N8PN·Status·one_word_product = 9
   ,N8PN·Status·two_word_product = 10
  } N8PN·Status;

  typedef enum{
    N8PN·Order_lt = -1
   ,N8PN·Order_eq = 0
   ,N8PN·Order_gt = 1
  } N8PN·Order;

  typedef N8PN·T *( *N8PN·Allocate_MemoryFault )(Extent);

  //----------------------------------------
  // Interface

  typedef struct{

    N8PN·T *(*allocate_array_zero)(Extent, N8PN·Allocate_MemoryFault);
    N8PN·T *(*allocate_array)(Extent, N8PN·Allocate_MemoryFault);
    void (*deallocate)(N8PN·T*);

    void (*copy)(N8PN·T*, N8PN·T*);
    void (*bit_and)(N8PN·T*, N8PN·T*, N8PN·T*);
    void (*bit_or)(N8PN·T*, N8PN·T*, N8PN·T*);
    void (*bit_complement)(N8PN·T*, N8PN·T*);
    void (*bit_twos_complement)(N8PN·T*, N8PN·T*);
    N8PN·Order (*compare)(N8PN·T*, N8PN·T*);
    bool (*lt)(N8PN·T*, N8PN·T*);
    bool (*gt)(N8PN·T*, N8PN·T*);
    bool (*eq)(N8PN·T*, N8PN·T*);
    bool (*eq_zero)(N8PN·T*);
    N8PN·Status (*accumulate)(N8PN·T *accumulator1 ,N8PN·T *accumulator0 ,...);
    N8PN·Status (*add)(N8PN·T*, N8PN·T*, N8PN·T*);
    bool (*increment)(N8PN·T *a);
    N8PN·Status (*subtract)(N8PN·T*, N8PN·T*, N8PN·T*);
    N8PN·Status (*multiply)(N8PN·T*, N8PN·T*, N8PN·T*, N8PN·T*);
    N8PN·Status (*divide)(N8PN·T*, N8PN·T*, N8PN·T*, N8PN·T*);
    N8PN·Status (*modulus)(N8PN·T*, N8PN·T*, N8PN·T*);
    N8PN·Status (*shift_left)(Extent, N8PN·T*, N8PN·T*, N8PN·T*);
    N8PN·Status (*shift_right)(Extent, N8PN·T*, N8PN·T*, N8PN·T*);
    N8PN·Status (*arithmetic_shift_right)(Extent, N8PN·T*, N8PN·T*);

    N8PN·T* (*access)(N8PN·T*, Extent);
    void (*from_uint32)(N8PN·T *destination ,uint32_t value);
  } N8PN·Λ;

  Local const N8PN·Λ N8PN·λ; // initialized in the LOCAL section

#endif

//--------------------------------------------------------------------------------
// Implementation

#ifdef N8PN·IMPLEMENTATION

  // this part goes into the library
  #ifndef LOCAL

  #include <stdarg.h>
  #include <stdlib.h>

  struct N8PN·T{
    Digit d0;
  };

  N8PN·T N8PN·constant[4] = {
  {.d0 = 0},
  {.d0 = 1},
  {.d0 = ~(uint8_t)0},
  {.d0 = 1 << 7}
  };

  N8PN·T *N8PN·zero = &N8PN·constant[0];
  N8PN·T *N8PN·one = &N8PN·constant[1];
  N8PN·T *N8PN·all_one_bit = &N8PN·constant[2];
  N8PN·T *N8PN·msb = &N8PN·constant[3];
  N8PN·T *N8PN·lsb = &N8PN·constant[1];

  // the allocate an array of N8
  N8PN·T *N8PN·allocate_array(Extent extent ,N8PN·Allocate_MemoryFault memory_fault){
    N8PN·T *instance = malloc((extent + 1) * sizeof(N8PN·T));
    if(!instance){
      return memory_fault ? memory_fault(extent) : NULL;
    }
    return instance;
  }

  N8PN·T *N8PN·allocate_array_zero(Extent extent ,N8PN·Allocate_MemoryFault memory_fault){
    N8PN·T *instance = calloc(extent + 1, sizeof(N8PN·T));
    if(!instance){
      return memory_fault ? memory_fault(extent) : NULL;
    }
    return instance;
  }

  void N8PN·deallocate(N8PN·T *unencumbered){
    free(unencumbered);
  }


  #endif

  // This part is included after the library user's code
  #ifdef LOCAL

    // instance

    struct N8PN·T{
      Digit d0;
    };

    // temporary variables
    Local N8PN·T N8PN·t[4];

    // allocation 

    extern N8PN·T *N8PN·allocate_array(Extent, N8PN·Allocate_MemoryFault);
    extern N8PN·T *N8PN·allocate_array_zero(Extent, N8PN·Allocate_MemoryFault);
    extern void N8PN·deallocate(N8PN·T *);

    // so the user can access numbers in an array allocation
    Local N8PN·T* N8PN·access(N8PN·T *array ,Extent index){
      return &array[index];
    }

    Local void N8PN·from_uint32(N8PN·T *destination ,uint32_t value){
      if(destination == NULL) return;
      destination->d0 = (uint8_t)(value & 0xFF);
    }

    // copy, convenience copy

    Local void N8PN·copy(N8PN·T *destination ,N8PN·T *source){
      if(source == destination) return;
      *destination = *source;
    }

    Local void N8PN·set_to_zero(N8PN·T *instance){
      instance->d0 = 0;
    }

    Local void N8PN·set_to_one(N8PN·T *instance){
      instance->d0 = 1;
    }

    // bit operations

    Local void N8PN·bit_and(N8PN·T *result, N8PN·T *a, N8PN·T *b){
      result->d0 = a->d0 & b->d0;
    }

    Local void N8PN·bit_or(N8PN·T *result, N8PN·T *a, N8PN·T *b){
      result->d0 = a->d0 | b->d0;
    }

    Local void N8PN·bit_complement(N8PN·T *result, N8PN·T *a){
      result->d0 = ~a->d0;
    }

    Local void N8PN·bit_twos_complement(N8PN·T *result ,N8PN·T *a){
      result->d0 = (uint8_t)(~a->d0 + 1);
    }

    // test functions

    Local N8PN·Order N8PN·compare(N8PN·T *a, N8PN·T *b){
      if(a->d0 < b->d0) return N8PN·Order_lt;
      if(a->d0 > b->d0) return N8PN·Order_gt;
      return N8PN·Order_eq;
    }

    Local bool N8PN·lt(N8PN·T *a ,N8PN·T *b){
      return a->d0 < b->d0;
    }    

    Local bool N8PN·gt(N8PN·T *a ,N8PN·T *b){
      return a->d0 > b->d0;
    }    

    Local bool N8PN·eq(N8PN·T *a ,N8PN·T *b){
      return a->d0 == b->d0;
    }    

    Local bool N8PN·eq_zero(N8PN·T *a){
      return a->d0 == 0;
    }    

    // arithmetic operations

    Local N8PN·Status N8PN·accumulate(N8PN·T *accumulator1 ,N8PN·T *accumulator0 ,...){

      va_list args;
      va_start(args ,accumulator0);
      uint32_t sum = accumulator0->d0;
      uint32_t carry = 0;
      N8PN·T *current;

      while( (current = va_arg(args ,N8PN·T*)) ){
        sum += current->d0;
        if(sum < current->d0){
          (carry)++;
          if(carry == 0){
            va_end(args);
            return N8PN·Status·accumulator1_overflow;
          }
        }
      }
      va_end(args);

      accumulator1->d0 = (uint8_t)carry;
      return N8PN·Status·ok;
    }

    Local N8PN·Status N8PN·add(N8PN·T *sum ,N8PN·T *a ,N8PN·T *b){
      uint32_t result = (uint32_t)a->d0 + (uint32_t)b->d0;
      sum->d0 = (uint8_t)(result & 0xFF);
      return (result >> 8) ? N8PN·Status·carry : N8PN·Status·ok;
    }

    Local bool N8PN·increment(N8PN·T *a){
      a->d0++;
      return (a->d0 == 0);
    }

    Local N8PN·Status N8PN·subtract(N8PN·T *difference ,N8PN·T *a ,N8PN·T *b){
      uint32_t diff = (uint32_t)a->d0 - (uint32_t)b->d0;
      difference->d0 = (uint8_t)(diff & 0xFF);
      return (diff > a->d0) ? N8PN·Status·borrow : N8PN·Status·ok;
    }

    Local N8PN·Status N8PN·multiply(N8PN·T *product1 ,N8PN·T *product0 ,N8PN·T *a ,N8PN·T *b){
      uint32_t product = (uint32_t)a->d0 * (uint32_t)b->d0;
      product0->d0 = (uint8_t)(product & 0xFF);
      product1->d0 = (uint8_t)((product >> 8) & 0xFF);

      if(product1->d0 == 0) return N8PN·Status·one_word_product;
      return N8PN·Status·two_word_product;
    }

    Local N8PN·Status N8PN·divide(N8PN·T *remainder ,N8PN·T *quotient ,N8PN·T *a ,N8PN·T *b){
      if(b->d0 == 0) return N8PN·Status·undefined_divide_by_zero;

      uint32_t dividend = a->d0;
      uint32_t divisor = b->d0;
      quotient->d0 = (uint8_t)(dividend / divisor);
      remainder->d0 = (uint8_t)(dividend - (quotient->d0 * divisor));

      return N8PN·Status·ok;
    }

    Local N8PN·Status N8PN·modulus(N8PN·T *remainder ,N8PN·T *a ,N8PN·T *b){
      if(b->d0 == 0) return N8PN·Status·undefined_modulus_zero;
      uint32_t dividend = a->d0;
      uint32_t divisor = b->d0;
      uint32_t q = dividend / divisor;
      remainder->d0 = (uint8_t)(dividend - (q * divisor));
      return N8PN·Status·ok;
    }

    // bit motion

    typedef uint8_t (*ShiftOp)(uint8_t, uint8_t);

    Local uint8_t shift_left_op(uint8_t value, uint8_t amount){
      return (uint8_t)(value << amount);
    }

    Local uint8_t shift_right_op(uint8_t value, uint8_t amount){
      return (uint8_t)(value >> amount);
    }

    Local N8PN·Status N8PN·shift
    (
     uint8_t shift_count
     ,N8PN·T *spill
     ,N8PN·T *operand
     ,N8PN·T *fill
     ,ShiftOp shift_op
     ,ShiftOp complement_shift_op
    ){

      if(operand == NULL && spill == NULL) return N8PN·Status·ok;

      if(operand == NULL){
        operand = &N8PN·t[0];
        N8PN·copy(operand, N8PN·zero);
      }

      if(shift_count > 7) return N8PN·Status·gt_max_shift_count;

      N8PN·T *given_operand = &N8PN·t[1];
      N8PN·copy(given_operand, operand);

      operand->d0 = shift_op(given_operand->d0, shift_count);
      if(fill != NULL){
        fill->d0 = complement_shift_op(fill->d0, (8 - shift_count));
        N8PN·bit_or(operand, operand, fill);
      }
      if(spill != NULL){
        spill->d0 = shift_op(spill->d0, shift_count);
        spill->d0 += complement_shift_op(given_operand->d0, (8 - shift_count));
      }

      return N8PN·Status·ok;
    }

    Local N8PN·Status 
    N8PN·shift_left(uint8_t shift_count, N8PN·T *spill, N8PN·T *operand, N8PN·T *fill){
      return N8PN·shift(shift_count, spill, operand, fill, shift_left_op, shift_right_op);
    }

    Local N8PN·Status 
    N8PN·shift_right(uint8_t shift_count, N8PN·T *spill, N8PN·T *operand, N8PN·T *fill){
      return N8PN·shift(shift_count, spill, operand, fill, shift_right_op, shift_left_op);
    }

    Local N8PN·Status 
    N8PN·arithmetic_shift_right(uint8_t shift_count, N8PN·T *operand, N8PN·T *spill){

      if(shift_count > 7) return N8PN·Status·gt_max_shift_count;

      if(operand == NULL){
        operand = &N8PN·t[0];
        N8PN·copy(operand, N8PN·zero);
      }

      N8PN·T *fill = (operand->d0 & 0x80) ? N8PN·all_one_bit : N8PN·zero;
      return N8PN·shift_right(shift_count, spill, operand, fill);
    }

    Local const N8PN·Λ N8PN·λ = {

      .allocate_array = N8PN·allocate_array
     ,.allocate_array_zero = N8PN·allocate_array_zero
     ,.deallocate = N8PN·deallocate

     ,.copy = N8PN·copy
     ,.bit_and = N8PN·bit_and
     ,.bit_or = N8PN·bit_or
     ,.bit_complement = N8PN·bit_complement
     ,.bit_twos_complement = N8PN·bit_twos_complement
     ,.compare = N8PN·compare
     ,.lt = N8PN·lt
     ,.gt = N8PN·gt
     ,.eq = N8PN·eq
     ,.eq_zero = N8PN·eq_zero
     ,.accumulate = N8PN·accumulate
     ,.add = N8PN·add
     ,.increment = N8PN·increment
     ,.subtract = N8PN·subtract
     ,.multiply = N8PN·multiply
     ,.divide = N8PN·divide
     ,.modulus = N8PN·modulus
     ,.shift_left = N8PN·shift_left
     ,.shift_right = N8PN·shift_right
     ,.arithmetic_shift_right = N8PN·arithmetic_shift_right

     ,.access = N8PN·access
     ,.from_uint32 = N8PN·from_uint32
    };

  #endif

#endif
