/*
  N_32 - a processor native type

  For binary operations:  a op b -> c

  To use N_32, first allocate a block of N_32. Do the arithmetic,
  if any results need be kept, copy them to another block. Then deallocate
  the block. Do not allocate numbers one at a time, or it would be slow.

*/

#ifndef IFACE
#define N_32·IMPLEMENTATION
#define IFACE
#endif

#ifndef N_32·IFACE
#define N_32·IFACE

  //----------------------------------------
  // The instance data
  //   no way to avoid adding this definition to the interface due to the use of inline functions

    typedef uint32_t Extent;
    typedef uint32_t Digit;;

    struct N_32 {
      Digit d0;
    };

    const N_32 N_32·zero;
    const N_32 N_32·one;
    const N_32 N_32·all_ones;


  //----------------------------------------
  // error status return values, and error handlers
  //

    typedef enum {
      N_32·Status·ok = 0
      ,N_32·Status·overflow = 1
      ,N_32·Status·accumulator_overflow = 2
      ,N_32·Status·carry = 3
      ,N_32·Status·borrow = 4
      ,N_32·Status·undefined_divide_by_zero = 5
      ,N_32·Status·undefined_modulus_zero = 6
    } N_32·Status;

    typedef enum {
      N_32·Order·lt = -1  // Less Than
      ,N_32·Order·eq = 0   // Equal
      ,N_32·Order·gt = 1    // Greater Than
    } N_32·Order;

    typedef N_32 *( *N_32·Allocate·MemoryFault )(Extent);

  //----------------------------------------
  // inline interface
  //

    // copy, convenience copy

    inline void N_32·copy(N_32 *destination ,N_32 *source){
      *destination = *source;
    }

    inline void N_32·set_to_zero(N_32 *instance){
      instance->d0 = 0;
    }

    inline void N_32·set_to_one(N_32 *instance){
      instance->d0 = 1;
    }

    // bit operations

    inline void N_32·and(N_32 *result ,N_32 *a ,N_32 *b){
      result->d0 = a->d0 & b->d0;
    }

    inline void N_32·or(N_32 *result ,N_32 *a ,N_32 *b){
      result->d0 = a->d0 | b->d0;
    }

    inline void N_32·complement(N_32 *result ,N_32 *a){
      result->d0 = ~a->d0;
    }

    inline void N_32·twos_complement(N_32 *result ,N_32 *a){
      result->d0 = ~a->d0 + 1;
    }

    inline bool N_32·lsb_is_set(N_32 *a){
      return a->d0 & 0x0000001;
    }

    inline bool N_32·msb_is_set(N_32 *a){
      return a->d0 & 0x8000000;
    }

    // for low precision Natural, and large number of summands for accumulate/add/sub, overflow could overflow and thus this op would fail
    inline N_32·Status N_32·accumulate(N_32 *overflow ,N_32 *accumulator ,...){
      va_list args;
      va_start(args ,accumulator);

      uint64_t *sum = &accumulator->d0;
      uint64_t *carry = &overflow->d0;
      N_32 *current;

      while( (current = va_arg(args ,N_32 *)) ){
        *sum += current->d0;
        if(*sum < current->d0){  // Overflow into carry
          (*carry)++;
          if(*carry == 0){
            va_end(args);
            return N_32·Status·accumulator_overflow;
          }
        }
      }
      va_end(args);

      if(*carry == 0) return N_32·Status·ok;
      return N_32·Status·OVERFLOW;
    }

    inline N_32·Order N_32·compare(N_32 *a ,N_32 *b){
      if(a->d0 < b->d0) return N_32·order·lt;
      if(a->d0 > b->d0) return N_32·order·gt;
      return N_32·order·eq;
    }    

    inline  bool N_32·lt(N_32 *a ,N_32 *b){
      return  a->d0 < b->d0;
    }    

    inline  bool N_32·gt(N_32 *a ,N_32 *b){
      return  a->d0 > b->d0;
    }    

    inline  bool N_32·eq(N_32 *a ,N_32 *b){
      return  a->d0 == b->d0;
    }    

    // arithmetic operations

    inline N_32·Status N_32·add(N_32 *sum ,N_32 *a ,N_32 *b){
      uint64_t result = (uint64_t)a->d0 + (uint64_t)b->d0;
      sum->d0 = (uint32_t)result;
      return (result >> 32) ? N_32·Status·carry : N_32·Status·ok;
    }

    inline N_32·Status N_32·next(N_32 *overflow ,N_32 *result ,N_32 *a){
      uint64_t sum = (uint64_t)a->d0 + (uint64_t)1;
      result->d0 = (uint32_t)sum;
      overflow->d0 = (uint32_t)(sum >> 32);

      if(overflow->d0 == 0) return N_32·status·ok;
      return N_32·status·overflow;
    }

    inline N_32·Status N_32·subtract(N_32 *difference ,N_32 *a ,N_32 *b){
      uint64_t diff = (uint64_t)a->d0 - (uint64_t)b->d0;
      difference->d0 = (uint32_t)diff;
      return (diff > a->d0) ? N_32·Status·borrow : N_32·Status·ok;
    }

    inline N_32·Status N_32·multiply(N_32 *overflow ,N_32 *result ,N_32 *a ,N_32 *b){
      uint64_t product = (uint64_t)a->d0 * (uint64_t)b->d0;
      result->d0 = (uint32_t)product;
      overflow->d0 = (uint32_t)(product >> 32);

      if(overflow->d0 == 0) return N_32·status·ok;
      return N_32·status·overflow;
    }

    inline N_32·Status N_32·divide(N_32 *remainder ,N_32 *quotient ,N_32 *a ,N_32 *b){
      if(b->d0 == 0) return N_32·Status·undefined_divide_by_zero; 

      quotient->d0 = a->d0 / b->d0;
      remainder->d0 = a->d0 - (quotient->d0 * b->d0);

      return N_32·Status·ok;
    }

    inline N_32·Status N_32·modulus(N_32 *remainder ,N_32 *a ,N_32 *b){
      if(b->d0 == 0) return N_32·Status·undefined_modulus_zero; 
      uint32_t quotient = a->d0 / b->d0;
      remainder->d0 = a->d0 - (quotient * b->d0);
      return N_32·Status·ok;
    }

    // shift
    inline void N_32·shift_left(Extent shift ,N_32 *sink ,N_32 *result ,N_32 *source){
      *sink = *source;
      result->d0 = source->d0 << shift;
      sink->d0 = source->d0 >> (32 - shift);
    }

    inline void N_32·shift_right(Extent shift ,N_32 *source ,N_32 *result ,N_32 *sink){
      *sink = *source;
      result->d0 = source->d0 >> shift;
      sink->d0 = source->d0 << (32 - shift);
    }

    inline void N_32·arithmetic_shift_right(Extent shift ,N_32 *result ,N_32 *sink){
      N_32 source;
      N_32 source = msb_is_set(result) ? N_32·all_ones : N_32·zero;
      N_32·shift_right(shift ,&source ,result ,sink);
    }

  //----------------------------------------
  // compiled interface

  typedef struct {
    N_32·Allocate allocate;
    N_32·Zero zero;
    N_32·One one;
    N_32·Deallocate deallocate;
  } N_32·Interface;

  extern const N_32·Interface N_32·interface;


#endif

#ifdef N_32·IMPLEMENTATION

  #include <stdarg.h>
  #include <stdlib.h>

  const N_32 N_32·zero = { .d0 = 0 };
  const N_32 N_32·one = { .d0 = 1 };
  const N_32 N_32·all_ones = { .d0 = ~(uint32_t)0 };

   // the allocate an array of N_32
  N_32 *N_32·allocate( Extent extent ,N_32 *(*memory_fault)(Extent) ){
    N_32 *instance = malloc((extent + 1) * sizeof(N_32) );
    if(!instance){
      return memory_fault ? memory_fault(extent) : NULL;
    }
    return instance;
  }

  N_32 *N_32·alloc_zero( Extent extent ,N_32 *(*memory_fault)(Extent) ){
    N_32 *instance = calloc( extent + 1 ,sizeof(N_32) );
    if(!instance){
      return memory_fault ? memory_fault(extent) : NULL;
    }
    return instance;
  }

  // initialize all with x
  N_32 *N_32·alloc_x(Extent extent ,N_32 *(*memory_fault)(Extent) ,N_32 *x){
    N_32 *instance = malloc((extent + 1) * sizeof(N_32));
    if(!instance){
      return memory_fault ? memory_fault(extent) : NULL;
    }
    N_32 *pt = instance;
    while( pt <= instance + extent ){
      *pt = *x;
      pt++;
    }
    return instance;
  }

  const N_32·Interface N_32·interface = {
    .allocate = N_32·allocate
    ,.allocate_zero = N_32·alloc_zero
    ,.allocate_all_x = N_32·alloc_x
    ,.deallocate = N_32·deallocate
  };

  void N_32·deallocate(N_32 *unencumbered){
    free(unencumbered);
  }

#endif
