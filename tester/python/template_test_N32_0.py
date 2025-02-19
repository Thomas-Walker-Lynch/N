# {NS} must be a N32 type.
#
# test_N32 can be used to test any 32 bit natural number types. As examples: N32PN the processor native type; N32_4_by_8 a 32 bit natural number type made of 4 digits, where each digit is 8 bits.
#

def test_N32_0(namespace: str) -> str:
    """
    Returns a source code file for cc.
    """
    template = template_test_N32_0()
    code = template.format(
        NS = namespace
    )
    return code

def template_test_N32_0():
    return r'''#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <setjmp.h>

// Enable interface section
#define FACE
#include "{NS}.lib.c"
#undef FACE

// Jump buffer for signal handling
static sigjmp_buf jump_buffer;

// Signal handler for catching fatal errors
void signal_handler(int signal){{
  siglongjmp(jump_buffer ,1); // Jump back to test_head on error
}}

// Test function prototypes
bool test_copy();
bool test_bitwise_operations();
bool test_comparisons();
bool test_arithmetic();
bool test_shifts();

// Test array (null-terminated)
typedef bool (*TestFunction)();
typedef struct{{
  TestFunction function;
  const char *name;
}}TestEntry;

TestEntry test_list[] = {{
   {{test_copy ,"test_copy"}}
  ,{{test_bitwise_operations ,"test_bitwise_operations"}}
  ,{{test_comparisons ,"test_comparisons"}}
  ,{{test_arithmetic ,"test_arithmetic"}}
  ,{{test_shifts ,"test_shifts"}}
  ,{{NULL ,NULL}}  // Null termination
}};

// The test runner
int test_head(){{
  int pass_count = 0;
  int fail_count = 0;

  // Set up signal handlers
  signal(SIGSEGV ,signal_handler);  // Catch segmentation faults
  signal(SIGFPE ,signal_handler);   // Catch floating point errors
  signal(SIGABRT ,signal_handler);  // Catch abort() calls

  for(TestEntry *entry = test_list; entry->function != NULL; entry++){{
    if( sigsetjmp(jump_buffer ,1) == 0 ){{
      // Run the test normally
      if( !entry->function() ){{
        printf("Failed: %s\n" ,entry->name);
        fail_count++;
      }}else{{
        pass_count++;
      }}
    }}else{{
      // If a signal was caught
      printf("Failed due to signaling: %s\n" ,entry->name);
      fail_count++;
    }}
  }}

  printf("Tests passed: %d\n" ,pass_count);
  printf("Tests failed: %d\n" ,fail_count);
  return (fail_count == 0) ? 0 : 1;
}}

// Main function
int main(int argc ,char **argv){{
  return test_head();
}}

//------------------------------------------------------------------------------
// Test Implementations
//------------------------------------------------------------------------------

bool test_copy(){{
  // Allocate memory
  {NS}·T *array = {NS}·m.allocate_array(1 ,NULL);
  if( !array ) return false;

  // Access elements via access function
  {NS}·T *a = {NS}·m.access(array ,0);
  {NS}·T *b = {NS}·m.access(array ,1);

  // Assign value and copy
  {NS}·m.from_uint32(a ,42);
  {NS}·m.copy(b ,a);

  bool success = ( {NS}·m.compare(b ,a) == {NS}·Order_eq );
  {NS}·m.deallocate(array);
  return success;
}}

bool test_arithmetic(){{
  // Allocate memory
  {NS}·T *array = {NS}·m.allocate_array(2 ,NULL);
  if( !array ) return false;

  {NS}·T *a = {NS}·m.access(array ,0);
  {NS}·T *b = {NS}·m.access(array ,1);
  {NS}·T *result = {NS}·m.access(array ,2);

  {NS}·m.from_uint32(a ,20);
  {NS}·m.from_uint32(b ,22);

  if( {NS}·m.add(result ,a ,b) != {NS}·Status·ok ) return false;
  if( {NS}·m.compare(result ,{NS}·m.access(array ,0)) != {NS}·Order_gt ) return false;

  if( {NS}·m.subtract(result ,b ,a) != {NS}·Status·ok ) return false;
  if( {NS}·m.compare(result ,{NS}·m.access(array ,0)) != {NS}·Order_lt ) return false;

  {NS}·m.deallocate(array);
  return true;
}}

bool test_bitwise_operations(){{
  // Allocate memory
  {NS}·T *array = {NS}·m.allocate_array(2, NULL);
  if(!array) return false;

  {NS}·T *a = {NS}·m.access(array, 0);
  {NS}·T *b = {NS}·m.access(array, 1);
  {NS}·T *result = {NS}·m.access(array, 2);

  // a = 0x0F0F0F0F, b = 0xF0F0F0F0
  {NS}·m.from_uint32(a, 0x0F0F0F0F);
  {NS}·m.from_uint32(b, 0xF0F0F0F0);

  // bit_and => expect 0x00000000
  {NS}·m.bit_and(result, a, b);
  {NS}·m.from_uint32(a, 0x00000000);
  if({NS}·m.compare(result, a) != {NS}·Order_eq){{
    {NS}·m.deallocate(array);
    return false;
  }}

  // Reset a to 0x0F0F0F0F for next tests
  {NS}·m.from_uint32(a, 0x0F0F0F0F);

  // bit_or => expect 0xFFFFFFFF
  {NS}·m.bit_or(result, a, b);
  {NS}·m.from_uint32(b, 0xFFFFFFFF);
  if({NS}·m.compare(result, b) != {NS}·Order_eq){{
    {NS}·m.deallocate(array);
    return false;
  }}

  // bit_complement(a=0x0F0F0F0F) => expect 0xF0F0F0F0
  {NS}·m.from_uint32(a, 0x0F0F0F0F);
  {NS}·m.bit_complement(result, a);
  {NS}·m.from_uint32(b, 0xF0F0F0F0);
  if({NS}·m.compare(result, b) != {NS}·Order_eq){{
    {NS}·m.deallocate(array);
    return false;
  }}

  // bit_twos_complement(a=0x0F0F0F0F) => expect 0xF0F0F0F1
  {NS}·m.from_uint32(a, 0x0F0F0F0F);
  {NS}·m.bit_twos_complement(result, a);
  {NS}·m.from_uint32(b, 0xF0F0F0F1);
  if({NS}·m.compare(result, b) != {NS}·Order_eq){{
    {NS}·m.deallocate(array);
    return false;
  }}

  {NS}·m.deallocate(array);
  return true;
}}

bool test_comparisons(){{
  // Allocate memory
  {NS}·T *array = {NS}·m.allocate_array(2, NULL);
  if(!array) return false;

  {NS}·T *a = {NS}·m.access(array, 0);
  {NS}·T *b = {NS}·m.access(array, 1);
  {NS}·T *c = {NS}·m.access(array, 2);

  // First set: a=0, b=42, c=42
  {NS}·m.from_uint32(a, 0);
  {NS}·m.from_uint32(b, 42);
  {NS}·m.from_uint32(c, 42);

  // eq_zero(a) => true
  if(!{NS}·m.eq_zero(a)){{
    {NS}·m.deallocate(array);
    return false;
  }}
  // eq_zero(b) => false
  if({NS}·m.eq_zero(b)){{
    {NS}·m.deallocate(array);
    return false;
  }}
  // eq(b, c) => true
  if(!{NS}·m.eq(b, c)){{
    {NS}·m.deallocate(array);
    return false;
  }}
  // eq(a, b) => false
  if({NS}·m.eq(a, b)){{
    {NS}·m.deallocate(array);
    return false;
  }}
  // compare(a, b) => {NS}·Order_lt
  if({NS}·m.compare(a, b) != {NS}·Order_lt){{
    {NS}·m.deallocate(array);
    return false;
  }}
  // compare(b, a) => {NS}·Order_gt
  if({NS}·m.compare(b, a) != {NS}·Order_gt){{
    {NS}·m.deallocate(array);
    return false;
  }}
  // compare(b, c) => {NS}·Order_eq
  if({NS}·m.compare(b, c) != {NS}·Order_eq){{
    {NS}·m.deallocate(array);
    return false;
  }}
  // lt(a, b) => true, gt(b, a) => true
  if(!{NS}·m.lt(a, b) || !{NS}·m.gt(b, a)){{
    {NS}·m.deallocate(array);
    return false;
  }}

  // Second set: a=100, b=50
  {NS}·m.from_uint32(a, 100);
  {NS}·m.from_uint32(b, 50);
  if({NS}·m.compare(a, b) != {NS}·Order_gt){{
    {NS}·m.deallocate(array);
    return false;
  }}
  // eq_zero(a) => false
  if({NS}·m.eq_zero(a)){{
    {NS}·m.deallocate(array);
    return false;
  }}
  // eq_zero(b) => false
  if({NS}·m.eq_zero(b)){{
    {NS}·m.deallocate(array);
    return false;
  }}

  {NS}·m.deallocate(array);
  return true;
}}

bool test_shifts(){{
  // Allocate memory for operand, fill, spill
  {NS}·T *array = {NS}·m.allocate_array(2, NULL);
  if(!array) return false;

  {NS}·T *operand = {NS}·m.access(array, 0);
  {NS}·T *fill    = {NS}·m.access(array, 1);
  {NS}·T *spill   = {NS}·m.access(array, 2);

  // Subtest A: shift_left(4) with operand=1 => expect operand=16, fill=0, spill=0
  {NS}·m.from_uint32(operand, 1);
  {NS}·m.from_uint32(fill, 0);
  {NS}·m.from_uint32(spill, 0);
  if({NS}·m.shift_left(4, spill, operand, fill) != {NS}·Status·ok){{
    {NS}·m.deallocate(array);
    return false;
  }}
  {NS}·T *temp = {NS}·m.allocate_array(1, NULL);
  if(!temp){{
    {NS}·m.deallocate(array);
    return false;
  }}
  {NS}·m.from_uint32(temp, 16);
  if({NS}·m.compare(operand, temp) != {NS}·Order_eq){{
    {NS}·m.deallocate(temp);
    {NS}·m.deallocate(array);
    return false;
  }}
  if({NS}·m.compare(fill, {NS}·zero) != {NS}·Order_eq){{
    {NS}·m.deallocate(temp);
    {NS}·m.deallocate(array);
    return false;
  }}
  if({NS}·m.compare(spill, {NS}·zero) != {NS}·Order_eq){{
    {NS}·m.deallocate(temp);
    {NS}·m.deallocate(array);
    return false;
  }}

  // Subtest B: shift_left(1) with operand=0x80000000 => expect operand=0, spill=1
  {NS}·m.from_uint32(operand, 0x80000000);
  {NS}·m.from_uint32(fill, 0);
  {NS}·m.from_uint32(spill, 0);
  if({NS}·m.shift_left(1, spill, operand, fill) != {NS}·Status·ok){{
    {NS}·m.deallocate(temp);
    {NS}·m.deallocate(array);
    return false;
  }}
  if(!{NS}·m.eq_zero(operand)){{
    {NS}·m.deallocate(temp);
    {NS}·m.deallocate(array);
    return false;
  }}
  {NS}·m.from_uint32(temp, 1);
  if({NS}·m.compare(spill, temp) != {NS}·Order_eq){{
    {NS}·m.deallocate(temp);
    {NS}·m.deallocate(array);
    return false;
  }}

  // Subtest C: shift_right(1) with operand=0x80000000 => expect operand=0x40000000, spill=0
  {NS}·m.from_uint32(operand, 0x80000000);
  {NS}·m.from_uint32(fill, 0);
  {NS}·m.from_uint32(spill, 0);
  if({NS}·m.shift_right(1, spill, operand, fill) != {NS}·Status·ok){{
    {NS}·m.deallocate(temp);
    {NS}·m.deallocate(array);
    return false;
  }}
  {NS}·m.from_uint32(temp, 0x40000000);
  if({NS}·m.compare(operand, temp) != {NS}·Order_eq){{
    {NS}·m.deallocate(temp);
    {NS}·m.deallocate(array);
    return false;
  }}
  if(!{NS}·m.eq_zero(spill)){{
    {NS}·m.deallocate(temp);
    {NS}·m.deallocate(array);
    return false;
  }}

  // Subtest D: arithmetic_shift_right(1) with operand=0x80000000 => expect operand=0xC0000000, spill=0
  {NS}·m.from_uint32(operand, 0x80000000);
  {NS}·m.from_uint32(spill, 0);
  if({NS}·m.arithmetic_shift_right(1, operand, spill) != {NS}·Status·ok){{
    {NS}·m.deallocate(temp);
    {NS}·m.deallocate(array);
    return false;
  }}
  {NS}·m.from_uint32(temp, 0xC0000000);
  if({NS}·m.compare(operand, temp) != {NS}·Order_eq){{
    {NS}·m.deallocate(temp);
    {NS}·m.deallocate(array);
    return false;
  }}
  if(!{NS}·m.eq_zero(spill)){{
    {NS}·m.deallocate(temp);
    {NS}·m.deallocate(array);
    return false;
  }}

  {NS}·m.deallocate(temp);
  {NS}·m.deallocate(array);
  return true;
}}



// Include the local section of {NS}.lib.c for testing
#define LOCAL
#include "{NS}.lib.c"
#undef LOCAL
'''
