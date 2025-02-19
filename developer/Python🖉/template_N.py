def N(namespace: str ,digit_type: str ,digit_extent: int ,constants_block: str) -> str:
    """
    Returns a source code file for cc to munch on
    """
    template = template_N()
    code = template.format(
        NS = namespace
        ,DIGIT_TYPE = digit_type
        ,DIGIT_EXTENT = digit_extent
        ,CONSTANTS_BLOCK = constants_block
    )
    return code

def template_N():
    return r'''
/*
  M - The type for the function dictionary.
  m - A function dictionary.
  T - Is the type for the tableau.  The tableau is a memory shared among the functions
      In the function dictionary.

*/
#define {NS}·DEBUG

#ifndef FACE
#define {NS}·IMPLEMENTATION
#define FACE
#endif

//--------------------------------------------------------------------------------
// Interface

#ifndef {NS}·FACE
#define {NS}·FACE

  #include <stdint.h>
  #include <stdbool.h>
  #include <stdarg.h>
  #include <stdlib.h>

  //----------------------------------------
  // Instance Data (Declaration Only)

  typedef uint32_t Extent;
  typedef uint32_t Digit;

  typedef struct {NS}·T {NS}·T;

  extern {NS}·T *{NS}·zero;
  extern {NS}·T *{NS}·one;
  extern {NS}·T *{NS}·all_one_bit;
  extern {NS}·T *{NS}·lsb;
  extern {NS}·T *{NS}·msb;

  //----------------------------------------
  // Return/Error Status and handlers

  typedef enum{{
    {NS}·Status·ok = 0
    ,{NS}·Status·overflow = 1
    ,{NS}·Status·accumulator1_overflow = 2
    ,{NS}·Status·carry = 3
    ,{NS}·Status·borrow = 4
    ,{NS}·Status·undefined_divide_by_zero = 5
    ,{NS}·Status·undefined_modulus_zero = 6
    ,{NS}·Status·gt_max_shift_count = 7
    ,{NS}·Status·spill_eq_operand = 8 // not currently signaled, result will be spill value
    ,{NS}·Status·one_word_product = 9
    ,{NS}·Status·two_word_product = 10
  }} {NS}·Status;

  typedef enum{{
    {NS}·Order_lt = -1
    ,{NS}·Order_eq = 0
    ,{NS}·Order_gt = 1
  }} {NS}·Order;

  typedef {NS}·T *( *{NS}·Allocate_MemoryFault )(Extent);

  //----------------------------------------
  // Interface

  typedef struct{{

    {NS}·T *(*allocate_array_zero)(Extent, {NS}·Allocate_MemoryFault);
    {NS}·T *(*allocate_array)(Extent, {NS}·Allocate_MemoryFault);
    void (*deallocate)({NS}·T*);

    void (*copy)({NS}·T*, {NS}·T*);
    void (*bit_and)({NS}·T*, {NS}·T*, {NS}·T*);
    void (*bit_or)({NS}·T*, {NS}·T*, {NS}·T*);
    void (*bit_complement)({NS}·T*, {NS}·T*);
    void (*bit_twos_complement)({NS}·T*, {NS}·T*);
    {NS}·Order (*compare)({NS}·T*, {NS}·T*);
    bool (*lt)({NS}·T*, {NS}·T*);
    bool (*gt)({NS}·T*, {NS}·T*);
    bool (*eq)({NS}·T*, {NS}·T*);
    bool (*eq_zero)({NS}·T*);
    {NS}·Status (*accumulate)({NS}·T *accumulator1 ,{NS}·T *accumulator0 ,...);
    {NS}·Status (*add)({NS}·T*, {NS}·T*, {NS}·T*);
    bool (*increment)({NS}·T *a);
    {NS}·Status (*subtract)({NS}·T*, {NS}·T*, {NS}·T*);
    {NS}·Status (*multiply)({NS}·T*, {NS}·T*, {NS}·T*, {NS}·T*);
    {NS}·Status (*divide)({NS}·T*, {NS}·T*, {NS}·T*, {NS}·T*);
    {NS}·Status (*modulus)({NS}·T*, {NS}·T*, {NS}·T*);
    {NS}·Status (*shift_left)(Extent, {NS}·T*, {NS}·T*, {NS}·T*);
    {NS}·Status (*shift_right)(Extent, {NS}·T*, {NS}·T*, {NS}·T*);
    {NS}·Status (*arithmetic_shift_right)(Extent, {NS}·T*, {NS}·T*);

    {NS}·T* (*access)({NS}·T*, Extent);
    void (*from_uint32)({NS}·T *destination ,uint32_t value);
  }} {NS}·Λ;

  Local const {NS}·Λ {NS}·λ; // initialized in the LOCAL section

#endif

//--------------------------------------------------------------------------------
// Implementation

#ifdef {NS}·IMPLEMENTATION

  // this part goes into the library
  #ifndef LOCAL

    #include <stdarg.h>
    #include <stdlib.h>

    struct {NS}·T{{
      Digit d0;
    }};

    {NS}·T {NS}·constant[4] = {{
      {{.d0 = 0}},
      {{.d0 = 1}},
      {{.d0 = ~(uint32_t)0}},
      {{.d0 = 1 << 31}}
    }};

    {NS}·T *{NS}·zero = &{NS}·constant[0];
    {NS}·T *{NS}·one = &{NS}·constant[1];
    {NS}·T *{NS}·all_one_bit = &{NS}·constant[2];
    {NS}·T *{NS}·msb = &{NS}·constant[3];
    {NS}·T *{NS}·lsb = &{NS}·constant[1];

    // the allocate an array of N32
    {NS}·T *{NS}·allocate_array(Extent extent ,{NS}·Allocate_MemoryFault memory_fault){{
      {NS}·T *instance = malloc((extent + 1) * sizeof({NS}·T) );
      if(!instance){{
        return memory_fault ? memory_fault(extent) : NULL;
      }}
      return instance;
    }}

    {NS}·T *{NS}·allocate_array_zero(Extent extent ,{NS}·Allocate_MemoryFault memory_fault){{
      {NS}·T *instance = calloc( extent + 1 ,sizeof({NS}·T) );
      if(!instance){{
        return memory_fault ? memory_fault(extent) : NULL;
      }}
      return instance;
    }}

    void {NS}·deallocate({NS}·T *unencumbered){{
      free(unencumbered);
    }}

  #endif

  // This part is included after the library user's code
  #ifdef LOCAL

    // instance

    struct {NS}·T{{
      Digit d0;
    }};

    // temporary variables
    // making these LOCAL rather than reserving one block in the library is thread safe
    // allocating a block once is more efficient
    // library code writes these, they are not on the interface

    Local {NS}·T {NS}·t[4];


    // allocation 

    extern {NS}·T *{NS}·allocate_array(Extent, {NS}·Allocate_MemoryFault);
    extern {NS}·T *{NS}·allocate_array_zero(Extent, {NS}·Allocate_MemoryFault);
    extern void {NS}·deallocate({NS}·T *);

    // so the user can access numbers in an array allocation
    Local {NS}·T* {NS}·access({NS}·T *array ,Extent index){{
      return &array[index];
    }}

    Local void {NS}·from_uint32({NS}·T *destination ,uint32_t value){{
      if(destination == NULL) return;
      destination->d0 = value;
    }}

    // copy, convenience copy

    Local void {NS}·copy({NS}·T *destination ,{NS}·T *source){{
      if(source == destination) return; // that was easy! 
      *destination = *source;
    }}

    Local void {NS}·set_to_zero({NS}·T *instance){{
      instance->d0 = 0;
    }}

    Local void {NS}·set_to_one({NS}·T *instance){{
      instance->d0 = 1;
    }}

    // bit operations

    Local void {NS}·bit_and({NS}·T *result, {NS}·T *a, {NS}·T *b){{
      result->d0 = a->d0 & b->d0;
    }}

    // result can be one of the operands
    Local void {NS}·bit_or({NS}·T *result, {NS}·T *a, {NS}·T *b){{
      result->d0 = a->d0 | b->d0;
    }}

    // result can the same as the operand
    Local void {NS}·bit_complement({NS}·T *result, {NS}·T *a){{
      result->d0 = ~a->d0;
    }}

    // result can the same as the operand
    Local void {NS}·bit_twos_complement({NS}·T *result ,{NS}·T *a){{
      result->d0 = ~a->d0 + 1;
    }}

    // test functions

    Local {NS}·Order {NS}·compare({NS}·T *a, {NS}·T *b){{
      if(a->d0 < b->d0) return {NS}·Order_lt;
      if(a->d0 > b->d0) return {NS}·Order_gt;
      return {NS}·Order_eq;
    }}

    Local bool {NS}·lt({NS}·T *a ,{NS}·T *b){{
      return  a->d0 < b->d0;
    }}    

    Local bool {NS}·gt({NS}·T *a ,{NS}·T *b){{
      return  a->d0 > b->d0;
    }}    

    Local bool {NS}·eq({NS}·T *a ,{NS}·T *b){{
      return  a->d0 == b->d0;
    }}    

    Local bool {NS}·eq_zero({NS}·T *a){{
      return  a->d0 == 0;
    }}    


    // arithmetic operations

    // For a large number of summands for the lower precision Natural implementations, for accumulate/add/sub, the 'overflow' operand could overflow and thus this routine will halt and return {NS}·Status·accumulator1_overflow
    //
    // When accumulator1 and accumulator0 point to the same location, the result is the accumulator1 value.
    Local {NS}·Status {NS}·accumulate({NS}·T *accumulator1 ,{NS}·T *accumulator0 ,...){{

      va_list args;
      va_start(args ,accumulator0);
      uint32_t sum = accumulator0->d0;
      uint32_t carry = 0;
      {NS}·T *current;

      while( (current = va_arg(args ,{NS}·T *)) ){{
        sum += current->d0;
        if(sum < current->d0){{  // Accumulator1 into carry
          (carry)++;
          if(carry == 0){{
            va_end(args);
            return {NS}·Status·accumulator1_overflow;
          }}
        }}
      }}
      va_end(args);

      // wipes out prior value of accumulator1
      accumulator1->d0 = carry;

      return {NS}·Status·ok;
    }}

    Local {NS}·Status {NS}·add({NS}·T *sum ,{NS}·T *a ,{NS}·T *b){{
      uint64_t result = (uint64_t)a->d0 + (uint64_t)b->d0;
      sum->d0 = (uint32_t)result;
      return (result >> 32) ? {NS}·Status·carry : {NS}·Status·ok;
    }}

    Local bool {NS}·increment({NS}·T *a){{
      a->d0++;
      return a->d0 == 0;
    }}

    Local {NS}·Status {NS}·subtract({NS}·T *difference ,{NS}·T *a ,{NS}·T *b){{
      uint64_t diff = (uint64_t) a->d0 - (uint64_t) b->d0;
      difference->d0 = (uint32_t)diff;
      return (diff > a->d0) ? {NS}·Status·borrow : {NS}·Status·ok;
    }}


    Local {NS}·Status {NS}·multiply({NS}·T *product1 ,{NS}·T *product0 ,{NS}·T *a ,{NS}·T *b){{
      uint64_t product = (uint64_t)a->d0 * (uint64_t)b->d0;
      product0->d0 = (uint32_t)product;
      product1->d0 = (uint32_t)(product >> 32);

      if(product1->d0 == 0) return {NS}·Status·one_word_product;
      return {NS}·Status·two_word_product;
    }}

    Local {NS}·Status {NS}·divide({NS}·T *remainder ,{NS}·T *quotient ,{NS}·T *a ,{NS}·T *b){{
      if(b->d0 == 0) return {NS}·Status·undefined_divide_by_zero; 

      quotient->d0 = a->d0 / b->d0;
      remainder->d0 = a->d0 - (quotient->d0 * b->d0);

      return {NS}·Status·ok;
    }}

    Local {NS}·Status {NS}·modulus({NS}·T *remainder ,{NS}·T *a ,{NS}·T *b){{
      if(b->d0 == 0) return {NS}·Status·undefined_modulus_zero; 
      uint32_t quotient = a->d0 / b->d0;
      remainder->d0 = a->d0 - (quotient * b->d0);
      return {NS}·Status·ok;
    }}

    // bit motion

    typedef uint32_t (*ShiftOp)(uint32_t, uint32_t);

    Local uint32_t shift_left_op(uint32_t value, uint32_t amount){{
      return value << amount;
    }}

    Local uint32_t shift_right_op(uint32_t value, uint32_t amount){{
      return value >> amount;
    }}

    // modifies all three of its operands
    // in the case of duplicate operands this is the order: first modifies operand, then fill, then spill, 
    Local {NS}·Status {NS}·shift
    (
     uint32_t shift_count
     ,{NS}·T *spill
     ,{NS}·T *operand
     ,{NS}·T *fill
     ,ShiftOp shift_op
     ,ShiftOp complement_shift_op
     ){{

      // If no result is needed, return immediately.
      if(operand == NULL && spill == NULL) return {NS}·Status·ok;

      // Treat NULL operand as zero.
      if(operand == NULL){{
        operand = &{NS}·t[0];
        {NS}·copy(operand, {NS}·zero);
      }}

      // Shifting more than one word breaks our fill/spill model.
      if(shift_count > 31) return {NS}·Status·gt_max_shift_count;

      // The given operand is still required after it is modified, so we copy it.
      {NS}·T *given_operand = &{NS}·t[1];
      {NS}·copy(given_operand, operand);

      // Perform the shift
      operand->d0 = shift_op(given_operand->d0, shift_count);
      if(fill != NULL){{
        fill->d0 = complement_shift_op(fill->d0, (32 - shift_count));
        {NS}·bit_or(operand, operand, fill);
      }}
      if(spill != NULL){{
        spill->d0 = shift_op(spill->d0, shift_count);
        spill->d0 += complement_shift_op(given_operand->d0, (32 - shift_count));
      }}

      return {NS}·Status·ok;
    }}

    // Define concrete shift functions using valid C function pointers
    Local {NS}·Status 
    {NS}·shift_left(uint32_t shift_count, {NS}·T *spill, {NS}·T *operand, {NS}·T *fill){{
      return {NS}·shift(shift_count, spill, operand, fill, shift_left_op, shift_right_op);
    }}

    Local {NS}·Status 
    {NS}·shift_right(uint32_t shift_count, {NS}·T *spill, {NS}·T *operand, {NS}·T *fill){{
      return {NS}·shift(shift_count, spill, operand, fill, shift_right_op, shift_left_op);
    }}

    Local {NS}·Status 
    {NS}·arithmetic_shift_right(uint32_t shift_count, {NS}·T *operand, {NS}·T *spill){{

      // Guard against excessive shift counts
      if(shift_count > 31) return {NS}·Status·gt_max_shift_count;

      // A NULL operand is treated as zero
      if(operand == NULL){{
        operand = &{NS}·t[0];
        {NS}·copy(operand, {NS}·zero);
      }}

      // Pick the fill value based on the sign bit
      {NS}·T *fill = (operand->d0 & 0x80000000) ? {NS}·all_one_bit : {NS}·zero;

      // Call shift_right with the appropriate fill
      return {NS}·shift_right(shift_count, spill, operand, fill);
    }}

    Local const {NS}·Λ {NS}·λ = {{

      .allocate_array = {NS}·allocate_array
      ,.allocate_array_zero = {NS}·allocate_array_zero
      ,.deallocate = {NS}·deallocate

      ,.copy = {NS}·copy
      ,.bit_and = {NS}·bit_and
      ,.bit_or = {NS}·bit_or
      ,.bit_complement = {NS}·bit_complement
      ,.bit_twos_complement = {NS}·bit_twos_complement
      ,.compare = {NS}·compare
      ,.lt = {NS}·lt
      ,.gt = {NS}·gt
      ,.eq = {NS}·eq
      ,.eq_zero = {NS}·eq_zero
      ,.accumulate = {NS}·accumulate
      ,.add = {NS}·add
      ,.increment = {NS}·increment
      ,.subtract = {NS}·subtract
      ,.multiply = {NS}·multiply
      ,.divide = {NS}·divide
      ,.modulus = {NS}·modulus
      ,.shift_left = {NS}·shift_left
      ,.shift_right = {NS}·shift_right
      ,.arithmetic_shift_right = {NS}·arithmetic_shift_right

      ,.access = {NS}·access
      ,.from_uint32 = {NS}·from_uint32
    }};

  #endif

#endif
'''
