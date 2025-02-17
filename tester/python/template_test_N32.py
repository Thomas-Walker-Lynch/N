# {NAMESPACE} must be a N32 type.
#
# test_N32 can be used to test any 32 bit natural number types. As examples: N32PN the processor native type; N32_4_by_8 a 32 bit natural number type made of 4 digits, where each digit is 8 bits.
#

def test_N32(namespace: str) -> str:
    """
    Returns a source code file for cc.
    """
    template = template_test_N32()
    code = template.format(
        NAMESPACE = namespace
    )
    return code

def template_test_N32():
    return r'''#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <setjmp.h>

// Enable interface section
#define FACE
#include "{NAMESPACE}.lib.c"
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
  {NAMESPACE}·T *array = {NAMESPACE}·λ.allocate_array(2 ,NULL);
  if( !array ) return false;

  // Access elements via access function
  {NAMESPACE}·T *a = {NAMESPACE}·λ.access(array ,0);
  {NAMESPACE}·T *b = {NAMESPACE}·λ.access(array ,1);

  // Assign value and copy
  {NAMESPACE}·λ.from_uint32(a ,42);
  {NAMESPACE}·λ.copy(b ,a);

  bool success = ( {NAMESPACE}·λ.compare(b ,a) == {NAMESPACE}·Order_eq );
  {NAMESPACE}·λ.deallocate(array);
  return success;
}}

bool test_arithmetic(){{
  // Allocate memory
  {NAMESPACE}·T *array = {NAMESPACE}·λ.allocate_array(3 ,NULL);
  if( !array ) return false;

  {NAMESPACE}·T *a = {NAMESPACE}·λ.access(array ,0);
  {NAMESPACE}·T *b = {NAMESPACE}·λ.access(array ,1);
  {NAMESPACE}·T *result = {NAMESPACE}·λ.access(array ,2);

  {NAMESPACE}·λ.from_uint32(a ,20);
  {NAMESPACE}·λ.from_uint32(b ,22);

  if( {NAMESPACE}·λ.add(result ,a ,b) != {NAMESPACE}·Status·ok ) return false;
  if( {NAMESPACE}·λ.compare(result ,{NAMESPACE}·λ.access(array ,0)) != {NAMESPACE}·Order_gt ) return false;

  if( {NAMESPACE}·λ.subtract(result ,b ,a) != {NAMESPACE}·Status·ok ) return false;
  if( {NAMESPACE}·λ.compare(result ,{NAMESPACE}·λ.access(array ,0)) != {NAMESPACE}·Order_lt ) return false;

  {NAMESPACE}·λ.deallocate(array);
  return true;
}}

bool test_bitwise_operations(){{
  // Allocate memory
  {NAMESPACE}·T *array = {NAMESPACE}·λ.allocate_array(3, NULL);
  if(!array) return false;

  {NAMESPACE}·T *a = {NAMESPACE}·λ.access(array, 0);
  {NAMESPACE}·T *b = {NAMESPACE}·λ.access(array, 1);
  {NAMESPACE}·T *result = {NAMESPACE}·λ.access(array, 2);

  // a = 0x0F0F0F0F, b = 0xF0F0F0F0
  {NAMESPACE}·λ.from_uint32(a, 0x0F0F0F0F);
  {NAMESPACE}·λ.from_uint32(b, 0xF0F0F0F0);

  // bit_and => expect 0x00000000
  {NAMESPACE}·λ.bit_and(result, a, b);
  {NAMESPACE}·λ.from_uint32(a, 0x00000000);
  if({NAMESPACE}·λ.compare(result, a) != {NAMESPACE}·Order_eq){{
    {NAMESPACE}·λ.deallocate(array);
    return false;
  }}

  // Reset a to 0x0F0F0F0F for next tests
  {NAMESPACE}·λ.from_uint32(a, 0x0F0F0F0F);

  // bit_or => expect 0xFFFFFFFF
  {NAMESPACE}·λ.bit_or(result, a, b);
  {NAMESPACE}·λ.from_uint32(b, 0xFFFFFFFF);
  if({NAMESPACE}·λ.compare(result, b) != {NAMESPACE}·Order_eq){{
    {NAMESPACE}·λ.deallocate(array);
    return false;
  }}

  // bit_complement(a=0x0F0F0F0F) => expect 0xF0F0F0F0
  {NAMESPACE}·λ.from_uint32(a, 0x0F0F0F0F);
  {NAMESPACE}·λ.bit_complement(result, a);
  {NAMESPACE}·λ.from_uint32(b, 0xF0F0F0F0);
  if({NAMESPACE}·λ.compare(result, b) != {NAMESPACE}·Order_eq){{
    {NAMESPACE}·λ.deallocate(array);
    return false;
  }}

  // bit_twos_complement(a=0x0F0F0F0F) => expect 0xF0F0F0F1
  {NAMESPACE}·λ.from_uint32(a, 0x0F0F0F0F);
  {NAMESPACE}·λ.bit_twos_complement(result, a);
  {NAMESPACE}·λ.from_uint32(b, 0xF0F0F0F1);
  if({NAMESPACE}·λ.compare(result, b) != {NAMESPACE}·Order_eq){{
    {NAMESPACE}·λ.deallocate(array);
    return false;
  }}

  {NAMESPACE}·λ.deallocate(array);
  return true;
}}

bool test_comparisons(){{
  // Allocate memory
  {NAMESPACE}·T *array = {NAMESPACE}·λ.allocate_array(3, NULL);
  if(!array) return false;

  {NAMESPACE}·T *a = {NAMESPACE}·λ.access(array, 0);
  {NAMESPACE}·T *b = {NAMESPACE}·λ.access(array, 1);
  {NAMESPACE}·T *c = {NAMESPACE}·λ.access(array, 2);

  // First set: a=0, b=42, c=42
  {NAMESPACE}·λ.from_uint32(a, 0);
  {NAMESPACE}·λ.from_uint32(b, 42);
  {NAMESPACE}·λ.from_uint32(c, 42);

  // eq_zero(a) => true
  if(!{NAMESPACE}·λ.eq_zero(a)){{
    {NAMESPACE}·λ.deallocate(array);
    return false;
  }}
  // eq_zero(b) => false
  if({NAMESPACE}·λ.eq_zero(b)){{
    {NAMESPACE}·λ.deallocate(array);
    return false;
  }}
  // eq(b, c) => true
  if(!{NAMESPACE}·λ.eq(b, c)){{
    {NAMESPACE}·λ.deallocate(array);
    return false;
  }}
  // eq(a, b) => false
  if({NAMESPACE}·λ.eq(a, b)){{
    {NAMESPACE}·λ.deallocate(array);
    return false;
  }}
  // compare(a, b) => {NAMESPACE}·Order_lt
  if({NAMESPACE}·λ.compare(a, b) != {NAMESPACE}·Order_lt){{
    {NAMESPACE}·λ.deallocate(array);
    return false;
  }}
  // compare(b, a) => {NAMESPACE}·Order_gt
  if({NAMESPACE}·λ.compare(b, a) != {NAMESPACE}·Order_gt){{
    {NAMESPACE}·λ.deallocate(array);
    return false;
  }}
  // compare(b, c) => {NAMESPACE}·Order_eq
  if({NAMESPACE}·λ.compare(b, c) != {NAMESPACE}·Order_eq){{
    {NAMESPACE}·λ.deallocate(array);
    return false;
  }}
  // lt(a, b) => true, gt(b, a) => true
  if(!{NAMESPACE}·λ.lt(a, b) || !{NAMESPACE}·λ.gt(b, a)){{
    {NAMESPACE}·λ.deallocate(array);
    return false;
  }}

  // Second set: a=100, b=50
  {NAMESPACE}·λ.from_uint32(a, 100);
  {NAMESPACE}·λ.from_uint32(b, 50);
  if({NAMESPACE}·λ.compare(a, b) != {NAMESPACE}·Order_gt){{
    {NAMESPACE}·λ.deallocate(array);
    return false;
  }}
  // eq_zero(a) => false
  if({NAMESPACE}·λ.eq_zero(a)){{
    {NAMESPACE}·λ.deallocate(array);
    return false;
  }}
  // eq_zero(b) => false
  if({NAMESPACE}·λ.eq_zero(b)){{
    {NAMESPACE}·λ.deallocate(array);
    return false;
  }}

  {NAMESPACE}·λ.deallocate(array);
  return true;
}}

bool test_shifts(){{
  // Allocate memory for operand, fill, spill
  {NAMESPACE}·T *array = {NAMESPACE}·λ.allocate_array(3, NULL);
  if(!array) return false;

  {NAMESPACE}·T *operand = {NAMESPACE}·λ.access(array, 0);
  {NAMESPACE}·T *fill    = {NAMESPACE}·λ.access(array, 1);
  {NAMESPACE}·T *spill   = {NAMESPACE}·λ.access(array, 2);

  // Subtest A: shift_left(4) with operand=1 => expect operand=16, fill=0, spill=0
  {NAMESPACE}·λ.from_uint32(operand, 1);
  {NAMESPACE}·λ.from_uint32(fill, 0);
  {NAMESPACE}·λ.from_uint32(spill, 0);
  if({NAMESPACE}·λ.shift_left(4, spill, operand, fill) != {NAMESPACE}·Status·ok){{
    {NAMESPACE}·λ.deallocate(array);
    return false;
  }}
  {NAMESPACE}·T *temp = {NAMESPACE}·λ.allocate_array(1, NULL);
  if(!temp){{
    {NAMESPACE}·λ.deallocate(array);
    return false;
  }}
  {NAMESPACE}·λ.from_uint32(temp, 16);
  if({NAMESPACE}·λ.compare(operand, temp) != {NAMESPACE}·Order_eq){{
    {NAMESPACE}·λ.deallocate(temp);
    {NAMESPACE}·λ.deallocate(array);
    return false;
  }}
  if({NAMESPACE}·λ.compare(fill, {NAMESPACE}·zero) != {NAMESPACE}·Order_eq){{
    {NAMESPACE}·λ.deallocate(temp);
    {NAMESPACE}·λ.deallocate(array);
    return false;
  }}
  if({NAMESPACE}·λ.compare(spill, {NAMESPACE}·zero) != {NAMESPACE}·Order_eq){{
    {NAMESPACE}·λ.deallocate(temp);
    {NAMESPACE}·λ.deallocate(array);
    return false;
  }}

  // Subtest B: shift_left(1) with operand=0x80000000 => expect operand=0, spill=1
  {NAMESPACE}·λ.from_uint32(operand, 0x80000000);
  {NAMESPACE}·λ.from_uint32(fill, 0);
  {NAMESPACE}·λ.from_uint32(spill, 0);
  if({NAMESPACE}·λ.shift_left(1, spill, operand, fill) != {NAMESPACE}·Status·ok){{
    {NAMESPACE}·λ.deallocate(temp);
    {NAMESPACE}·λ.deallocate(array);
    return false;
  }}
  if(!{NAMESPACE}·λ.eq_zero(operand)){{
    {NAMESPACE}·λ.deallocate(temp);
    {NAMESPACE}·λ.deallocate(array);
    return false;
  }}
  {NAMESPACE}·λ.from_uint32(temp, 1);
  if({NAMESPACE}·λ.compare(spill, temp) != {NAMESPACE}·Order_eq){{
    {NAMESPACE}·λ.deallocate(temp);
    {NAMESPACE}·λ.deallocate(array);
    return false;
  }}

  // Subtest C: shift_right(1) with operand=0x80000000 => expect operand=0x40000000, spill=0
  {NAMESPACE}·λ.from_uint32(operand, 0x80000000);
  {NAMESPACE}·λ.from_uint32(fill, 0);
  {NAMESPACE}·λ.from_uint32(spill, 0);
  if({NAMESPACE}·λ.shift_right(1, spill, operand, fill) != {NAMESPACE}·Status·ok){{
    {NAMESPACE}·λ.deallocate(temp);
    {NAMESPACE}·λ.deallocate(array);
    return false;
  }}
  {NAMESPACE}·λ.from_uint32(temp, 0x40000000);
  if({NAMESPACE}·λ.compare(operand, temp) != {NAMESPACE}·Order_eq){{
    {NAMESPACE}·λ.deallocate(temp);
    {NAMESPACE}·λ.deallocate(array);
    return false;
  }}
  if(!{NAMESPACE}·λ.eq_zero(spill)){{
    {NAMESPACE}·λ.deallocate(temp);
    {NAMESPACE}·λ.deallocate(array);
    return false;
  }}

  // Subtest D: arithmetic_shift_right(1) with operand=0x80000000 => expect operand=0xC0000000, spill=0
  {NAMESPACE}·λ.from_uint32(operand, 0x80000000);
  {NAMESPACE}·λ.from_uint32(spill, 0);
  if({NAMESPACE}·λ.arithmetic_shift_right(1, operand, spill) != {NAMESPACE}·Status·ok){{
    {NAMESPACE}·λ.deallocate(temp);
    {NAMESPACE}·λ.deallocate(array);
    return false;
  }}
  {NAMESPACE}·λ.from_uint32(temp, 0xC0000000);
  if({NAMESPACE}·λ.compare(operand, temp) != {NAMESPACE}·Order_eq){{
    {NAMESPACE}·λ.deallocate(temp);
    {NAMESPACE}·λ.deallocate(array);
    return false;
  }}
  if(!{NAMESPACE}·λ.eq_zero(spill)){{
    {NAMESPACE}·λ.deallocate(temp);
    {NAMESPACE}·λ.deallocate(array);
    return false;
  }}

  {NAMESPACE}·λ.deallocate(temp);
  {NAMESPACE}·λ.deallocate(array);
  return true;
}}



// Include the local section of {NAMESPACE}.lib.c for testing
#define LOCAL
#include "{NAMESPACE}.lib.c"
#undef LOCAL
'''
