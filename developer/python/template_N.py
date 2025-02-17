def N(namespace: str ,digit_type: str ,digit_extent: int ,constants_block: str) -> str:
    """
    Returns a source code file for cc to munch on
    """
    template = template_N()
    code = template.format(
        NAMESPACE = namespace
        ,DIGIT_TYPE = digit_type
        ,DIGIT_EXTENT = digit_extent
        ,CONSTANTS_BLOCK = constants_block
    )
    return code

def template_N():
    return r'''/*
  N32 - a processor native type

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

#define {NAMESPACE}·DEBUG

#ifndef FACE
#define {NAMESPACE}·IMPLEMENTATION
#define FACE
#endif

//--------------------------------------------------------------------------------
// Interface

#ifndef {NAMESPACE}·FACE
#define {NAMESPACE}·FACE

  #include <stdint.h>
  #include <stdbool.h>
  #include <stdarg.h>
  #include <stdlib.h>

  //----------------------------------------
  // Instance Data (Declaration Only)

  typedef uint32_t Extent;
  typedef uint32_t Digit;

  typedef struct {NAMESPACE}·T {NAMESPACE}·T;

  extern {NAMESPACE}·T *{NAMESPACE}·zero;
  extern {NAMESPACE}·T *{NAMESPACE}·one;
  extern {NAMESPACE}·T *{NAMESPACE}·all_one_bit;
  extern {NAMESPACE}·T *{NAMESPACE}·lsb;
  extern {NAMESPACE}·T *{NAMESPACE}·msb;

  //----------------------------------------
  // Return/Error Status and handlers

  typedef enum{{
    {NAMESPACE}·Status·ok = 0
    ,{NAMESPACE}·Status·overflow = 1
    ,{NAMESPACE}·Status·accumulator1_overflow = 2
    ,{NAMESPACE}·Status·carry = 3
    ,{NAMESPACE}·Status·borrow = 4
    ,{NAMESPACE}·Status·undefined_divide_by_zero = 5
    ,{NAMESPACE}·Status·undefined_modulus_zero = 6
    ,{NAMESPACE}·Status·gt_max_shift_count = 7
    ,{NAMESPACE}·Status·spill_eq_operand = 8 // not currently signaled, result will be spill value
    ,{NAMESPACE}·Status·one_word_product = 9
    ,{NAMESPACE}·Status·two_word_product = 10
  }} {NAMESPACE}·Status;

  typedef enum{{
    {NAMESPACE}·Order_lt = -1
    ,{NAMESPACE}·Order_eq = 0
    ,{NAMESPACE}·Order_gt = 1
  }} {NAMESPACE}·Order;

  typedef {NAMESPACE}·T *( *{NAMESPACE}·Allocate_MemoryFault )(Extent);

  //----------------------------------------
  // Interface

  typedef struct{{

    {NAMESPACE}·T *(*allocate_array_zero)(Extent, {NAMESPACE}·Allocate_MemoryFault);
    {NAMESPACE}·T *(*allocate_array)(Extent, {NAMESPACE}·Allocate_MemoryFault);
    void (*deallocate)({NAMESPACE}·T*);

    void (*copy)({NAMESPACE}·T*, {NAMESPACE}·T*);
    void (*bit_and)({NAMESPACE}·T*, {NAMESPACE}·T*, {NAMESPACE}·T*);
    void (*bit_or)({NAMESPACE}·T*, {NAMESPACE}·T*, {NAMESPACE}·T*);
    void (*bit_complement)({NAMESPACE}·T*, {NAMESPACE}·T*);
    void (*bit_twos_complement)({NAMESPACE}·T*, {NAMESPACE}·T*);
    {NAMESPACE}·Order (*compare)({NAMESPACE}·T*, {NAMESPACE}·T*);
    bool (*lt)({NAMESPACE}·T*, {NAMESPACE}·T*);
    bool (*gt)({NAMESPACE}·T*, {NAMESPACE}·T*);
    bool (*eq)({NAMESPACE}·T*, {NAMESPACE}·T*);
    bool (*eq_zero)({NAMESPACE}·T*);
    {NAMESPACE}·Status (*accumulate)({NAMESPACE}·T *accumulator1 ,{NAMESPACE}·T *accumulator0 ,...);
    {NAMESPACE}·Status (*add)({NAMESPACE}·T*, {NAMESPACE}·T*, {NAMESPACE}·T*);
    bool (*increment)({NAMESPACE}·T *a);
    {NAMESPACE}·Status (*subtract)({NAMESPACE}·T*, {NAMESPACE}·T*, {NAMESPACE}·T*);
    {NAMESPACE}·Status (*multiply)({NAMESPACE}·T*, {NAMESPACE}·T*, {NAMESPACE}·T*, {NAMESPACE}·T*);
    {NAMESPACE}·Status (*divide)({NAMESPACE}·T*, {NAMESPACE}·T*, {NAMESPACE}·T*, {NAMESPACE}·T*);
    {NAMESPACE}·Status (*modulus)({NAMESPACE}·T*, {NAMESPACE}·T*, {NAMESPACE}·T*);
    {NAMESPACE}·Status (*shift_left)(Extent, {NAMESPACE}·T*, {NAMESPACE}·T*, {NAMESPACE}·T*);
    {NAMESPACE}·Status (*shift_right)(Extent, {NAMESPACE}·T*, {NAMESPACE}·T*, {NAMESPACE}·T*);
    {NAMESPACE}·Status (*arithmetic_shift_right)(Extent, {NAMESPACE}·T*, {NAMESPACE}·T*);

    {NAMESPACE}·T* (*access)({NAMESPACE}·T*, Extent);
    void (*from_uint32)({NAMESPACE}·T *destination ,uint32_t value);
  }} {NAMESPACE}·Λ;

  Local const {NAMESPACE}·Λ {NAMESPACE}·λ; // initialized in the LOCAL section

#endif

//--------------------------------------------------------------------------------
// Implementation

#ifdef {NAMESPACE}·IMPLEMENTATION

  // this part goes into the library
  #ifndef LOCAL

    #include <stdarg.h>
    #include <stdlib.h>

    struct {NAMESPACE}·T{{
      Digit d0;
    }};

    {NAMESPACE}·T {NAMESPACE}·constant[4] = {{
      {{.d0 = 0}},
      {{.d0 = 1}},
      {{.d0 = ~(uint32_t)0}},
      {{.d0 = 1 << 31}}
    }};

    {NAMESPACE}·T *{NAMESPACE}·zero = &{NAMESPACE}·constant[0];
    {NAMESPACE}·T *{NAMESPACE}·one = &{NAMESPACE}·constant[1];
    {NAMESPACE}·T *{NAMESPACE}·all_one_bit = &{NAMESPACE}·constant[2];
    {NAMESPACE}·T *{NAMESPACE}·msb = &{NAMESPACE}·constant[3];
    {NAMESPACE}·T *{NAMESPACE}·lsb = &{NAMESPACE}·constant[1];

    // the allocate an array of N32
    {NAMESPACE}·T *{NAMESPACE}·allocate_array(Extent extent ,{NAMESPACE}·Allocate_MemoryFault memory_fault){{
      {NAMESPACE}·T *instance = malloc((extent + 1) * sizeof({NAMESPACE}·T) );
      if(!instance){{
        return memory_fault ? memory_fault(extent) : NULL;
      }}
      return instance;
    }}

    {NAMESPACE}·T *{NAMESPACE}·allocate_array_zero(Extent extent ,{NAMESPACE}·Allocate_MemoryFault memory_fault){{
      {NAMESPACE}·T *instance = calloc( extent + 1 ,sizeof({NAMESPACE}·T) );
      if(!instance){{
        return memory_fault ? memory_fault(extent) : NULL;
      }}
      return instance;
    }}

    void {NAMESPACE}·deallocate({NAMESPACE}·T *unencumbered){{
      free(unencumbered);
    }}

  #endif

  // This part is included after the library user's code
  #ifdef LOCAL

    // instance

    struct {NAMESPACE}·T{{
      Digit d0;
    }};

    // temporary variables
    // making these LOCAL rather than reserving one block in the library is thread safe
    // allocating a block once is more efficient
    // library code writes these, they are not on the interface

    Local {NAMESPACE}·T {NAMESPACE}·t[4];


    // allocation 

    extern {NAMESPACE}·T *{NAMESPACE}·allocate_array(Extent, {NAMESPACE}·Allocate_MemoryFault);
    extern {NAMESPACE}·T *{NAMESPACE}·allocate_array_zero(Extent, {NAMESPACE}·Allocate_MemoryFault);
    extern void {NAMESPACE}·deallocate({NAMESPACE}·T *);

    // so the user can access numbers in an array allocation
    Local {NAMESPACE}·T* {NAMESPACE}·access({NAMESPACE}·T *array ,Extent index){{
      return &array[index];
    }}

    Local void {NAMESPACE}·from_uint32({NAMESPACE}·T *destination ,uint32_t value){{
      if(destination == NULL) return;
      destination->d0 = value;
    }}

    // copy, convenience copy

    Local void {NAMESPACE}·copy({NAMESPACE}·T *destination ,{NAMESPACE}·T *source){{
      if(source == destination) return; // that was easy! 
      *destination = *source;
    }}

    Local void {NAMESPACE}·set_to_zero({NAMESPACE}·T *instance){{
      instance->d0 = 0;
    }}

    Local void {NAMESPACE}·set_to_one({NAMESPACE}·T *instance){{
      instance->d0 = 1;
    }}

    // bit operations

    Local void {NAMESPACE}·bit_and({NAMESPACE}·T *result, {NAMESPACE}·T *a, {NAMESPACE}·T *b){{
      result->d0 = a->d0 & b->d0;
    }}

    // result can be one of the operands
    Local void {NAMESPACE}·bit_or({NAMESPACE}·T *result, {NAMESPACE}·T *a, {NAMESPACE}·T *b){{
      result->d0 = a->d0 | b->d0;
    }}

    // result can the same as the operand
    Local void {NAMESPACE}·bit_complement({NAMESPACE}·T *result, {NAMESPACE}·T *a){{
      result->d0 = ~a->d0;
    }}

    // result can the same as the operand
    Local void {NAMESPACE}·bit_twos_complement({NAMESPACE}·T *result ,{NAMESPACE}·T *a){{
      result->d0 = ~a->d0 + 1;
    }}

    // test functions

    Local {NAMESPACE}·Order {NAMESPACE}·compare({NAMESPACE}·T *a, {NAMESPACE}·T *b){{
      if(a->d0 < b->d0) return {NAMESPACE}·Order_lt;
      if(a->d0 > b->d0) return {NAMESPACE}·Order_gt;
      return {NAMESPACE}·Order_eq;
    }}

    Local bool {NAMESPACE}·lt({NAMESPACE}·T *a ,{NAMESPACE}·T *b){{
      return  a->d0 < b->d0;
    }}    

    Local bool {NAMESPACE}·gt({NAMESPACE}·T *a ,{NAMESPACE}·T *b){{
      return  a->d0 > b->d0;
    }}    

    Local bool {NAMESPACE}·eq({NAMESPACE}·T *a ,{NAMESPACE}·T *b){{
      return  a->d0 == b->d0;
    }}    

    Local bool {NAMESPACE}·eq_zero({NAMESPACE}·T *a){{
      return  a->d0 == 0;
    }}    


    // arithmetic operations

    // For a large number of summands for the lower precision Natural implementations, for accumulate/add/sub, the 'overflow' operand could overflow and thus this routine will halt and return {NAMESPACE}·Status·accumulator1_overflow
    //
    // When accumulator1 and accumulator0 point to the same location, the result is the accumulator1 value.
    Local {NAMESPACE}·Status {NAMESPACE}·accumulate({NAMESPACE}·T *accumulator1 ,{NAMESPACE}·T *accumulator0 ,...){{

      va_list args;
      va_start(args ,accumulator0);
      uint32_t sum = accumulator0->d0;
      uint32_t carry = 0;
      {NAMESPACE}·T *current;

      while( (current = va_arg(args ,{NAMESPACE}·T *)) ){{
        sum += current->d0;
        if(sum < current->d0){{  // Accumulator1 into carry
          (carry)++;
          if(carry == 0){{
            va_end(args);
            return {NAMESPACE}·Status·accumulator1_overflow;
          }}
        }}
      }}
      va_end(args);

      // wipes out prior value of accumulator1
      accumulator1->d0 = carry;

      return {NAMESPACE}·Status·ok;
    }}

    Local {NAMESPACE}·Status {NAMESPACE}·add({NAMESPACE}·T *sum ,{NAMESPACE}·T *a ,{NAMESPACE}·T *b){{
      uint64_t result = (uint64_t)a->d0 + (uint64_t)b->d0;
      sum->d0 = (uint32_t)result;
      return (result >> 32) ? {NAMESPACE}·Status·carry : {NAMESPACE}·Status·ok;
    }}

    Local bool {NAMESPACE}·increment({NAMESPACE}·T *a){{
      a->d0++;
      return a->d0 == 0;
    }}

    Local {NAMESPACE}·Status {NAMESPACE}·subtract({NAMESPACE}·T *difference ,{NAMESPACE}·T *a ,{NAMESPACE}·T *b){{
      uint64_t diff = (uint64_t) a->d0 - (uint64_t) b->d0;
      difference->d0 = (uint32_t)diff;
      return (diff > a->d0) ? {NAMESPACE}·Status·borrow : {NAMESPACE}·Status·ok;
    }}


    Local {NAMESPACE}·Status {NAMESPACE}·multiply({NAMESPACE}·T *product1 ,{NAMESPACE}·T *product0 ,{NAMESPACE}·T *a ,{NAMESPACE}·T *b){{
      uint64_t product = (uint64_t)a->d0 * (uint64_t)b->d0;
      product0->d0 = (uint32_t)product;
      product1->d0 = (uint32_t)(product >> 32);

      if(product1->d0 == 0) return {NAMESPACE}·Status·one_word_product;
      return {NAMESPACE}·Status·two_word_product;
    }}

    Local {NAMESPACE}·Status {NAMESPACE}·divide({NAMESPACE}·T *remainder ,{NAMESPACE}·T *quotient ,{NAMESPACE}·T *a ,{NAMESPACE}·T *b){{
      if(b->d0 == 0) return {NAMESPACE}·Status·undefined_divide_by_zero; 

      quotient->d0 = a->d0 / b->d0;
      remainder->d0 = a->d0 - (quotient->d0 * b->d0);

      return {NAMESPACE}·Status·ok;
    }}

    Local {NAMESPACE}·Status {NAMESPACE}·modulus({NAMESPACE}·T *remainder ,{NAMESPACE}·T *a ,{NAMESPACE}·T *b){{
      if(b->d0 == 0) return {NAMESPACE}·Status·undefined_modulus_zero; 
      uint32_t quotient = a->d0 / b->d0;
      remainder->d0 = a->d0 - (quotient * b->d0);
      return {NAMESPACE}·Status·ok;
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
    Local {NAMESPACE}·Status {NAMESPACE}·shift
    (
     uint32_t shift_count
     ,{NAMESPACE}·T *spill
     ,{NAMESPACE}·T *operand
     ,{NAMESPACE}·T *fill
     ,ShiftOp shift_op
     ,ShiftOp complement_shift_op
     ){{

      // If no result is needed, return immediately.
      if(operand == NULL && spill == NULL) return {NAMESPACE}·Status·ok;

      // Treat NULL operand as zero.
      if(operand == NULL){{
        operand = &{NAMESPACE}·t[0];
        {NAMESPACE}·copy(operand, {NAMESPACE}·zero);
      }}

      // Shifting more than one word breaks our fill/spill model.
      if(shift_count > 31) return {NAMESPACE}·Status·gt_max_shift_count;

      // The given operand is still required after it is modified, so we copy it.
      {NAMESPACE}·T *given_operand = &{NAMESPACE}·t[1];
      {NAMESPACE}·copy(given_operand, operand);

      // Perform the shift
      operand->d0 = shift_op(given_operand->d0, shift_count);
      if(fill != NULL){{
        fill->d0 = complement_shift_op(fill->d0, (32 - shift_count));
        {NAMESPACE}·bit_or(operand, operand, fill);
      }}
      if(spill != NULL){{
        spill->d0 = shift_op(spill->d0, shift_count);
        spill->d0 += complement_shift_op(given_operand->d0, (32 - shift_count));
      }}

      return {NAMESPACE}·Status·ok;
    }}

    // Define concrete shift functions using valid C function pointers
    Local {NAMESPACE}·Status 
    {NAMESPACE}·shift_left(uint32_t shift_count, {NAMESPACE}·T *spill, {NAMESPACE}·T *operand, {NAMESPACE}·T *fill){{
      return {NAMESPACE}·shift(shift_count, spill, operand, fill, shift_left_op, shift_right_op);
    }}

    Local {NAMESPACE}·Status 
    {NAMESPACE}·shift_right(uint32_t shift_count, {NAMESPACE}·T *spill, {NAMESPACE}·T *operand, {NAMESPACE}·T *fill){{
      return {NAMESPACE}·shift(shift_count, spill, operand, fill, shift_right_op, shift_left_op);
    }}

    Local {NAMESPACE}·Status 
    {NAMESPACE}·arithmetic_shift_right(uint32_t shift_count, {NAMESPACE}·T *operand, {NAMESPACE}·T *spill){{

      // Guard against excessive shift counts
      if(shift_count > 31) return {NAMESPACE}·Status·gt_max_shift_count;

      // A NULL operand is treated as zero
      if(operand == NULL){{
        operand = &{NAMESPACE}·t[0];
        {NAMESPACE}·copy(operand, {NAMESPACE}·zero);
      }}

      // Pick the fill value based on the sign bit
      {NAMESPACE}·T *fill = (operand->d0 & 0x80000000) ? {NAMESPACE}·all_one_bit : {NAMESPACE}·zero;

      // Call shift_right with the appropriate fill
      return {NAMESPACE}·shift_right(shift_count, spill, operand, fill);
    }}

    Local const {NAMESPACE}·Λ {NAMESPACE}·λ = {{

      .allocate_array = {NAMESPACE}·allocate_array
      ,.allocate_array_zero = {NAMESPACE}·allocate_array_zero
      ,.deallocate = {NAMESPACE}·deallocate

      ,.copy = {NAMESPACE}·copy
      ,.bit_and = {NAMESPACE}·bit_and
      ,.bit_or = {NAMESPACE}·bit_or
      ,.bit_complement = {NAMESPACE}·bit_complement
      ,.bit_twos_complement = {NAMESPACE}·bit_twos_complement
      ,.compare = {NAMESPACE}·compare
      ,.lt = {NAMESPACE}·lt
      ,.gt = {NAMESPACE}·gt
      ,.eq = {NAMESPACE}·eq
      ,.eq_zero = {NAMESPACE}·eq_zero
      ,.accumulate = {NAMESPACE}·accumulate
      ,.add = {NAMESPACE}·add
      ,.increment = {NAMESPACE}·increment
      ,.subtract = {NAMESPACE}·subtract
      ,.multiply = {NAMESPACE}·multiply
      ,.divide = {NAMESPACE}·divide
      ,.modulus = {NAMESPACE}·modulus
      ,.shift_left = {NAMESPACE}·shift_left
      ,.shift_right = {NAMESPACE}·shift_right
      ,.arithmetic_shift_right = {NAMESPACE}·arithmetic_shift_right

      ,.access = {NAMESPACE}·access
      ,.from_uint32 = {NAMESPACE}·from_uint32
    }};

  #endif

#endif
'''
