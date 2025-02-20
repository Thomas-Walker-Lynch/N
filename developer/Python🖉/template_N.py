
from template_conversion import conversion
from make_N_constants import make_N_constants

def N(
      namespace 
      ,digit_type 
      ,digit_array_extent_type 
      ,digit_array_extent 
      ,address_type
      ):
  """
  Returns a source code file for cc to munch on.
  """

  template = template_N()
  code = (
    template
    .replace("NS" ,namespace)
    .replace("DIGIT_TYPE" ,digit_type)
    .replace("DIGIT_ARRAY_EXTENT_TYPE" ,str(digit_array_extent_type))
    .replace("DIGIT_ARRAY_EXTENT" ,str(digit_array_extent))
    .replace("ADDRESS_TYPE" ,address_type)
    .replace("CONSTANTS_BLOCK"
      ,make_N_constants(namespace ,digit_type ,digit_array_extent))
    .replace("CONV_8" ,conversion("uint8_t"))
    .replace("CONV_16" ,conversion("uint16_t"))
    .replace("CONV_32" ,conversion("uint32_t"))
    .replace("CONV_64" ,conversion("uint64_t"))
    .replace("CONV_128" ,conversion("__uint128_t"))
  )
  
  return code

def template_N():
    return r'''
/*
  M - The type for the function dictionary (manifold).
  m - a manifold instance, there can be many, m0, m1 ...
  T - Is the type for the tableau. 

*/
#define NS·DEBUG

#ifndef FACE
#define NS·IMPLEMENTATION
#define FACE
#endif

//--------------------------------------------------------------------------------
// Interface

#ifndef NS·FACE
#define NS·FACE

  #include <stdint.h>
  #include <stdbool.h>
  #include <stdarg.h>
  #include <stdlib.h>

  //----------------------------------------
  // Instance Data (Declaration Only)

  typedef ADDRESS_TYPE Address;

  // tableau type, encapsulated data is unavailable to user code
  typedef struct NS·T NS·T;

  extern NS·T *NS·zero;
  extern NS·T *NS·one;
  extern NS·T *NS·all_one_bit;
  extern NS·T *NS·lsb;
  extern NS·T *NS·msb;

  //----------------------------------------
  // Return/Error Status and handlers

  typedef enum{
    NS·Status·ok,
    NS·Status·overflow,
    NS·Status·accumulator_overflow,
    NS·Status·carry,
    NS·Status·borrow,
    NS·Status·undefined_divide_by_zero,
    NS·Status·undefined_modulus_zero,
    NS·Status·gt_max_shift_count,
    NS·Status·spill_eq_operand, // not currently signaled, result will be spill value
    NS·Status·one_word_product,
    NS·Status·two_word_product,
    NS·Status·ConversionOverflow
  } NS·Status;

  typedef enum{
    NS·Order_lt = -1
    ,NS·Order_eq = 0
    ,NS·Order_gt = 1
  } NS·Order;

  // Incomplete conversion NS·T -> PNT,  NS·T leftovers
  typedef struct {
    size_t scale; // this is in bytes
    NS·T *d;      // digits, programmer must point this to a register
  } NS·Leftover_N;

  // Incomplete conversion PNT -> NS·T, PNT leftovers
  #define NS·LEFTOVER_PNT(PNT)\
    typedef struct {\
      size_t scale; // this is in bytes\
      PNT leftover;   // Residual value in PNT format\
    } NS·Leftover_##PNT;

  #ifdef UINT8_MAX
    NS·LEFTOVER_PNT(uint8_t)
  #endif
  #ifdef UINT16_MAX
    NS·LEFTOVER_PNT(uint16_t)
  #endif
  #ifdef UINT32_MAX
    NS·LEFTOVER_PNT(uint32_t)
  #endif
  #ifdef UINT64_MAX
    NS·LEFTOVER_PNT(uint64_t)
  #endif
  #ifdef __UINT128_MAX
    NS·LEFTOVER_PNT(__uint128_t)
  #endif

  // when alloc runs out of memory
  typedef NS·T *( *NS·Allocate_MemoryFault )(Address);

  //----------------------------------------
  // Interface

  #define NS·TO_TYPE(PNT) NS·Status (*to_##PNT)(const NS·T *, PNT *, NS·Leftover_N *)
  #define NS·FROM_TYPE(PNT) NS·Status (*from_##PNT)(const PNT *, NS·T * ,NS·Leftover_##PNT *)

  typedef struct{

    // memory allocation 
    NS·T *(*allocate_array_zero)(Address, NS·Allocate_MemoryFault);
    NS·T *(*allocate_array)(Address, NS·Allocate_MemoryFault);
    void (*deallocate)(NS·T*);
    NS·T* (*access)(NS·T*, Address);

    // results fits in operand type functions
    void (*copy)(NS·T*, NS·T*);
    void (*bit_and)(NS·T*, NS·T*, NS·T*);
    void (*bit_or)(NS·T*, NS·T*, NS·T*);
    void (*bit_complement)(NS·T*, NS·T*);
    void (*bit_twos_complement)(NS·T*, NS·T*);

    // tests  
    NS·Order (*compare)(NS·T*, NS·T*);
    bool (*lt)(NS·T*, NS·T*);
    bool (*gt)(NS·T*, NS·T*);
    bool (*eq)(NS·T*, NS·T*);
    bool (*eq_zero)(NS·T*);

    // arithmetic
    NS·Status (*accumulate)(NS·T *accumulator1 ,NS·T *accumulator0 ,...);
    NS·Status (*add)(NS·T*, NS·T*, NS·T*);
    bool (*increment)(NS·T *a);
    NS·Status (*subtract)(NS·T*, NS·T*, NS·T*);
    NS·Status (*multiply)(NS·T*, NS·T*, NS·T*, NS·T*);
    NS·Status (*divide)(NS·T*, NS·T*, NS·T*, NS·T*);
    NS·Status (*modulus)(NS·T*, NS·T*, NS·T*);

    // shift
    NS·Status (*shift_left)(Address, NS·T*, NS·T*, NS·T*);
    NS·Status (*shift_right)(Address, NS·T*, NS·T*, NS·T*);
    NS·Status (*arithmetic_shift_right)(Address, NS·T*, NS·T*);

    // import/export
    char *(*to_string)(NS·T *);

    #ifdef UINT8_MAX
      NS·TO_TYPE(uint8_t)
      NS·FROM_TYPE(uint8_t)
    #endif
    #ifdef UINT16_MAX
      NS·TO_TYPE(uint16_t)
      NS·FROM_TYPE(uint16_t)
    #endif
    #ifdef UINT32_MAX
      NS·TO_TYPE(uint32_t)
      NS·FROM_TYPE(uint32_t)
    #endif
    #ifdef UINT64_MAX
      NS·TO_TYPE(uint64_t)
      NS·FROM_TYPE(uint64_t)
    #endif
    #ifdef __UINT128_MAX
      NS·TO_TYPE(__uint128_t)
      NS·FROM_TYPE(__uint128_t)
    #endif

  } NS·M;

  Local const NS·M NS·m; // initialized in the LOCAL section

#endif

//--------------------------------------------------------------------------------
// Implementation

#ifdef NS·IMPLEMENTATION

  typedef DIGIT_TYPE Digit;
  const DIGIT_ARRAY_EXTENT_TYPE digit_array_extent = {DIGIT_ARRAY_EXTENT};

  // full type definition for Tableau
  struct NS·T{
    Digit d[digit_array_extent + 1];
  };

  // this part goes into Nlib.a
  #ifndef LOCAL

    #include <stdarg.h>
    #include <stdlib.h>
    #include <stdio.h>

    // the allocate an array of N32
    NS·T *NS·allocate_array(Address extent ,NS·Allocate_MemoryFault memory_fault){
      NS·T *instance = malloc((extent + 1) * sizeof(NS·T) );
      if(!instance){
        return memory_fault ? memory_fault(extent) : NULL;
      }
      return instance;
    }

    NS·T *NS·allocate_array_zero(Address extent ,NS·Allocate_MemoryFault memory_fault){
      NS·T *instance = calloc( extent + 1 ,sizeof(NS·T) );
      if(!instance){
        return memory_fault ? memory_fault(extent) : NULL;
      }
      return instance;
    }

    void NS·deallocate(NS·T *unencumbered){
      free(unencumbered);
    }

  char *to_string(NS·T *n) {
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

    CONSTANTS_BLOCK

    NS·T *NS·zero = NS·constant + 0;
    NS·T *NS·one =  NS·constant + 1;
    NS·T *NS·all_one_bit = NS·constant + 2;
    NS·T *NS·msb = &NS·constant + 3;
    NS·T *NS·lsb = &NS·constant + 1;

    // temporary variables
    // making these LOCAL rather than reserving one block in the library is thread safe
    // allocating a block once is more efficient
    // library code writes these, they are not on the interface

    Local NS·T NS·t[4];


    // allocation 

    extern NS·T *NS·allocate_array(Address, NS·Allocate_MemoryFault);
    extern NS·T *NS·allocate_array_zero(Address, NS·Allocate_MemoryFault);
    extern void NS·deallocate(NS·T *);

    // so the user can access numbers in an array allocation
    Local NS·T* NS·access(NS·T *array ,Address index){
      return array + index;
    }

    // copy, convenience copy

    Local void NS·copy(NS·T *destination ,NS·T *source){
      if(source == destination) return; // that was easy! 
      *destination = *source;
    }

    Local void NS·set_to_zero(NS·T *instance){
      instance->d0 = 0;
    }

    Local void NS·set_to_one(NS·T *instance){
      instance->d0 = 1;
    }

    // bit operations

    Local void NS·bit_and(NS·T *result, NS·T *a, NS·T *b){
      result->d0 = a->d0 & b->d0;
    }

    // result can be one of the operands
    Local void NS·bit_or(NS·T *result, NS·T *a, NS·T *b){
      result->d0 = a->d0 | b->d0;
    }

    // result can the same as the operand
    Local void NS·bit_complement(NS·T *result, NS·T *a){
      result->d0 = ~a->d0;
    }

    // result can the same as the operand
    Local void NS·bit_twos_complement(NS·T *result ,NS·T *a){
      result->d0 = ~a->d0 + 1;
    }

    // test functions

    Local NS·Order NS·compare(NS·T *a, NS·T *b){
      if(a->d0 < b->d0) return NS·Order_lt;
      if(a->d0 > b->d0) return NS·Order_gt;
      return NS·Order_eq;
    }

    Local bool NS·lt(NS·T *a ,NS·T *b){
      return  a->d0 < b->d0;
    }    

    Local bool NS·gt(NS·T *a ,NS·T *b){
      return  a->d0 > b->d0;
    }    

    Local bool NS·eq(NS·T *a ,NS·T *b){
      return  a->d0 == b->d0;
    }    

    Local bool NS·eq_zero(NS·T *a){
      return  a->d0 == 0;
    }    


    // arithmetic operations

    // For a large number of summands for the lower precision Natural implementations, for accumulate/add/sub, the 'overflow' operand could overflow and thus this routine will halt and return NS·Status·accumulator1_overflow
    //
    // When accumulator1 and accumulator0 point to the same location, the result is the accumulator1 value.
    Local NS·Status NS·accumulate(NS·T *accumulator1 ,NS·T *accumulator0 ,...){

      va_list args;
      va_start(args ,accumulator0);
      uint32_t sum = accumulator0->d0;
      uint32_t carry = 0;
      NS·T *current;

      while( (current = va_arg(args ,NS·T *)) ){
        sum += current->d0;
        if(sum < current->d0){  // Accumulator1 into carry
          (carry)++;
          if(carry == 0){
            va_end(args);
            return NS·Status·accumulator1_overflow;
          }
        }
      }
      va_end(args);

      // wipes out prior value of accumulator1
      accumulator1->d0 = carry;

      return NS·Status·ok;
    }

    Local NS·Status NS·add(NS·T *sum ,NS·T *a ,NS·T *b){
      uint64_t result = (uint64_t)a->d0 + (uint64_t)b->d0;
      sum->d0 = (uint32_t)result;
      return (result >> 32) ? NS·Status·carry : NS·Status·ok;
    }

    Local bool NS·increment(NS·T *a){
      a->d0++;
      return a->d0 == 0;
    }

    Local NS·Status NS·subtract(NS·T *difference ,NS·T *a ,NS·T *b){
      uint64_t diff = (uint64_t) a->d0 - (uint64_t) b->d0;
      difference->d0 = (uint32_t)diff;
      return (diff > a->d0) ? NS·Status·borrow : NS·Status·ok;
    }

    Local NS·Status NS·multiply(NS·T *product1 ,NS·T *product0 ,NS·T *a ,NS·T *b){
      uint64_t product = (uint64_t)a->d0 * (uint64_t)b->d0;
      product0->d0 = (uint32_t)product;
      product1->d0 = (uint32_t)(product >> 32);

      if(product1->d0 == 0) return NS·Status·one_word_product;
      return NS·Status·two_word_product;
    }

    Local NS·Status NS·divide(NS·T *remainder ,NS·T *quotient ,NS·T *a ,NS·T *b){
      if(b->d0 == 0) return NS·Status·undefined_divide_by_zero; 

      quotient->d0 = a->d0 / b->d0;
      remainder->d0 = a->d0 - (quotient->d0 * b->d0);

      return NS·Status·ok;
    }

    Local NS·Status NS·modulus(NS·T *remainder ,NS·T *a ,NS·T *b){
      if(b->d0 == 0) return NS·Status·undefined_modulus_zero; 
      uint32_t quotient = a->d0 / b->d0;
      remainder->d0 = a->d0 - (quotient * b->d0);
      return NS·Status·ok;
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
    Local NS·Status NS·shift
    (
     uint32_t shift_count
     ,NS·T *spill
     ,NS·T *operand
     ,NS·T *fill
     ,ShiftOp shift_op
     ,ShiftOp complement_shift_op
     ){

      // If no result is needed, return immediately.
      if(operand == NULL && spill == NULL) return NS·Status·ok;

      // Treat NULL operand as zero.
      if(operand == NULL){
        operand = &NS·t[0];
        NS·copy(operand, NS·zero);
      }

      // Shifting more than one word breaks our fill/spill model.
      if(shift_count > 31) return NS·Status·gt_max_shift_count;

      // The given operand is still required after it is modified, so we copy it.
      NS·T *given_operand = &NS·t[1];
      NS·copy(given_operand, operand);

      // Perform the shift
      operand->d0 = shift_op(given_operand->d0, shift_count);
      if(fill != NULL){
        fill->d0 = complement_shift_op(fill->d0, (32 - shift_count));
        NS·bit_or(operand, operand, fill);
      }
      if(spill != NULL){
        spill->d0 = shift_op(spill->d0, shift_count);
        spill->d0 += complement_shift_op(given_operand->d0, (32 - shift_count));
      }

      return NS·Status·ok;
    }

    // Define concrete shift functions using valid C function pointers
    Local NS·Status 
    NS·shift_left(uint32_t shift_count, NS·T *spill, NS·T *operand, NS·T *fill){
      return NS·shift(shift_count, spill, operand, fill, shift_left_op, shift_right_op);
    }

    Local NS·Status 
    NS·shift_right(uint32_t shift_count, NS·T *spill, NS·T *operand, NS·T *fill){
      return NS·shift(shift_count, spill, operand, fill, shift_right_op, shift_left_op);
    }

    Local NS·Status 
    NS·arithmetic_shift_right(uint32_t shift_count, NS·T *operand, NS·T *spill){

      // Guard against excessive shift counts
      if(shift_count > 31) return NS·Status·gt_max_shift_count;

      // A NULL operand is treated as zero
      if(operand == NULL){
        operand = &NS·t[0];
        NS·copy(operand, NS·zero);
      }

      // Pick the fill value based on the sign bit
      NS·T *fill = (operand->d0 & 0x80000000) ? NS·all_one_bit : NS·zero;

      // Call shift_right with the appropriate fill
      return NS·shift_right(shift_count, spill, operand, fill);
    }

    #ifdef UINT8_MAX
      CONV_8
    #endif
    #ifdef UINT16_MAX
      CONV_16
    #endif
    #ifdef UINT32_MAX
      CONV_32
    #endif
    #ifdef UINT64_MAX
      CONV_64
    #endif
    #ifdef __UINT128_MAX
      CONV_128
    #endif

    Local const NS·M NS·m = {

      .allocate_array = NS·allocate_array
      ,.allocate_array_zero = NS·allocate_array_zero
      ,.deallocate = NS·deallocate

      ,.copy = NS·copy
      ,.bit_and = NS·bit_and
      ,.bit_or = NS·bit_or
      ,.bit_complement = NS·bit_complement
      ,.bit_twos_complement = NS·bit_twos_complement
      ,.compare = NS·compare
      ,.lt = NS·lt
      ,.gt = NS·gt
      ,.eq = NS·eq
      ,.eq_zero = NS·eq_zero
      ,.accumulate = NS·accumulate
      ,.add = NS·add
      ,.increment = NS·increment
      ,.subtract = NS·subtract
      ,.multiply = NS·multiply
      ,.divide = NS·divide
      ,.modulus = NS·modulus
      ,.shift_left = NS·shift_left
      ,.shift_right = NS·shift_right
      ,.arithmetic_shift_right = NS·arithmetic_shift_right

      ,.access = NS·access
      ,.from_uint32 = NS·from_uint32

      #ifdef UINT8_MAX
        ,.to_uint8_t = NS·to_uint8_t
        ,.from_uint8_t = NS·from_uint8_t
      #endif
      #ifdef UINT16_MAX
        ,.to_uint16_t = NS·to_uint16_t
        ,.from_uint16_t = NS·from_uint16_t
      #endif
      #ifdef UINT32_MAX
        ,.to_uint32_t = NS·to_uint32_t
        ,.from_uint32_t = NS·from_uint32_t
      #endif
      #ifdef UINT64_MAX
        ,.to_uint64_t = NS·to_uint64_t
        ,.from_uint64_t = NS·from_uint64_t
      #endif
      #ifdef __UINT128_MAX
        ,.to___uint128_t = NS·to___uint128_t
        ,.from___uint128_t = NS·from___uint128_t
      #endif

    };

  #endif

#endif
'''



 
