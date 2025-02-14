/*
  N8n - a processor native type

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

#define N8n·DEBUG

#ifndef FACE
#define N8n·IMPLEMENTATION
#define FACE
#endif

//--------------------------------------------------------------------------------
// Interface

#ifndef N8n·FACE
#define N8n·FACE

  #include <stdint.h>
  #include <stdbool.h>
  #include <stdarg.h>
  #include <stdlib.h>

  //----------------------------------------
  // Instance Data (Declaration Only)

  typedef uint8_t Extent;
  typedef uint8_t Digit;

  typedef struct N8n·T N8n·T;

  extern N8n·T *N8n·zero;
  extern N8n·T *N8n·one;
  extern N8n·T *N8n·all_one_bit;
  extern N8n·T *N8n·lsb;
  extern N8n·T *N8n·msb;

  //----------------------------------------
  // Return/Error Status and handlers

  typedef enum{
    N8n·Status·ok = 0
   ,N8n·Status·overflow = 1
   ,N8n·Status·accumulator1_overflow = 2
   ,N8n·Status·carry = 3
   ,N8n·Status·borrow = 4
   ,N8n·Status·undefined_divide_by_zero = 5
   ,N8n·Status·undefined_modulus_zero = 6
   ,N8n·Status·gt_max_shift_count = 7
   ,N8n·Status·spill_eq_operand = 8
   ,N8n·Status·one_word_product = 9
   ,N8n·Status·two_word_product = 10
  } N8n·Status;

  typedef enum{
    N8n·Order_lt = -1
   ,N8n·Order_eq = 0
   ,N8n·Order_gt = 1
  } N8n·Order;

  typedef N8n·T *( *N8n·Allocate_MemoryFault )(Extent);

  //----------------------------------------
  // Interface

  typedef struct{

    N8n·T *(*allocate_array_zero)(Extent, N8n·Allocate_MemoryFault);
    N8n·T *(*allocate_array)(Extent, N8n·Allocate_MemoryFault);
    void (*deallocate)(N8n·T*);

    void (*copy)(N8n·T*, N8n·T*);
    void (*bit_and)(N8n·T*, N8n·T*, N8n·T*);
    void (*bit_or)(N8n·T*, N8n·T*, N8n·T*);
    void (*bit_complement)(N8n·T*, N8n·T*);
    void (*bit_twos_complement)(N8n·T*, N8n·T*);
    N8n·Order (*compare)(N8n·T*, N8n·T*);
    bool (*lt)(N8n·T*, N8n·T*);
    bool (*gt)(N8n·T*, N8n·T*);
    bool (*eq)(N8n·T*, N8n·T*);
    bool (*eq_zero)(N8n·T*);
    N8n·Status (*accumulate)(N8n·T *accumulator1 ,N8n·T *accumulator0 ,...);
    N8n·Status (*add)(N8n·T*, N8n·T*, N8n·T*);
    bool (*increment)(N8n·T *a);
    N8n·Status (*subtract)(N8n·T*, N8n·T*, N8n·T*);
    N8n·Status (*multiply)(N8n·T*, N8n·T*, N8n·T*, N8n·T*);
    N8n·Status (*divide)(N8n·T*, N8n·T*, N8n·T*, N8n·T*);
    N8n·Status (*modulus)(N8n·T*, N8n·T*, N8n·T*);
    N8n·Status (*shift_left)(Extent, N8n·T*, N8n·T*, N8n·T*);
    N8n·Status (*shift_right)(Extent, N8n·T*, N8n·T*, N8n·T*);
    N8n·Status (*arithmetic_shift_right)(Extent, N8n·T*, N8n·T*);

    N8n·T* (*access)(N8n·T*, Extent);
    void (*from_uint32)(N8n·T *destination ,uint32_t value);
  } N8n·Λ;

  Local const N8n·Λ N8n·λ; // initialized in the LOCAL section

#endif

//--------------------------------------------------------------------------------
// Implementation

#ifdef N8n·IMPLEMENTATION

  // this part goes into the library
  #ifndef LOCAL

  #include <stdarg.h>
  #include <stdlib.h>

  struct N8n·T{
    Digit d0;
  };

  N8n·T N8n·constant[4] = {
  {.d0 = 0},
  {.d0 = 1},
  {.d0 = ~(uint8_t)0},
  {.d0 = 1 << 7}
  };

  N8n·T *N8n·zero = &N8n·constant[0];
  N8n·T *N8n·one = &N8n·constant[1];
  N8n·T *N8n·all_one_bit = &N8n·constant[2];
  N8n·T *N8n·msb = &N8n·constant[3];
  N8n·T *N8n·lsb = &N8n·constant[1];

  // the allocate an array of N8
  N8n·T *N8n·allocate_array(Extent extent ,N8n·Allocate_MemoryFault memory_fault){
    N8n·T *instance = malloc((extent + 1) * sizeof(N8n·T));
    if(!instance){
      return memory_fault ? memory_fault(extent) : NULL;
    }
    return instance;
  }

  N8n·T *N8n·allocate_array_zero(Extent extent ,N8n·Allocate_MemoryFault memory_fault){
    N8n·T *instance = calloc(extent + 1, sizeof(N8n·T));
    if(!instance){
      return memory_fault ? memory_fault(extent) : NULL;
    }
    return instance;
  }

  void N8n·deallocate(N8n·T *unencumbered){
    free(unencumbered);
  }


  #endif

  // This part is included after the library user's code
  #ifdef LOCAL

    // instance

    struct N8n·T{
      Digit d0;
    };

    // temporary variables
    Local N8n·T N8n·t[4];

    // allocation 

    extern N8n·T *N8n·allocate_array(Extent, N8n·Allocate_MemoryFault);
    extern N8n·T *N8n·allocate_array_zero(Extent, N8n·Allocate_MemoryFault);
    extern void N8n·deallocate(N8n·T *);

    // so the user can access numbers in an array allocation
    Local N8n·T* N8n·access(N8n·T *array ,Extent index){
      return &array[index];
    }

    Local void N8n·from_uint32(N8n·T *destination ,uint32_t value){
      if(destination == NULL) return;
      destination->d0 = (uint8_t)(value & 0xFF);
    }

    // copy, convenience copy

    Local void N8n·copy(N8n·T *destination ,N8n·T *source){
      if(source == destination) return;
      *destination = *source;
    }

    Local void N8n·set_to_zero(N8n·T *instance){
      instance->d0 = 0;
    }

    Local void N8n·set_to_one(N8n·T *instance){
      instance->d0 = 1;
    }

    // bit operations

    Local void N8n·bit_and(N8n·T *result, N8n·T *a, N8n·T *b){
      result->d0 = a->d0 & b->d0;
    }

    Local void N8n·bit_or(N8n·T *result, N8n·T *a, N8n·T *b){
      result->d0 = a->d0 | b->d0;
    }

    Local void N8n·bit_complement(N8n·T *result, N8n·T *a){
      result->d0 = ~a->d0;
    }

    Local void N8n·bit_twos_complement(N8n·T *result ,N8n·T *a){
      result->d0 = (uint8_t)(~a->d0 + 1);
    }

    // test functions

    Local N8n·Order N8n·compare(N8n·T *a, N8n·T *b){
      if(a->d0 < b->d0) return N8n·Order_lt;
      if(a->d0 > b->d0) return N8n·Order_gt;
      return N8n·Order_eq;
    }

    Local bool N8n·lt(N8n·T *a ,N8n·T *b){
      return a->d0 < b->d0;
    }    

    Local bool N8n·gt(N8n·T *a ,N8n·T *b){
      return a->d0 > b->d0;
    }    

    Local bool N8n·eq(N8n·T *a ,N8n·T *b){
      return a->d0 == b->d0;
    }    

    Local bool N8n·eq_zero(N8n·T *a){
      return a->d0 == 0;
    }    

    // arithmetic operations

    Local N8n·Status N8n·accumulate(N8n·T *accumulator1 ,N8n·T *accumulator0 ,...){

      va_list args;
      va_start(args ,accumulator0);
      uint32_t sum = accumulator0->d0;
      uint32_t carry = 0;
      N8n·T *current;

      while( (current = va_arg(args ,N8n·T*)) ){
        sum += current->d0;
        if(sum < current->d0){
          (carry)++;
          if(carry == 0){
            va_end(args);
            return N8n·Status·accumulator1_overflow;
          }
        }
      }
      va_end(args);

      accumulator1->d0 = (uint8_t)carry;
      return N8n·Status·ok;
    }

    Local N8n·Status N8n·add(N8n·T *sum ,N8n·T *a ,N8n·T *b){
      uint32_t result = (uint32_t)a->d0 + (uint32_t)b->d0;
      sum->d0 = (uint8_t)(result & 0xFF);
      return (result >> 8) ? N8n·Status·carry : N8n·Status·ok;
    }

    Local bool N8n·increment(N8n·T *a){
      a->d0++;
      return (a->d0 == 0);
    }

    Local N8n·Status N8n·subtract(N8n·T *difference ,N8n·T *a ,N8n·T *b){
      uint32_t diff = (uint32_t)a->d0 - (uint32_t)b->d0;
      difference->d0 = (uint8_t)(diff & 0xFF);
      return (diff > a->d0) ? N8n·Status·borrow : N8n·Status·ok;
    }

    Local N8n·Status N8n·multiply(N8n·T *product1 ,N8n·T *product0 ,N8n·T *a ,N8n·T *b){
      uint32_t product = (uint32_t)a->d0 * (uint32_t)b->d0;
      product0->d0 = (uint8_t)(product & 0xFF);
      product1->d0 = (uint8_t)((product >> 8) & 0xFF);

      if(product1->d0 == 0) return N8n·Status·one_word_product;
      return N8n·Status·two_word_product;
    }

    Local N8n·Status N8n·divide(N8n·T *remainder ,N8n·T *quotient ,N8n·T *a ,N8n·T *b){
      if(b->d0 == 0) return N8n·Status·undefined_divide_by_zero;

      uint32_t dividend = a->d0;
      uint32_t divisor = b->d0;
      quotient->d0 = (uint8_t)(dividend / divisor);
      remainder->d0 = (uint8_t)(dividend - (quotient->d0 * divisor));

      return N8n·Status·ok;
    }

    Local N8n·Status N8n·modulus(N8n·T *remainder ,N8n·T *a ,N8n·T *b){
      if(b->d0 == 0) return N8n·Status·undefined_modulus_zero;
      uint32_t dividend = a->d0;
      uint32_t divisor = b->d0;
      uint32_t q = dividend / divisor;
      remainder->d0 = (uint8_t)(dividend - (q * divisor));
      return N8n·Status·ok;
    }

    // bit motion

    typedef uint8_t (*ShiftOp)(uint8_t, uint8_t);

    Local uint8_t shift_left_op(uint8_t value, uint8_t amount){
      return (uint8_t)(value << amount);
    }

    Local uint8_t shift_right_op(uint8_t value, uint8_t amount){
      return (uint8_t)(value >> amount);
    }

    Local N8n·Status N8n·shift
    (
     uint8_t shift_count
     ,N8n·T *spill
     ,N8n·T *operand
     ,N8n·T *fill
     ,ShiftOp shift_op
     ,ShiftOp complement_shift_op
    ){

      if(operand == NULL && spill == NULL) return N8n·Status·ok;

      if(operand == NULL){
        operand = &N8n·t[0];
        N8n·copy(operand, N8n·zero);
      }

      if(shift_count > 7) return N8n·Status·gt_max_shift_count;

      N8n·T *given_operand = &N8n·t[1];
      N8n·copy(given_operand, operand);

      operand->d0 = shift_op(given_operand->d0, shift_count);
      if(fill != NULL){
        fill->d0 = complement_shift_op(fill->d0, (8 - shift_count));
        N8n·bit_or(operand, operand, fill);
      }
      if(spill != NULL){
        spill->d0 = shift_op(spill->d0, shift_count);
        spill->d0 += complement_shift_op(given_operand->d0, (8 - shift_count));
      }

      return N8n·Status·ok;
    }

    Local N8n·Status 
    N8n·shift_left(uint8_t shift_count, N8n·T *spill, N8n·T *operand, N8n·T *fill){
      return N8n·shift(shift_count, spill, operand, fill, shift_left_op, shift_right_op);
    }

    Local N8n·Status 
    N8n·shift_right(uint8_t shift_count, N8n·T *spill, N8n·T *operand, N8n·T *fill){
      return N8n·shift(shift_count, spill, operand, fill, shift_right_op, shift_left_op);
    }

    Local N8n·Status 
    N8n·arithmetic_shift_right(uint8_t shift_count, N8n·T *operand, N8n·T *spill){

      if(shift_count > 7) return N8n·Status·gt_max_shift_count;

      if(operand == NULL){
        operand = &N8n·t[0];
        N8n·copy(operand, N8n·zero);
      }

      N8n·T *fill = (operand->d0 & 0x80) ? N8n·all_one_bit : N8n·zero;
      return N8n·shift_right(shift_count, spill, operand, fill);
    }

    Local const N8n·Λ N8n·λ = {

      .allocate_array = N8n·allocate_array
     ,.allocate_array_zero = N8n·allocate_array_zero
     ,.deallocate = N8n·deallocate

     ,.copy = N8n·copy
     ,.bit_and = N8n·bit_and
     ,.bit_or = N8n·bit_or
     ,.bit_complement = N8n·bit_complement
     ,.bit_twos_complement = N8n·bit_twos_complement
     ,.compare = N8n·compare
     ,.lt = N8n·lt
     ,.gt = N8n·gt
     ,.eq = N8n·eq
     ,.eq_zero = N8n·eq_zero
     ,.accumulate = N8n·accumulate
     ,.add = N8n·add
     ,.increment = N8n·increment
     ,.subtract = N8n·subtract
     ,.multiply = N8n·multiply
     ,.divide = N8n·divide
     ,.modulus = N8n·modulus
     ,.shift_left = N8n·shift_left
     ,.shift_right = N8n·shift_right
     ,.arithmetic_shift_right = N8n·arithmetic_shift_right

     ,.access = N8n·access
     ,.from_uint32 = N8n·from_uint32
    };

  #endif

#endif
