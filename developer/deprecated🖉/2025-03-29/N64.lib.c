/*
  N64 - a 64-bit native type

  For binary operations: a op b -> c

  Similar to N32, but now each Digit is 64 bits. Where a 128-bit
  intermediate is necessary (e.g. multiplication), we handle it
  manually using two 64-bit parts.

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

#define N64·DEBUG

#ifndef FACE
#define N64·IMPLEMENTATION
#define FACE
#endif

//--------------------------------------------------------------------------------
// Interface

#ifndef N64·FACE
#define N64·FACE

  #include <stdint.h>
  #include <stdbool.h>
  #include <stdarg.h>
  #include <stdlib.h>

  //----------------------------------------
  // Instance Data (Declaration Only)

  typedef uint64_t Extent;
  typedef uint64_t Digit;

  typedef struct N64·T N64·T;

  extern N64·T *N64·zero;
  extern N64·T *N64·one;
  extern N64·T *N64·all_one_bit;
  extern N64·T *N64·lsb;
  extern N64·T *N64·msb;

  //----------------------------------------
  // Return/Error Status and handlers

  typedef enum {
    N64·Status·ok = 0
   ,N64·Status·overflow = 1
   ,N64·Status·accumulator1_overflow = 2
   ,N64·Status·carry = 3
   ,N64·Status·borrow = 4
   ,N64·Status·undefined_divide_by_zero = 5
   ,N64·Status·undefined_modulus_zero = 6
   ,N64·Status·gt_max_shift_count = 7
   ,N64·Status·spill_eq_operand = 8 // not currently signaled, result will be spill value
   ,N64·Status·one_word_product = 9
   ,N64·Status·two_word_product = 10
  } N64·Status;

  typedef enum {
    N64·Order_lt = -1
   ,N64·Order_eq = 0
   ,N64·Order_gt = 1
  } N64·Order;

  typedef N64·T *(*N64·Allocate_MemoryFault)(Extent);

  //----------------------------------------
  // Interface

  typedef struct {

    N64·T *(*allocate_array_zero)(Extent, N64·Allocate_MemoryFault);
    N64·T *(*allocate_array)(Extent, N64·Allocate_MemoryFault);
    void   (*deallocate)(N64·T*);

    void        (*copy)(N64·T*, N64·T*);
    void        (*bit_and)(N64·T*, N64·T*, N64·T*);
    void        (*bit_or)(N64·T*, N64·T*, N64·T*);
    void        (*bit_complement)(N64·T*, N64·T*);
    void        (*bit_twos_complement)(N64·T*, N64·T*);
    N64·Order   (*compare)(N64·T*, N64·T*);
    bool        (*lt)(N64·T*, N64·T*);
    bool        (*gt)(N64·T*, N64·T*);
    bool        (*eq)(N64·T*, N64·T*);
    bool        (*eq_zero)(N64·T*);
    N64·Status  (*accumulate)(N64·T *accumulator1, N64·T *accumulator0, ...);
    N64·Status  (*add)(N64·T*, N64·T*, N64·T*);
    bool        (*increment)(N64·T *a);
    N64·Status  (*subtract)(N64·T*, N64·T*, N64·T*);
    N64·Status  (*multiply)(N64·T*, N64·T*, N64·T*, N64·T*);
    N64·Status  (*divide)(N64·T*, N64·T*, N64·T*, N64·T*);
    N64·Status  (*modulus)(N64·T*, N64·T*, N64·T*);
    N64·Status  (*shift_left)(Extent, N64·T*, N64·T*, N64·T*);
    N64·Status  (*shift_right)(Extent, N64·T*, N64·T*, N64·T*);
    N64·Status  (*arithmetic_shift_right)(Extent, N64·T*, N64·T*);

    N64·T*      (*access)(N64·T*, Extent);
    void        (*from_uint64)(N64·T *destination, uint64_t value);

  } N64·M;

  Local const N64·M N64·m; // initialized in the LOCAL section

#endif

//--------------------------------------------------------------------------------
// Implementation

#ifdef N64·IMPLEMENTATION

  // this part goes into the library
  #ifndef LOCAL

    #include <stdarg.h>
    #include <stdlib.h>

    struct N64·T {
      Digit d0;
    };

    // For constants, we store them in an array for convenience
    // 0, 1, all bits set (~0ULL), and MSB set (1ULL<<63)
    N64·T N64·constant[4] = {
      {.d0 = 0ULL},
      {.d0 = 1ULL},
      {.d0 = ~(uint64_t)0ULL},
      {.d0 = 1ULL << 63}
    };

    N64·T *N64·zero = &N64·constant[0];
    N64·T *N64·one = &N64·constant[1];
    N64·T *N64·all_one_bit = &N64·constant[2];
    N64·T *N64·msb = &N64·constant[3];
    N64·T *N64·lsb = &N64·constant[1];

    // allocate an array of N64
    N64·T *N64·allocate_array(Extent extent, N64·Allocate_MemoryFault memory_fault){
      N64·T *instance = malloc( (extent + 1) * sizeof(N64·T) );
      if(!instance){
        return memory_fault ? memory_fault(extent) : NULL;
      }
      return instance;
    }

    N64·T *N64·allocate_array_zero(Extent extent, N64·Allocate_MemoryFault memory_fault){
      N64·T *instance = calloc(extent + 1, sizeof(N64·T));
      if(!instance){
        return memory_fault ? memory_fault(extent) : NULL;
      }
      return instance;
    }

    void N64·deallocate(N64·T *unencumbered){
      free(unencumbered);
    }

  #endif

  // This part is included after the library user's code
  #ifdef LOCAL

    // instance

    struct N64·T {
      Digit d0;
    };

    // local temporary variables
    Local N64·T N64·t[4];

    // allocation references
    extern N64·T *N64·allocate_array(Extent, N64·Allocate_MemoryFault);
    extern N64·T *N64·allocate_array_zero(Extent, N64·Allocate_MemoryFault);
    extern void   N64·deallocate(N64·T *);

    // Access array
    Local N64·T* N64·access(N64·T *array, Extent index){
      return &array[index];
    }

    Local void N64·from_uint64(N64·T *destination, uint64_t value){
      if(destination == NULL) return;
      destination->d0 = value;
    }

    // copy
    Local void N64·copy(N64·T *destination, N64·T *source){
      if(source == destination) return;
      *destination = *source;
    }

    // bit operations

    Local void N64·bit_and(N64·T *result, N64·T *a, N64·T *b){
      result->d0 = a->d0 & b->d0;
    }

    Local void N64·bit_or(N64·T *result, N64·T *a, N64·T *b){
      result->d0 = a->d0 | b->d0;
    }

    Local void N64·bit_complement(N64·T *result, N64·T *a){
      result->d0 = ~a->d0;
    }

    Local void N64·bit_twos_complement(N64·T *result, N64·T *a){
      result->d0 = ~a->d0 + 1ULL;
    }

    // compare & test functions

    Local N64·Order N64·compare(N64·T *a, N64·T *b){
      if(a->d0 < b->d0) return N64·Order_lt;
      if(a->d0 > b->d0) return N64·Order_gt;
      return N64·Order_eq;
    }

    Local bool N64·lt(N64·T *a, N64·T *b){
      return (a->d0 < b->d0);
    }

    Local bool N64·gt(N64·T *a, N64·T *b){
      return (a->d0 > b->d0);
    }

    Local bool N64·eq(N64·T *a, N64·T *b){
      return (a->d0 == b->d0);
    }

    Local bool N64·eq_zero(N64·T *a){
      return (a->d0 == 0ULL);
    }

    // arithmetic operations

    // accumulate
    Local N64·Status N64·accumulate(N64·T *accumulator1, N64·T *accumulator0, ...){
      va_list args;
      va_start(args, accumulator0);

      uint64_t sum = accumulator0->d0;
      uint64_t carry = 0;
      N64·T *current;

      while( (current = va_arg(args, N64·T*)) ){
        uint64_t prior = sum;
        sum += current->d0;
        if(sum < prior){  // indicates carry
          carry++;
          // if carry overflowed a 64-bit, that's an accumulator1 overflow
          if(carry == 0ULL){
            va_end(args);
            return N64·Status·accumulator1_overflow;
          }
        }
      }
      va_end(args);

      accumulator1->d0 = carry;
      return N64·Status·ok;
    }

    // add
    Local N64·Status N64·add(N64·T *sum, N64·T *a, N64·T *b){
      __uint128_t result = ( __uint128_t )a->d0 + ( __uint128_t )b->d0;
      // But to avoid using a GNU extension, we can do the simpler approach:
      // Actually let's do it directly with 64-bit since we only need to detect carry out of 64 bits:
      uint64_t temp = a->d0 + b->d0;
      sum->d0 = temp;
      if(temp < a->d0) return N64·Status·carry;  // means we overflowed
      return N64·Status·ok;
    }

    Local bool N64·increment(N64·T *a){
      uint64_t old = a->d0;
      a->d0++;
      // if it wrapped around to 0, then it was 0xFFFFFFFFFFFFFFFF
      return (a->d0 < old);
    }

    // subtract
    Local N64·Status N64·subtract(N64·T *difference, N64·T *a, N64·T *b){
      uint64_t tmpA = a->d0;
      uint64_t tmpB = b->d0;
      uint64_t diff = tmpA - tmpB;
      difference->d0 = diff;
      if(diff > tmpA) return N64·Status·borrow; // indicates we borrowed
      return N64·Status·ok;
    }

    // multiply
    // We'll do a 64x64->128 using two 64-bit accumulators
    Local N64·Status N64·multiply(N64·T *product1, N64·T *product0, N64·T *a, N64·T *b){
      uint64_t A = a->d0;
      uint64_t B = b->d0;

      // Break each operand into high & low 32 bits
      uint64_t a_lo = (uint32_t)(A & 0xffffffffULL);
      uint64_t a_hi = A >> 32;
      uint64_t b_lo = (uint32_t)(B & 0xffffffffULL);
      uint64_t b_hi = B >> 32;

      // partial products
      uint64_t low = a_lo * b_lo;                   // 64-bit
      uint64_t cross = (a_lo * b_hi) + (a_hi * b_lo); // potentially up to 2 * 32 bits => 64 bits
      uint64_t high = a_hi * b_hi;                  // up to 64 bits

      // incorporate cross into low, high
      // cross is effectively the middle bits, so shift cross by 32 and add to low
      uint64_t cross_low = (cross & 0xffffffffULL) << 32;  // lower part
      uint64_t cross_high = cross >> 32;                   // upper part

      // add cross_low to low, capture carry
      uint64_t old_low = low;
      low += cross_low;
      if(low < old_low) cross_high++;

      // final high
      high += cross_high;

      // store results
      product0->d0 = low;
      product1->d0 = high;

      if(high == 0ULL) return N64·Status·one_word_product;
      return N64·Status·two_word_product;
    }

    // divide
    Local N64·Status N64·divide(N64·T *remainder, N64·T *quotient, N64·T *a, N64·T *b){
      // we do not handle a > 64-bit, just the single 64-bit
      if(b->d0 == 0ULL) return N64·Status·undefined_divide_by_zero;

      uint64_t divd = a->d0; // dividend
      uint64_t divs = b->d0; // divisor

      quotient->d0 = divd / divs;
      remainder->d0 = divd - (quotient->d0 * divs);

      return N64·Status·ok;
    }

    // modulus
    Local N64·Status N64·modulus(N64·T *remainder, N64·T *a, N64·T *b){
      if(b->d0 == 0ULL) return N64·Status·undefined_modulus_zero;

      uint64_t divd = a->d0;
      uint64_t divs = b->d0;
      uint64_t q = divd / divs;
      remainder->d0 = divd - (q * divs);

      return N64·Status·ok;
    }

    // bit motion

    typedef uint64_t (*ShiftOp)(uint64_t, uint64_t);

    Local uint64_t shift_left_op(uint64_t value, uint64_t amount){
      return (value << amount);
    }

    Local uint64_t shift_right_op(uint64_t value, uint64_t amount){
      return (value >> amount);
    }

    // modifies all three of its operands
    // in the case of duplicate operands this is the order: first modifies operand, then fill, then spill
    Local N64·Status N64·shift
    (
     uint64_t shift_count,
     N64·T *spill,
     N64·T *operand,
     N64·T *fill,
     ShiftOp shift_op,
     ShiftOp complement_shift_op
    ){
      if(operand == NULL && spill == NULL) return N64·Status·ok;

      // Treat NULL operand as zero
      if(operand == NULL){
        operand = &N64·t[0];
        N64·copy(operand, N64·zero);
      }

      // Shifting more than 63 bits breaks fill/spill logic
      if(shift_count > 63ULL) return N64·Status·gt_max_shift_count;

      N64·T *given_operand = &N64·t[1];
      N64·copy(given_operand, operand);

      // Perform the shift
      operand->d0 = shift_op(given_operand->d0, shift_count);
      if(fill != NULL){
        fill->d0 = complement_shift_op(fill->d0, (64ULL - shift_count));
        N64·bit_or(operand, operand, fill);
      }
      if(spill != NULL){
        spill->d0 = shift_op(spill->d0, shift_count);
        spill->d0 += complement_shift_op(given_operand->d0, (64ULL - shift_count));
      }

      return N64·Status·ok;
    }

    Local N64·Status N64·shift_left(uint64_t shift_count, N64·T *spill, N64·T *operand, N64·T *fill){
      return N64·shift(shift_count, spill, operand, fill, shift_left_op, shift_right_op);
    }

    Local N64·Status N64·shift_right(uint64_t shift_count, N64·T *spill, N64·T *operand, N64·T *fill){
      return N64·shift(shift_count, spill, operand, fill, shift_right_op, shift_left_op);
    }

    Local N64·Status N64·arithmetic_shift_right(uint64_t shift_count, N64·T *operand, N64·T *spill){
      if(shift_count > 63ULL) return N64·Status·gt_max_shift_count;

      // A NULL operand is treated as zero
      if(operand == NULL){
        operand = &N64·t[0];
        N64·copy(operand, N64·zero);
      }

      // sign bit check
      N64·T *fill = (operand->d0 & (1ULL << 63)) ? N64·all_one_bit : N64·zero;
      return N64·shift_right(shift_count, spill, operand, fill);
    }

    Local const N64·M N64·m = {
      .allocate_array = N64·allocate_array
     ,.allocate_array_zero = N64·allocate_array_zero
     ,.deallocate = N64·deallocate

     ,.copy = N64·copy
     ,.bit_and = N64·bit_and
     ,.bit_or = N64·bit_or
     ,.bit_complement = N64·bit_complement
     ,.bit_twos_complement = N64·bit_twos_complement
     ,.compare = N64·compare
     ,.lt = N64·lt
     ,.gt = N64·gt
     ,.eq = N64·eq
     ,.eq_zero = N64·eq_zero
     ,.accumulate = N64·accumulate
     ,.add = N64·add
     ,.increment = N64·increment
     ,.subtract = N64·subtract
     ,.multiply = N64·multiply
     ,.divide = N64·divide
     ,.modulus = N64·modulus
     ,.shift_left = N64·shift_left
     ,.shift_right = N64·shift_right
     ,.arithmetic_shift_right = N64·arithmetic_shift_right

     ,.access = N64·access
     ,.from_uint64 = N64·from_uint64
    };

  #endif

#endif
